// ******************************************************************************
// Filename:    QubeInput.cpp
// Project:     Qube
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 11/04/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
using namespace std;

#include "QubeGame.h"


// Input callbacks
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (action)
	{
		case GLFW_PRESS:
		{
			QubeGame::GetInstance()->KeyPressed(key, scancode, mods);
			
			break;
		}
		case GLFW_RELEASE:
		{
			QubeGame::GetInstance()->KeyReleased(key, scancode, mods);
			break;
		}
		case GLFW_REPEAT:
		{
			break;
		}
	}
}

void CharacterCallback(GLFWwindow* window, unsigned int keyCode)
{
	QubeGame::GetInstance()->CharacterEntered(keyCode);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (action)
	{
		case GLFW_PRESS:
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT)
				QubeGame::GetInstance()->MouseLeftPressed();
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
				QubeGame::GetInstance()->MouseRightPressed();
			if (button == GLFW_MOUSE_BUTTON_MIDDLE)
				QubeGame::GetInstance()->MouseMiddlePressed();

			break;
		}
		case GLFW_RELEASE:
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT)
				QubeGame::GetInstance()->MouseLeftReleased();
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
				QubeGame::GetInstance()->MouseRightReleased();
			if (button == GLFW_MOUSE_BUTTON_MIDDLE)
				QubeGame::GetInstance()->MouseMiddleReleased();

			break;
		}
	}
}

void MouseScrollCallback(GLFWwindow* window, double x, double y)
{
	QubeGame::GetInstance()->MouseScroll(x, y);
}

// Input
void QubeGame::KeyPressed(int key, int scancode, int mods)
{
	switch (key)
	{
		case GLFW_KEY_W:
		{
			m_bKeyboardForward = true;
			break;
		}
		case GLFW_KEY_S:
		{
			m_bKeyboardBackward = true;
			break;
		}
		case GLFW_KEY_A:
		{
			m_bKeyboardLeft = true;
			m_bKeyboardStrafeLeft = true;
			break;
		}
		case GLFW_KEY_D:
		{
			m_bKeyboardRight = true;
			m_bKeyboardStrafeRight = true;
			break;
		}
		case GLFW_KEY_F:
		{
			m_bKeyboardUp = true;
			break;
		}
		case GLFW_KEY_V:
		{
			m_bKeyboardDown = true;
			break;
		}
		case GLFW_KEY_SPACE:
		{
			m_bKeyboardSpace = true;
			break;
		}
		case GLFW_KEY_ESCAPE:
		{
			m_bKeyboardMenu = true;
			break;
		}		
	}
}

void QubeGame::KeyReleased(int key, int scancode, int mods)
{
	switch (key)
	{
		case GLFW_KEY_W:
		{
			m_bKeyboardForward = false;
			break;
		}
		case GLFW_KEY_S:
		{
			m_bKeyboardBackward = false;
			break;
		}
		case GLFW_KEY_A:
		{
			m_bKeyboardLeft = false;
			m_bKeyboardStrafeLeft = false;
			break;
		}
		case GLFW_KEY_D:
		{
			m_bKeyboardRight = false;
			m_bKeyboardStrafeRight = false;
			break;
		}
		case GLFW_KEY_F:
		{
			m_bKeyboardUp = false;
			break;
		}
		case GLFW_KEY_V:
		{
			m_bKeyboardDown = false;
			break;
		}
		case GLFW_KEY_SPACE:
		{
			m_bKeyboardSpace = false;
			break;
		}
		case GLFW_KEY_ESCAPE:
		{
			m_bKeyboardMenu = false;
			break;
		}
	}
}

void QubeGame::CharacterEntered(int keyCode)
{
}

void QubeGame::MouseLeftPressed()
{
	if (IsCursorOn() == false)
	{
		m_currentX = m_pQubeWindow->GetCursorX();
		m_currentY = m_pQubeWindow->GetCursorY();
		m_pressedX = m_currentX;
		m_pressedY = m_currentY;

		if (m_gameMode == GameMode_Debug || m_cameraMode == CameraMode_Debug)
		{
			// Turn cursor off
			if (IsCursorOn() == true)
			{
				TurnCursorOff(false);
			}

			m_bCameraRotate = true;
		}
	}
}

void QubeGame::MouseLeftReleased()
{
	if (m_gameMode == GameMode_Debug || m_cameraMode == CameraMode_Debug)
	{
		// Turn cursor on
		if (IsCursorOn() == false)
		{
			TurnCursorOn(true, false);
		}

		m_bCameraRotate = false;
	}
}

void QubeGame::MouseRightPressed()
{
	if (IsCursorOn() == false)
	{
		m_currentX = m_pQubeWindow->GetCursorX();
		m_currentY = m_pQubeWindow->GetCursorY();
		m_pressedX = m_currentX;
		m_pressedY = m_currentY;
	}
}

void QubeGame::MouseRightReleased()
{
}

void QubeGame::MouseMiddlePressed()
{
}

void QubeGame::MouseMiddleReleased()
{
}

void QubeGame::MouseScroll(double x, double y)
{
	GameMode gameMode = GetGameMode();

	if (IsCursorOn() == false)
	{
		m_maxCameraDistance += (float)(-y*0.5f);

		WrapCameraZoomValue();
	}
}

void QubeGame::WrapCameraZoomValue()
{
	float minAmount = 1.5f;
	float maxAmount = 15.0f;

	// Camera rotation modes
	if (m_gameMode == GameMode_Game && m_cameraMode == CameraMode_MouseRotate)
	{
		minAmount = 1.0f;
		maxAmount = 15.0f;
	}

	if (m_maxCameraDistance <= minAmount)
	{
		m_maxCameraDistance = minAmount;
	}

	if (m_maxCameraDistance >= maxAmount)
	{
		m_maxCameraDistance = maxAmount;
	}
}

// Mouse controls
void QubeGame::MouseCameraRotate()
{
}

// Joystick controls
void QubeGame::JoystickCameraMove(float dt)
{
}

void QubeGame::JoystickCameraRotate(float dt)
{
}

void QubeGame::JoystickCameraZoom(float dt)
{
	bool zoomOut = m_pQubeWindow->GetJoystickButton(0, 4);
	bool zoomIn = m_pQubeWindow->GetJoystickButton(0, 5);

	float zoomAmount = 0.0f;
	if (zoomIn)
	{
		zoomAmount = 10.0f;
	}
	if (zoomOut)
	{
		zoomAmount = -10.0f;
	}

	float changeY = zoomAmount * dt;

	if (changeY < 0.0f)
	{
		m_cameraDistance = 2.0f;
		m_maxCameraDistance = 2.0f;
	}

	WrapCameraZoomValue();
}