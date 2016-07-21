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