// ******************************************************************************
// Filename:    Shader.h
// Project:     Qube
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 22/07/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include <GL/glew.h>

class Shader
{
public:
	Shader(const char* vertexPath, const char* fragmentPath);
	~Shader();

	GLuint GetShader();

	void UseShader();

private:
	GLuint m_pProgram;
};