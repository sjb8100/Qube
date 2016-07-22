// ******************************************************************************
// Filename:	freetypefont.h
// Project:		Qube
// Author:		Steven Ball
//
// Purpose:
//   A font container class that is a wrapper around FreeType font rendering.
//
// Revision History:
//   Initial Revision - 11/10/08
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include <GL/glew.h>

#ifdef _WIN32
#include <windows.h>
#endif //_WIN32
#include <GL/gl.h>
#include <GL/glu.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include <map>
using namespace std;


// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	ivec2 Size;    // Size of glyph
	ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};

class FreeTypeFont {
public:
	FreeTypeFont();
	~FreeTypeFont();

	void BuildFont(const char* fontName, int size);

public:
	map<GLchar, Character> Characters;
	GLuint VAO, VBO;
};
