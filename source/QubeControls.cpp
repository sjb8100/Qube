// ******************************************************************************
// Filename:    QubeControls.cpp
// Project:     Qube
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 27/10/15
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "QubeGame.h"

// Controls
void QubeGame::UpdateControls(float dt)
{
	if (m_gamepadMovement == false)
	{
		UpdateKeyboardControls(dt);
		UpdateMouseControls(dt);
	}

	if (m_keyboardMovement == false)
	{
		if (m_pQubeWindow->IsJoyStickConnected(0))
		{
			UpdateGamePadControls(dt);
		}
	}
}

void QubeGame::UpdateKeyboardControls(float dt)
{
	GameMode gameMode = GetGameMode();

	if (gameMode == GameMode_Debug || m_cameraMode == CameraMode_Debug)
	{
	}
}

void QubeGame::UpdateMouseControls(float dt)
{
	GameMode gameMode = GetGameMode();

	if (gameMode == GameMode_Debug || m_cameraMode == CameraMode_Debug)
	{
		if (m_bCameraRotate)
		{
			MouseCameraRotate();
		}
	}
}

void QubeGame::UpdateGamePadControls(float dt)
{
	JoystickCameraZoom(dt);
}
