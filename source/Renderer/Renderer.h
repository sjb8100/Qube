// ******************************************************************************
// Filename:    Renderer.h
// Project:     Qube
// Author:      Steven Ball
//
// Purpose:
//   The OpenGL renderer that is an encapsulation of all the rendering
//   functionality of the engine. A wrapper around most common OpenGL calls.
//
// Revision History:
//   Initial Revision - 20/07/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include <glm/vec3.hpp>
using namespace glm;

#ifdef _WIN32
#include <windows.h>
#endif //_WIN32
#include <GL/gl.h>
#include <GL/glu.h>

#include "colour.h"
#include "../freetype/freetypefont.h"

#pragma comment (lib, "opengl32")
#pragma comment (lib, "glu32")


class Renderer
{
public:
	/* Public methods */
	Renderer(int width, int height);
	~Renderer();

	// Resize
	void ResizeWindow(int newWidth, int newHeight);

	// Scene
	bool ClearScene(bool pixel = true, bool depth = true, bool stencil = true);
	void SetColourMask(bool red, bool green, bool blue, bool alpha);
	void SetClearColour(float red, float green, float blue, float alpha);

	// Text and font rendering
	FreeTypeFont* CreateFreeTypeFont(const char *fontName, int fontSize, bool noAutoHint = false);
	void RenderFreeTypeText(FreeTypeFont* pFont, float x, float y, float z, Colour colour, float scale, const char *inText, ...);
	int GetFreeTypeTextWidth(FreeTypeFont* pFont, const char *inText, ...);
	int GetFreeTypeTextHeight(FreeTypeFont* pFont, const char *inText, ...);
	int GetFreeTypeTextAscent(FreeTypeFont* pFont);
	int GetFreeTypeTextDescent(FreeTypeFont* pFont);

protected:
	/* Protected methods */

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	int m_windowWidth;
	int m_windowHeight;
};

int CheckGLErrors(char *file, int line);
#define CHECK_GL_ERRORS() CheckGLErrors(__FILE__, __LINE__)
