// ******************************************************************************
// Filename:  Light.h
// Project:   Qube
// Author:    Steven Ball
//
// Purpose:
//   An OpenGL hardware light.
//
// Revision History:
//   Initial Revision - 01/03/06
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "colour.h"
#include <glm/glm.hpp>
using namespace glm;


class Light
{
public:
	vec3 m_position;
	Colour m_ambient;
	Colour m_diffuse;
	Colour m_specular;
	float m_constantAttenuation;
	float m_linearAttenuation;
	float m_quadraticAttenuation;
};
