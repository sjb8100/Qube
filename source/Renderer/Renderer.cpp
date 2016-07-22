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
	m_windowWidth = width;
	m_windowHeight = height;

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