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
#include "../Maths/3dmaths.h"
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

	// Create the perspective projection for the viewport
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(fov, pViewport->Aspect, m_clipNear, m_clipFar);
	float mat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, mat);
	pViewport->Perspective = mat;
	glPopMatrix();

	// Create the orthographic projection matrix for the viewport
	float coordright = 1.0f;
	float coordleft = -1.0f;
	float coordtop = 1.0f;
	float coordbottom = -1.0f;

	memset(&(pViewport->Orthographic), 0, sizeof(Matrix4x4));
	pViewport->Orthographic.m[0] = 2.0f / (coordright - coordleft);
	pViewport->Orthographic.m[5] = 2.0f / (coordtop - coordbottom);
	pViewport->Orthographic.m[10] = -2.0f / (m_clipFar - m_clipNear);
	pViewport->Orthographic.m[12] = -(coordright + coordleft) / (coordright - coordleft);
	pViewport->Orthographic.m[13] = -(coordtop + coordbottom) / (coordtop - coordbottom);
	pViewport->Orthographic.m[14] = -(m_clipFar + m_clipNear) / (m_clipFar - m_clipNear);
	pViewport->Orthographic.m[15] = 1.0f;

	// Create the 2d projection matrix for the viewport
	coordright = (float)m_windowWidth;
	coordleft = 0.0f;
	coordtop = (float)m_windowHeight;
	coordbottom = 0.0f;

	memset(&(pViewport->Projection2d), 0, sizeof(Matrix4x4));
	pViewport->Projection2d.m[0] = 2.0f / (coordright - coordleft);
	pViewport->Projection2d.m[5] = 2.0f / (coordtop - coordbottom);
	pViewport->Projection2d.m[10] = -2.0f / (m_clipFar - m_clipNear);
	pViewport->Projection2d.m[12] = -(coordright + coordleft) / (coordright - coordleft);
	pViewport->Projection2d.m[13] = -(coordtop + coordbottom) / (coordtop - coordbottom);
	pViewport->Projection2d.m[14] = -(m_clipFar + m_clipNear) / (m_clipFar - m_clipNear);
	pViewport->Projection2d.m[15] = 1.0f;

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

	// Create the perspective projection for the viewport
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(pViewport->Fov, pViewport->Aspect, m_clipNear, m_clipFar);
	float mat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, mat);
	pViewport->Perspective = mat;
	glPopMatrix();

	// Create the orthographic projection matrix for the viewport
	float coordright = 1.0f;
	float coordleft = -1.0f;
	float coordtop = 1.0f;
	float coordbottom = -1.0f;

	memset(&(pViewport->Orthographic), 0, sizeof(Matrix4x4));
	pViewport->Orthographic.m[0] = 2.0f / (coordright - coordleft);
	pViewport->Orthographic.m[5] = 2.0f / (coordtop - coordbottom);
	pViewport->Orthographic.m[10] = -2.0f / (m_clipFar - m_clipNear);
	pViewport->Orthographic.m[12] = -(coordright + coordleft) / (coordright - coordleft);
	pViewport->Orthographic.m[13] = -(coordtop + coordbottom) / (coordtop - coordbottom);
	pViewport->Orthographic.m[14] = -(m_clipFar + m_clipNear) / (m_clipFar - m_clipNear);
	pViewport->Orthographic.m[15] = 1.0f;

	// Create the 2d projection matrix for the viewport
	coordright = (float)m_windowWidth;
	coordleft = 0.0f;
	coordtop = (float)m_windowHeight;
	coordbottom = 0.0f;

	memset(&(pViewport->Projection2d), 0, sizeof(Matrix4x4));
	pViewport->Projection2d.m[0] = 2.0f / (coordright - coordleft);
	pViewport->Projection2d.m[5] = 2.0f / (coordtop - coordbottom);
	pViewport->Projection2d.m[10] = -2.0f / (m_clipFar - m_clipNear);
	pViewport->Projection2d.m[12] = -(coordright + coordleft) / (coordright - coordleft);
	pViewport->Projection2d.m[13] = -(coordtop + coordbottom) / (coordtop - coordbottom);
	pViewport->Projection2d.m[14] = -(m_clipFar + m_clipNear) / (m_clipFar - m_clipNear);
	pViewport->Projection2d.m[15] = 1.0f;
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