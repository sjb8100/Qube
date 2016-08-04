// ******************************************************************************
// Filename:  Material.h
// Project:   Qube
// Author:    Steven Ball
//
// Purpose:
//   An OpenGL material type.
//
// Revision History:
//   Initial Revision - 01/05/06
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "colour.h"


class Material
{
public:
	Colour m_ambient;
	Colour m_diffuse;
	Colour m_specular;
	Colour m_emission;
	float m_shininess;
};