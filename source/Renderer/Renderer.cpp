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

	// Depth buffer
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);
	glDepthFunc(GL_LESS);

	// Stencil buffer
	glClearStencil(0);

	// Glew init
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		cout << "Failed to initialize GLEW" << endl;
		cout << "Error:" << glewGetErrorString(err) << endl;
		return;
	}
}

Renderer::~Renderer()
{
	ResetLines();
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

void Renderer::RenderLines()
{
	// Calculate the stride
	GLsizei totalStride = sizeof(float) * 7;  // x, y, z, r, g, b, a

	unsigned int numVertices = (unsigned int)m_vpLines.size() * 2;
	float *pVA = new float[numVertices * 7];

	int arrayCounter = 0;
	for (unsigned int i = 0; i < m_vpLines.size(); i++)
	{
		pVA[arrayCounter + 0] = m_vpLines[i]->m_lineStart.x;
		pVA[arrayCounter + 1] = m_vpLines[i]->m_lineStart.y;
		pVA[arrayCounter + 2] = m_vpLines[i]->m_lineStart.z;
		pVA[arrayCounter + 3] = m_vpLines[i]->m_lineStartColour.GetRed();
		pVA[arrayCounter + 4] = m_vpLines[i]->m_lineStartColour.GetGreen();
		pVA[arrayCounter + 5] = m_vpLines[i]->m_lineStartColour.GetBlue();
		pVA[arrayCounter + 6] = m_vpLines[i]->m_lineStartColour.GetAlpha();
		pVA[arrayCounter + 7] = m_vpLines[i]->m_lineEnd.x;
		pVA[arrayCounter + 8] = m_vpLines[i]->m_lineEnd.y;
		pVA[arrayCounter + 9] = m_vpLines[i]->m_lineEnd.z;
		pVA[arrayCounter + 10] = m_vpLines[i]->m_lineEndColour.GetRed();
		pVA[arrayCounter + 11] = m_vpLines[i]->m_lineEndColour.GetGreen();
		pVA[arrayCounter + 12] = m_vpLines[i]->m_lineEndColour.GetBlue();
		pVA[arrayCounter + 13] = m_vpLines[i]->m_lineEndColour.GetAlpha();

		arrayCounter += 14;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, totalStride, pVA);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, totalStride, &pVA[3]);

	glDrawArrays(GL_POINTS, 0, numVertices);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}