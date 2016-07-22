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
	sprintf(lBuildInfo, "RELEASE %s", m_pVoxSettings->m_version.c_str());
#endif //defined(_DEBUG) || defined(NDEBUG)

	m_pRenderer->RenderFreeTypeText(m_pDefaultFont, 15.0f, 15.0f, 1.0f, Colour(0.75f, 0.75f, 0.75f), 1.0f, lBuildInfo);
}