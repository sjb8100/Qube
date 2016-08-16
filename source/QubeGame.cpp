// ******************************************************************************
// Filename:    QubeGame.cpp
// Project:     Qube
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 11/04/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "glew/include/GL/glew.h"
#include "QubeGame.h"
#include <glm/detail/func_geometric.hpp>

#include <nanogui/nanogui.h>
using namespace nanogui;

#include "nanovg/nanovg_gl.h"
#include "nanovg/perf.h"

#ifdef __linux__
#include <sys/time.h>
#endif //__linux__

#include "qbt/QBT.h"


// Initialize the singleton instance
QubeGame *QubeGame::c_instance = 0;

QubeGame* QubeGame::GetInstance()
{
	if (c_instance == 0)
		c_instance = new QubeGame;

	return c_instance;
}

// Creation
void QubeGame::Create(QubeSettings* pQubeSettings)
{
	m_pRenderer = NULL;
	m_pNanovg = NULL;
	m_pNanoGUIScreen = NULL;

	m_pQubeSettings = pQubeSettings;
	m_pQubeWindow = new QubeWindow(this, m_pQubeSettings);

	/* Create the GLFW window */
	m_pQubeWindow->Create();

	/* Setup the FPS and deltatime counters */
#ifdef _WIN32
	QueryPerformanceCounter(&m_fpsPreviousTicks);
	QueryPerformanceCounter(&m_fpsCurrentTicks);
	QueryPerformanceFrequency(&m_fpsTicksPerSecond);
#else
	struct timeval tm;
	gettimeofday(&tm, NULL);
	m_fpsCurrentTicks = (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
	m_fpsPreviousTicks = (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
#endif //_WIN32
	m_deltaTime = 0.0f;
	m_fps = 0.0f;

	/* Mouse name picking */
	m_pickedObject = -1;
	m_bNamePickingSelected = false;

	/* Setup the initial starting wait timing */
	m_initialWaitTimer = 0.0f;
	m_initialWaitTime = 0.5f;
	m_initialStartWait = true;

	/* Create the renderer */
	m_windowWidth = m_pQubeWindow->GetWindowWidth();
	m_windowHeight = m_pQubeWindow->GetWindowHeight();
	m_pRenderer = new Renderer(m_windowWidth, m_windowHeight);

	/* Create camera */
	m_pGameCamera = new Camera(m_pRenderer);
	m_pGameCamera->SetupCameraFromPositionAndView(vec3(10.0f, 10.0f, 10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	/* Create viewports */
	m_pDefaultViewport = m_pRenderer->CreateViewport(0, 0, m_windowWidth, m_windowHeight, 60.0f);

	/* Create lights */
	m_pDefaultLight = new Light();
	m_pDefaultLight->m_position = vec3(30.0f, 30.0f, 30.0f);
	m_pDefaultLight->m_ambient = Colour(0.2f, 0.2f, 0.2f);
	m_pDefaultLight->m_diffuse = Colour(0.8f, 0.8f, 0.8f);
	m_pDefaultLight->m_specular = Colour(0.8f, 0.8f, 0.8f);
	m_pDefaultLight->m_constantAttenuation = 1.0f;
	m_pDefaultLight->m_linearAttenuation = 0.0f;
	m_pDefaultLight->m_quadraticAttenuation = 0.0f;
	m_bLightMovement = false;
	m_lightTimer = 0.0f;

	/* Create the nanogui */
	m_pNanoGUIScreen = new Screen();
	m_pNanoGUIScreen->initialize(m_pQubeWindow->GetGLFWwindow(), true);

	/* Create the GUI objects */
	CreateGUI();

	/* Create the nanovg context */
	m_pNanovg = m_pNanoGUIScreen->nvgContext();
	int font = nvgCreateFont(m_pNanovg, "arial", "media/fonts/arial.ttf");
	initGraph(&m_fpsGraph, GRAPH_RENDER_FPS, "Frame Time");
	initGraph(&m_cpuGraph, GRAPH_RENDER_MS, "CPU Time");
	initGraph(&m_gpuGraph, GRAPH_RENDER_MS, "GPU Time");
	initGPUTimer(&m_gpuTimer);

	/* QBT File */
	m_pQBTFile = new QBT(m_pRenderer);
	m_pQBTFile->LoadQBTFile("media/assets/qbt/ground_tile1.qbt");

	/* Pause and quit */
	m_bGameQuit = false;
	m_bPaused = false;

	/* Keyboard movement */
	m_bKeyboardForward = false;
	m_bKeyboardBackward = false;
	m_bKeyboardStrafeLeft = false;
	m_bKeyboardStrafeRight = false;
	m_bKeyboardLeft = false;
	m_bKeyboardRight = false;
	m_bKeyboardUp = false;
	m_bKeyboardDown = false;
	m_bKeyboardSpace = false;
	m_bKeyboardMenu = false;

	/* Joystick flags */
	m_bJoystickJump = false;

	/* Camera movement */
	m_bCameraRotate = false;
	m_pressedX = 0;
	m_pressedY = 0;
	m_currentX = 0;
	m_currentY = 0;
	m_cameraDistance = 10.0f;
	m_maxCameraDistance = m_cameraDistance;

	/* Movement */
	m_keyboardMovement = false;
	m_gamepadMovement = false;

	/* Camera mode */
	m_cameraMode = CameraMode_Debug;
	m_previousCameraMode = CameraMode_Debug;

	/* Game mode */
	m_gameMode = GameMode_Loading;
	m_allowToChangeToGame = true;
	m_allowToChangeToFrontend = true;
	SetGameMode(m_gameMode);

	/* Set game and camera modes */
	SetGameMode(GameMode_Debug);
	SetCameraMode(CameraMode_Debug);

	/* Show the window */
	m_pQubeWindow->ShowWindow();

	/* Set the initial glfw time */
	m_pQubeWindow->SetTime(0);
}

// Destruction
void QubeGame::Destroy()
{
	if (c_instance)
	{
		DestroyGUI();

		delete m_pGameCamera;
		delete m_pDefaultViewport;
		delete m_pDefaultLight;

		delete m_pRenderer;

		m_pQubeWindow->Destroy();
		delete m_pQubeWindow;

		delete c_instance;
	}
}

// Events
void QubeGame::PollEvents()
{
	m_pQubeWindow->PollEvents();
}

bool QubeGame::ShouldClose()
{
	return m_bGameQuit;
}

// Window functionality
int QubeGame::GetWindowCursorX()
{
	return m_pQubeWindow->GetCursorX();
}

int QubeGame::GetWindowCursorY()
{
	return m_pQubeWindow->GetCursorY();
}


void QubeGame::TurnCursorOn(bool resetCursorPosition, bool forceOn)
{
	m_pQubeWindow->TurnCursorOn(resetCursorPosition, forceOn);
}

void QubeGame::TurnCursorOff(bool forceOff)
{
	m_pQubeWindow->TurnCursorOff(forceOff);

	// Make sure to set the current X and Y when we turn the cursor off, so that camera controls don't glitch.
	m_currentX = m_pQubeWindow->GetCursorX();
	m_currentY = m_pQubeWindow->GetCursorY();
}

bool QubeGame::IsCursorOn()
{
	return m_pQubeWindow->IsCursorOn();
}

void QubeGame::ResizeWindow(int width, int height)
{
	m_windowWidth = width;
	m_windowHeight = height;

	m_pQubeWindow->ResizeWindow(m_windowWidth, m_windowHeight);

	if(m_pRenderer)
	{
		// Let the renderer know we have resized the window
		m_pRenderer->ResizeWindow(m_windowWidth, m_windowHeight);

		// Resize the viewports
		m_pRenderer->ResizeViewport(m_pDefaultViewport, 0, 0, m_windowWidth, m_windowHeight, 60.0f);
	}
}

int QubeGame::GetWindowWidth()
{
	return m_windowWidth;
}

int QubeGame::GetWindowHeight()
{
	return m_windowHeight;
}

void QubeGame::CloseWindow()
{
	m_bGameQuit = true;
}

void QubeGame::UpdateJoySticks()
{
	m_pQubeWindow->UpdateJoySticks();
}

// Game functions
void QubeGame::QuitToFrontEnd()
{
	TurnCursorOn(true, false);
	SetGameMode(GameMode_FrontEnd);

	m_pQubeWindow->Update(m_deltaTime);
}

void QubeGame::SetupDataForGame()
{
}

void QubeGame::SetupDataForFrontEnd()
{
}

void QubeGame::StartGameFromFrontEnd()
{
	m_previousCameraMode = CameraMode_MouseRotate;
}

void QubeGame::SetGameMode(GameMode mode)
{
	GameMode previousgameMode = m_gameMode;
	m_gameMode = mode;

	if (m_gameMode == GameMode_Debug)
	{
	}

	if (m_gameMode == GameMode_FrontEnd)
	{
		if (previousgameMode == GameMode_Game || previousgameMode == GameMode_Loading)
		{
			// Setup the gamedata since we have just loaded fresh into the frontend.
			SetupDataForFrontEnd();
		}
	}

	if (m_gameMode == GameMode_Game)
	{
		if (previousgameMode == GameMode_FrontEnd || previousgameMode == GameMode_Loading)
		{
			// Setup the gamedata since we have just loaded fresh into a game.
			SetupDataForGame();
		}
	}
}

GameMode QubeGame::GetGameMode()
{
	return m_gameMode;
}

void QubeGame::SetCameraMode(CameraMode mode)
{
	m_cameraMode = mode;
}

CameraMode QubeGame::GetCameraMode()
{
	return m_cameraMode;
}

// Accessors
QubeSettings* QubeGame::GetQubeSettings()
{
	return m_pQubeSettings;
}

Screen* QubeGame::GetNanoGUIScreen()
{
	return m_pNanoGUIScreen;
}

QBT* QubeGame::GetQBTModel()
{
	return m_pQBTFile;
}