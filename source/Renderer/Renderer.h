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

#ifdef _WIN32
#include <windows.h>
#endif //_WIN32
#include <GL/gl.h>
#include <GL/glu.h>

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
