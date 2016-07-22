// ******************************************************************************
// Filename:    Renderer.h
// Project:     Qube
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 20/07/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "../glew/include/GL/glew.h"
#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
using namespace std;

// GL ERROR CHECK
int CheckGLErrors(const char *file, int line)
{
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		const GLubyte* sError = gluErrorString(glErr);

		if (sError)
			cout << "GL Error #" << glErr << "(" << gluErrorString(glErr) << ") " << " in File " << file << " at line: " << line << endl;
		else
			cout << "GL Error #" << glErr << " (no message available)" << " in File " << file << " at line: " << line << endl;

		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}

Renderer::Renderer(int width, int height)
{
	// Window dimensions
	m_windowWidth = width;
	m_windowHeight = height;

	// Default clipping planes
	m_clipNear = 0.1f;
	m_clipFar = 10000.0f;

	// Glew init
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		cout << "Failed to initialize GLEW" << endl;
		cout << "Error:" << glewGetErrorString(err) << endl;
		return;
	}

	// Setup the shaders
	SetupShaders();
}

Renderer::~Renderer()
{
	ResetLines();

	delete m_pPositionColorShader;
}

// Setup
void Renderer::SetupShaders()
{
	m_pPositionColorShader = new Shader("media/shaders/PositionColor.vertex", "media/shaders/PositionColor.fragment");
}

// Resize
void Renderer::ResizeWindow(int newWidth, int newHeight)
{
	m_windowWidth = newWidth;
	m_windowHeight = newHeight;
}

// Scene
bool Renderer::ClearScene(bool pixel, bool depth, bool stencil)
{
	GLbitfield clear(0);

	if (pixel)
		clear |= GL_COLOR_BUFFER_BIT;
	if (depth)
		clear |= GL_DEPTH_BUFFER_BIT;
	if (stencil)
		clear |= GL_STENCIL_BUFFER_BIT;

	glClear(clear);

	return true;
}

void Renderer::SetColourMask(bool red, bool green, bool blue, bool alpha)
{
	glColorMask(red, green, blue, alpha);
}

void Renderer::SetClearColour(float red, float green, float blue, float alpha)
{
	glClearColor(red, green, blue, alpha);
}

// Viewport
Viewport* Renderer::CreateViewport(int bottom, int left, int width, int height, float fov)
{
	Viewport* pViewport = new Viewport();

	pViewport->Bottom = bottom;
	pViewport->Left = left;
	pViewport->Width = width;
	pViewport->Height = height;
	pViewport->Fov = fov;
	pViewport->Aspect = (float)width / (float)height;

	return pViewport;
}

void Renderer::ResizeViewport(Viewport* pViewport, int bottom, int left, int width, int height, float fov)
{
	pViewport->Bottom = bottom;
	pViewport->Left = left;
	pViewport->Width = width;
	pViewport->Height = height;
	pViewport->Fov = fov;
	pViewport->Aspect = (float)width / (float)height;
}

void Renderer::SetViewport(Viewport* pViewport)
{
	glViewport(pViewport->Left, pViewport->Bottom, pViewport->Width, pViewport->Height);
}

// Text and font rendering
FreeTypeFont* Renderer::CreateFreeTypeFont(const char *fontName, int fontSize, bool noAutoHint)
{
	FreeTypeFont* font = new FreeTypeFont();

	// Build the new freetype font
	font->BuildFont(fontName, fontSize, noAutoHint);

	return font;
}

