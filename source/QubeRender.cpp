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

#include "nanovg/nanovg.h"
#include "nanovg/perf.h"


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

	// Start timings
	startGPUTimer(&m_gpuTimer);


	// Start the scene
	m_pRenderer->SetClearColour(0.2f, 0.3f, 0.4f, 1.0f);
	m_pRenderer->ClearScene();

	m_pRenderer->ResetLines();

	// Set viewport
	m_pRenderer->SetViewport(m_pDefaultViewport);

	m_pRenderer->DrawLine(vec3(0.0f, 0.0f, 0.0f), vec3(5.0f, 0.0f, 0.0f), Colour(1.0f, 0.0f, 0.0f), Colour(1.0f, 0.0f, 0.0f));
	m_pRenderer->DrawLine(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 5.0f, 0.0f), Colour(0.0f, 1.0f, 0.0f), Colour(0.0f, 1.0f, 0.0f));
	m_pRenderer->DrawLine(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 5.0f), Colour(0.0f, 0.0f, 1.0f), Colour(0.0f, 0.0f, 1.0f));

	for (int i = 0; i < 1; i++)
	{
		m_pRenderer->DrawCube(vec3(i, 0.0f, 0.0f), 1.0f, 1.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f));
	}

	m_pRenderer->RenderLines(m_pGameCamera);

	// Render debug information
	//RenderDebugInformation();
	
	// Render nanovg
	RenderNanoVG();

	// Render the nanogui
	//RenderNanoGUI();

	// Stop timings
	m_cpuTime = m_pQubeWindow->GetTime() - m_glfwTime;
	updateGraph(&m_cpuGraph, (float)m_cpuTime);

	// Update GPU graphs
	float gpuTimes[3];
	unsigned int n = stopGPUTimer(&m_gpuTimer, gpuTimes, 3);
	for (unsigned int i = 0; i < n; i++)
		updateGraph(&m_gpuGraph, gpuTimes[i]);


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

void QubeGame::RenderNanoVG()
{
	float pxRatio = (float)m_windowWidth / (float)m_windowHeight;
	nvgBeginFrame(m_pNanovg, m_windowWidth, m_windowHeight, pxRatio);

	renderGraph(m_pNanovg, 5, 5, &m_fpsGraph);
	renderGraph(m_pNanovg, 5 + 200 + 5, 5, &m_cpuGraph);
	if (m_gpuTimer.supported)
	{
		renderGraph(m_pNanovg, 5 + 200 + 5 + 200 + 5, 5, &m_gpuGraph);
	}

	nvgEndFrame(m_pNanovg);
}

void QubeGame::RenderNanoGUI()
{
	//m_pNanoGUIScreen->drawContents();
	//m_pNanoGUIScreen->drawWidgets();
}