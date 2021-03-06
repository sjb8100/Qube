// ******************************************************************************
// Filename:    QubeUpdate.cpp
// Project:     Qube
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 11/04/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "QubeGame.h"

#include <iostream>
using namespace std;


#ifdef __linux__
#include <sys/time.h>
#endif //__linux__

// Updating
void QubeGame::Update()
{
	// FPS
#ifdef _WIN32
	QueryPerformanceCounter(&m_fpsCurrentTicks);
	m_deltaTime = ((float)(m_fpsCurrentTicks.QuadPart - m_fpsPreviousTicks.QuadPart) / (float)m_fpsTicksPerSecond.QuadPart);
#else
	struct timeval tm;
	gettimeofday(&tm, NULL);
	m_fpsCurrentTicks = (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
	m_deltaTime = (m_fpsCurrentTicks - m_fpsPreviousTicks);
#endif //_WIN32
	m_fps = 1.0f / m_deltaTime;
	m_fpsPreviousTicks = m_fpsCurrentTicks;

	m_glfwTime = m_pQubeWindow->GetTime();
	updateGraph(&m_fpsGraph, m_deltaTime);

	float maxDeltaTime = 0.25f;
	if (m_deltaTime > maxDeltaTime)
	{
		cout << "Warning: DeltaTime exceeded sensible value, switching dt from " << m_deltaTime << " to " << maxDeltaTime << ".\n";
		m_deltaTime = maxDeltaTime;
	}

	// Update the initial wait timer and variables, so we dont do gameplay updates straight away
	if (m_initialStartWait == true)
	{
		if (m_initialWaitTimer > m_initialWaitTime)
		{
			m_initialStartWait = false;
		}
		else
		{
			m_initialWaitTimer += m_deltaTime;
			m_initialStartWait = true;
		}
	}

	// Update the lights
	UpdateLights(m_deltaTime);

	// Update the GUI
	int x = m_pQubeWindow->GetCursorX();
	int y = m_pQubeWindow->GetCursorY();
	UpdateGameGUI(m_deltaTime);

	// Main components update
	if (m_bPaused == false && m_initialStartWait == false)
	{
	}

	// Update controls
	UpdateControls(m_deltaTime);

	// Update the dynamic camera zoom
	UpdateCameraZoom(m_deltaTime);
	
	// Update the camera
	UpdateCamera(m_deltaTime);

	// Update the application and window
	m_pQubeWindow->Update(m_deltaTime);
}

void QubeGame::UpdateLights(float dt)
{
	if (m_bLightMovement)
	{
		m_lightTimer += dt;
		float radius = 15.0f;
		float movementSpeed = 5.0f;
		float angle = DegToRad((m_lightTimer/ movementSpeed) * 360.0f);
		vec3 lightPos = vec3(cos(angle) * radius, sin(angle * 1.25f) * radius, sin(angle) * radius);
		
		m_pDefaultLight->m_position = lightPos;
	}
	else
	{
	}
}

void QubeGame::UpdateNamePicking()
{
}

void QubeGame::UpdateGameGUI(float dt)
{
	UpdateGUI();
}
