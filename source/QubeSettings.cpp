// ******************************************************************************
// Filename:    QubeSettings.cpp
// Project:     Qube
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 11/04/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "QubeSettings.h"

#include <iostream>
#include "ini/INIReader.h"

#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
using namespace std;


QubeSettings::QubeSettings()
{
}

QubeSettings::~QubeSettings()
{
}

// Load settings
void QubeSettings::LoadSettings()
{
	string settingsIniFile = "media/config/settings.ini";
	INIReader reader(settingsIniFile);

	if (reader.ParseError() < 0)
	{
		cout << "Can't load '" << settingsIniFile << "'\n";
		return;
	}

	// Graphics
	m_windowWidth = reader.GetInteger("Graphics", "WindowWidth", 800);
	m_windowHeight = reader.GetInteger("Graphics", "WindowHeight", 800);
	m_vsync = reader.GetBoolean("Graphics", "VSync", false);
	m_fullscreen = reader.GetBoolean("Graphics", "FullScreen", false);

	// Debug
	m_debugRendering = reader.GetBoolean("Debug", "DebugRendering", false);
	m_gameMode = reader.Get("Debug", "GameMode", "Debug");
	m_version = reader.Get("Debug", "Version", "1.0");
}

// Save settings
void QubeSettings::SaveSettings()
{
}

// Load options
void QubeSettings::LoadOptions()
{
	string optionsIniFile = "media/config/options.ini";
	INIReader reader(optionsIniFile);

	if (reader.ParseError() < 0)
	{
		cout << "Can't load '" << optionsIniFile << "'\n";
		return;
	}
}

// Save options
void QubeSettings::SaveOptions()
{
	ofstream file;

	// Open the file
	string optionsIniFile = "media/config/options.ini";
	file.open(optionsIniFile.c_str(), ios::out);
}