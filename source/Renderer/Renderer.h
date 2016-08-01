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

#include <vector>
using namespace std;

#include "camera.h"
#include "Shader.h"
#include "colour.h"
#include "viewport.h"
#include "../Maths/3dmaths.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif //_WIN32
#include <GL/gl.h>
#include <GL/glu.h>

#pragma comment (lib, "opengl32")
#pragma comment (lib, "glu32")


class PositionColorVertex
{
public:
	float x, y, z;      // Position
	float r, g, b, a;   // Colour
};

class PositionColorNormalVertex
{
public:
	float x, y, z;      // Position
	float r, g, b, a;   // Colour
	float nx, ny, nz;   // Normal
};

class Line
{
public:
	vec3 m_lineStart;
	vec3 m_lineEnd;
	Colour m_lineStartColour;
	Colour m_lineEndColour;
};

class Renderer
{
public:
	/* Public methods */
	Renderer(int width, int height);
	~Renderer();

	// Setup
	void SetupShaders();

	// Resize
	void ResizeWindow(int newWidth, int newHeight);

	// Scene
	bool ClearScene(bool pixel = true, bool depth = true, bool stencil = true);
	void SetColourMask(bool red, bool green, bool blue, bool alpha);
	void SetClearColour(float red, float green, float blue, float alpha);

	// Viewport
	Viewport* CreateViewport(int bottom, int left, int width, int height, float fov);
	void ResizeViewport(Viewport* pViewport, int bottom, int left, int width, int height, float fov);
	void SetViewport(Viewport* pViewport);

	// Rendering
	void ResetLines();
	void DrawLine(vec3 lineSart, vec3 lineEnd, Colour lineStartColour, Colour lineEndColour);
	void DrawCube(vec3 pos, float length, float height, float width, Colour color);
	void RenderLines(Camera* pCamera);

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
	// Window dimensions
	int m_windowWidth;
	int m_windowHeight;

	// Clipping planes
	float m_clipNear;
	float m_clipFar;

	// Shaders
	Shader* m_pPositionColorShader;

	// Rendering
	vector<Line*> m_vpLines;
};

int CheckGLErrors(char *file, int line);
#define CHECK_GL_ERRORS() CheckGLErrors(__FILE__, __LINE__)