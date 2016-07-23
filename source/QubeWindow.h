// ******************************************************************************
// Filename:    QubeWindow.h
// Project:     Qube
// Author:      Steven Ball
//
// Purpose:
//   The Qube window class is an interface and wrapper around the glfw windows
//   library and adds common functionality to bind this to the Qube application.
//   The window class handles most 'window' functionality that isnt directly
//   part of the game, stuff like window resizing, fullscreen, cursors, etc.
// 
// Revision History:
//   Initial Revision - 11/04/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

#include <GLFW/glfw3.h>

class QubeGame;
class QubeSettings;


typedef struct Joystick
{
	bool m_present;
	char* m_name;
	float* m_axes;
	unsigned char* m_buttons;
	int m_axisCount;
	int m_buttonCount;
} Joystick;

class QubeWindow
{
public:
	/* Public methods */
	QubeWindow(QubeGame* pQubeGame, QubeSettings* pQubeSettings);
	~QubeWindow();

	void Create();
	void Destroy();
	void Update(float dt);
	void Render();

	// Initialize window context
	void InitializeWindowContext(GLFWwindow* window);

	// Get a pointer to the GLFWwindow object
	GLFWwindow* GetGLFWwindow();

	// Windows dimensions
	int GetWindowWidth();
	int GetWindowHeight();
	void ResizeWindow(int width, int height);

	// Minimized
	bool GetMinimized();

	// Show / Hide
	void ShowWindow();
	void HideWindow();

	// Title
	void SetWindowTitle(const char* title);

	// Cursor
	int GetCursorX();
	int GetCursorY();
	void SetCursorPosition(int x, int y);
	void TurnCursorOff(bool forceOff);
	void TurnCursorOn(bool resetCursorPosition, bool forceOn);
	bool IsCursorOn();

	// Joysticks
	void UpdateJoySticks();
	bool IsJoyStickConnected(int joyStickNum);
	float GetJoystickAxisValue(int joyStickNum, int axisIndex);
	bool GetJoystickButton(int joyStickNum, int axisIndex);
	float GetJoystickAnalogDeadZone();

	// Fullscreen
	void ToggleFullScreen(bool fullscreen);

	// Events
	void PollEvents();

protected:
	/* Protected methods */

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	QubeGame* m_pQubeGame;
	QubeSettings* m_pQubeSettings;

	// The window object pointer
	GLFWwindow* m_pWindow;

	// Window dimensions
	int m_windowWidth;
	int m_windowHeight;
	int m_oldWindowWidth;
	int m_oldWindowHeight;

	// Minimized flag
	bool m_minimized;

	// Cursor position
	int m_cursorX;
	int m_cursorY;
	int m_cursorOldX;
	int m_cursorOldY;

	// Joysticks
	Joystick m_joysticks[GLFW_JOYSTICK_LAST - GLFW_JOYSTICK_1 + 1];
	int m_joystickCount;
	float m_joystickAnalogDeadZone;
};
