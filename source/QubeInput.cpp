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

#include <glm/glm.hpp>

#include "QubeGame.h"


// Input callbacks
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Nanogui callback handling
	if (QubeGame::GetInstance()->GetNanoGUIScreen())
	{
		QubeGame::GetInstance()->GetNanoGUIScreen()->keyCallbackEvent(key, scancode, action, mods);
	}

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
	// Nanogui callback handling
	if (QubeGame::GetInstance()->GetNanoGUIScreen())
	{
		QubeGame::GetInstance()->GetNanoGUIScreen()->charCallbackEvent(keyCode);
	}

	QubeGame::GetInstance()->CharacterEntered(keyCode);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	// Nanogui callback handling
	if (QubeGame::GetInstance()->GetNanoGUIScreen())
	{
		if (QubeGame::GetInstance()->IsCursorOn() == true)
		{
			QubeGame::GetInstance()->GetNanoGUIScreen()->mouseButtonCallbackEvent(button, action, mods);
		}
	}

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
	// Nanogui callback handling
	if (QubeGame::GetInstance()->GetNanoGUIScreen())
	{
		if (QubeGame::GetInstance()->IsCursorOn() == true)
		{
			QubeGame::GetInstance()->GetNanoGUIScreen()->scrollCallbackEvent(x, y);
		}
	}

	QubeGame::GetInstance()->MouseScroll(x, y);
}

void SetCursorCallback(GLFWwindow* window, double x, double y)
{
	// Nanogui callback handling
	if (QubeGame::GetInstance()->GetNanoGUIScreen())
	{
		if (QubeGame::GetInstance()->IsCursorOn() == true)
		{
			QubeGame::GetInstance()->GetNanoGUIScreen()->cursorPosCallbackEvent(x, y);
		}
	}
}

void FrameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
	// Nanogui callback handling
	if (QubeGame::GetInstance()->GetNanoGUIScreen())
	{
		QubeGame::GetInstance()->GetNanoGUIScreen()->resizeCallbackEvent(width, height);
	}
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
	m_currentX = m_pQubeWindow->GetCursorX();
	m_currentY = m_pQubeWindow->GetCursorY();
	m_pressedX = m_currentX;
	m_pressedY = m_currentY;

	if (IsInteractingWithGUI())
	{
		return;
	}

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

	m_maxCameraDistance += (float)(-y*0.5f);

	WrapCameraZoomValue();
}

void QubeGame::WrapCameraZoomValue()
{
	float minAmount = m_pGameCamera->GetMinZoomAmount();
	float maxAmount = m_pGameCamera->GetMaxZoomAmount();

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
	int x = m_pQubeWindow->GetCursorX();
	int y = m_pQubeWindow->GetCursorY();

	float changeX;
	float changeY;

	// The mouse hasn't moved so just return
	if ((m_currentX == x) && (m_currentY == y))
	{
		return;
	}

	// Calculate and scale down the change in position
	changeX = (x - m_currentX) / 5.0f;
	changeY = (y - m_currentY) / 5.0f;

	// Upside down
	if (m_pGameCamera->GetUp().y < 0.0f)
	{
		changeX = -changeX;
	}

	// Limit the rotation, so we can't go 'over' or 'under' the player with our rotations
	vec3 cameraFacing = m_pGameCamera->GetFacing();
	float dotResult = acos(dot(cameraFacing, vec3(0.0f, 1.0f, 0.0f)));
	float rotationDegrees = RadToDeg(dotResult) - 90.0f;
	float limitAngle = 75.0f;
	if ((rotationDegrees > limitAngle && changeY < 0.0f) || (rotationDegrees < -limitAngle && changeY > 0.0f))
	{
		changeY = 0.0f;
	}

	m_pGameCamera->RotateAroundPoint(changeY*0.75f, 0.0f, 0.0f);
	m_pGameCamera->RotateAroundPointY(-changeX*0.75f);

	m_currentX = x;
	m_currentY = y;
}

// Joystick controls
void QubeGame::JoystickCameraMove(float dt)
{
	float axisX = m_pQubeWindow->GetJoystickAxisValue(0, 0);
	float axisY = m_pQubeWindow->GetJoystickAxisValue(0, 1);

	// Dead zones
	if (fabs(axisX) < m_pQubeWindow->GetJoystickAnalogDeadZone())
	{
		axisX = 0.0f;
	}
	if (fabs(axisY) < m_pQubeWindow->GetJoystickAnalogDeadZone())
	{
		axisY = 0.0f;
	}

	float changeX = axisX * 10.0f * dt;
	float changeY = axisY * 10.0f * dt;

	m_pGameCamera->Fly(-changeY);
	m_pGameCamera->Strafe(changeX);
}

void QubeGame::JoystickCameraRotate(float dt)
{
	float axisX = m_pQubeWindow->GetJoystickAxisValue(0, 4);
	float axisY = m_pQubeWindow->GetJoystickAxisValue(0, 3);

	// Dead zones
	if (fabs(axisX) < m_pQubeWindow->GetJoystickAnalogDeadZone())
	{
		axisX = 0.0f;
	}
	if (fabs(axisY) < m_pQubeWindow->GetJoystickAnalogDeadZone())
	{
		axisY = 0.0f;
	}

	float changeX = axisX * 150.0f * dt;
	float changeY = axisY * 150.0f * dt;

	// Upside down
	if (m_pGameCamera->GetUp().y < 0.0f)
	{
		changeX = -changeX;
	}

	// Limit the rotation, so we can't go 'over' or 'under' the player with our rotations
	vec3 cameraFacing = m_pGameCamera->GetFacing();
	float dotResult = acos(dot(cameraFacing, vec3(0.0f, 1.0f, 0.0f)));
	float rotationDegrees = RadToDeg(dotResult) - 90.0f;
	float limitAngle = 75.0f;
	if ((rotationDegrees > limitAngle && changeY < 0.0f) || (rotationDegrees < -limitAngle && changeY > 0.0f))
	{
		changeY = 0.0f;
	}

	m_pGameCamera->RotateAroundPoint(changeY, 0.0f, 0.0f);
	m_pGameCamera->RotateAroundPointY(-changeX);
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