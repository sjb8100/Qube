// ******************************************************************************
// Filename:    QubeRender.cpp
// Project:     Qube
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 11/04/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "QubeGame.h"
#include <glm/detail/func_geometric.hpp>

// Rendering
void QubeGame::PreRender()
{
	// Update matrices for game objects
}

void QubeGame::BeginShaderRender()
{
}

void QubeGame::EndShaderRender()
{
}

void QubeGame::Render()
{
	if (m_pQubeWindow->GetMinimized())
	{
		// Don't call any render functions if minimized
		return;
	}

	// Start the scene
	m_pRenderer->SetClearColour(0.2f, 0.3f, 0.4f, 1.0f);
	m_pRenderer->ClearScene();

	m_pRenderer->ResetLines();

	// Set viewport
	m_pRenderer->SetViewport(m_pDefaultViewport);

	m_pRenderer->DrawLine(vec3(0.0f, 0.0f, 0.0f), vec3(5.0f, 0.0f, 0.0f), Colour(1.0f, 0.0f, 0.0f), Colour(1.0f, 0.0f, 0.0f));
	m_pRenderer->DrawLine(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 5.0f, 0.0f), Colour(0.0f, 1.0f, 0.0f), Colour(0.0f, 1.0f, 0.0f));
	m_pRenderer->DrawLine(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 5.0f), Colour(0.0f, 0.0f, 1.0f), Colour(0.0f, 0.0f, 1.0f));

	m_pRenderer->DrawCube(vec3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f));

	m_pRenderer->RenderLines(m_pGameCamera);

	// Render debug information
	RenderDebugInformation();


	// Pass render call to the window class, allow to swap buffers
	m_pQubeWindow->Render();
}

void QubeGame::RenderDebugInformation()
{
	char lBuildInfo[128];
#if defined(_DEBUG) || defined(NDEBUG)
	sprintf(lBuildInfo, "DEV %s", m_pQubeSettings->m_version.c_str());
#else
	sprintf(lBuildInfo, "RELEASE %s", m_pQubeSettings->m_version.c_str());
#endif //defined(_DEBUG) || defined(NDEBUG)

	char lFPSBuff[128];
	float fpsWidthOffset = 65.0f;
	sprintf(lFPSBuff, "FPS: %.0f", m_fps);

	m_pRenderer->RenderFreeTypeText(m_pDefaultFont, 10.0f, 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lBuildInfo);
	m_pRenderer->RenderFreeTypeText(m_pDefaultFont, m_windowWidth - fpsWidthOffset, 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lFPSBuff);
}