void Renderer::RenderFreeTypeText(FreeTypeFont* pFont, float x, float y, float z, Colour colour, float scale, const char *inText, ...)
{
	char		outText[8192];
	va_list		ap;  // Pointer to list of arguments

	if (inText == NULL)
	{
		return;  // Early return if there is no text
	}

	// Loop through variable argument list and add them to the string
	va_start(ap, inText);
		vsprintf(outText, inText, ap);
	va_end(ap);

	glColor4fv(colour.GetRGBA());

	// Add on the descent value, so we don't draw letters with underhang out of bounds. (e.g - g, y, q and p)
	y -= GetFreeTypeTextDescent(pFont);

	// HACK : The descent has rounding errors and is usually off by about 1 pixel
	y -= 1;

	glPushMatrix();
		glTranslatef(x, y, 0);
		pFont->DrawString(outText, scale);
	glPopMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

int Renderer::GetFreeTypeTextWidth(FreeTypeFont* pFont, const char *inText, ...)
{
	char outText[8192];
	va_list ap;

	if (inText == NULL)
		return 0;

	// Loop through variable argument list and add them to the string
	va_start(ap, inText);
		vsprintf(outText, inText, ap);
	va_end(ap);

	return pFont->GetTextWidth(outText);
}

int Renderer::GetFreeTypeTextHeight(FreeTypeFont* pFont, const char *inText, ...)
{
	return pFont->GetCharHeight('a');
}

int Renderer::GetFreeTypeTextAscent(FreeTypeFont* pFont)
{
	return pFont->GetAscent();
}

int Renderer::GetFreeTypeTextDescent(FreeTypeFont* pFont)
{
	return pFont->GetDescent();
}

// Rendering
void Renderer::ResetLines()
{
	for (unsigned int i = 0; i < m_vpLines.size(); i++)
	{
		delete m_vpLines[i];
		m_vpLines[i] = 0;
	}
	m_vpLines.clear();
}

void Renderer::DrawLine(vec3 lineSart, vec3 lineEnd, Colour lineStartColour, Colour lineEndColour)
{
	Line* pNewLine = new Line();
	pNewLine->m_lineStart = lineSart;
	pNewLine->m_lineEnd = lineEnd;
	pNewLine->m_lineStartColour = lineStartColour;
	pNewLine->m_lineEndColour = lineEndColour;

	m_vpLines.push_back(pNewLine);
}

void Renderer::RenderLines(Camera* pCamera)
{
	// Num vertices
	unsigned int numVertices = (unsigned int)m_vpLines.size() * 2;

	// Vertices
	PositionColorVertex* linesBuffer;
	linesBuffer = new PositionColorVertex[numVertices];
	int arrayCounter = 0;
	for (unsigned int i = 0; i < (unsigned int)m_vpLines.size(); i++)
	{
		linesBuffer[arrayCounter + 0].x = m_vpLines[i]->m_lineStart.x;
		linesBuffer[arrayCounter + 0].y = m_vpLines[i]->m_lineStart.y;
		linesBuffer[arrayCounter + 0].z = m_vpLines[i]->m_lineStart.z;
		linesBuffer[arrayCounter + 0].r = m_vpLines[i]->m_lineStartColour.GetRed();
		linesBuffer[arrayCounter + 0].g = m_vpLines[i]->m_lineStartColour.GetGreen();
		linesBuffer[arrayCounter + 0].b = m_vpLines[i]->m_lineStartColour.GetBlue();
		linesBuffer[arrayCounter + 0].a = m_vpLines[i]->m_lineStartColour.GetAlpha();
		linesBuffer[arrayCounter + 1].x = m_vpLines[i]->m_lineEnd.x;
		linesBuffer[arrayCounter + 1].y = m_vpLines[i]->m_lineEnd.y;
		linesBuffer[arrayCounter + 1].z = m_vpLines[i]->m_lineEnd.z;
		linesBuffer[arrayCounter + 1].r = m_vpLines[i]->m_lineEndColour.GetRed();
		linesBuffer[arrayCounter + 1].g = m_vpLines[i]->m_lineEndColour.GetGreen();
		linesBuffer[arrayCounter + 1].b = m_vpLines[i]->m_lineEndColour.GetBlue();
		linesBuffer[arrayCounter + 1].a = m_vpLines[i]->m_lineEndColour.GetAlpha();

		arrayCounter += 2;
	}

	// Indices
	GLuint* indicesBuffer;
	indicesBuffer = new GLuint[numVertices];
	for (unsigned int i = 0; i < numVertices; i++)
	{
		indicesBuffer[i] = i;
	}

	GLuint VBO, VAO, EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PositionColorVertex)*numVertices, linesBuffer, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*numVertices, indicesBuffer, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 7, (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 7, (GLvoid*)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	// Use shader
	m_pPositionColorShader->UseShader();

	// Create transformations
	glm::mat4 view;
	glm::mat4 projection;
	view = glm::lookAt(pCamera->GetPosition(), pCamera->GetView(), pCamera->GetUp());
	projection = glm::perspective(45.0f, (GLfloat)m_windowWidth / (GLfloat)m_windowHeight, 0.01f, 1000.0f);
	
	// Get their uniform location
	GLint modelLoc = glGetUniformLocation(m_pPositionColorShader->GetShader(), "model");
	GLint viewLoc = glGetUniformLocation(m_pPositionColorShader->GetShader(), "view");
	GLint projLoc = glGetUniformLocation(m_pPositionColorShader->GetShader(), "projection");

	// Pass the matrices to the shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	// Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(VAO);

	glm::mat4 model;
	model = glm::translate(model, vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_LINES, numVertices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}