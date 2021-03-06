// ******************************************************************************
// Filename:    QubeGame.h
// Project:     Qube
// Author:      Steven Ball
//
// Purpose:
//   The Qube game class houses all the game functionality and logic that directly
//   interfaces with the game subsystems. Also this game class is the container
//   for all the renderer objects that are required to draw the scene, such as
//   shaders, viewports, frame buffers, etc. Finally this class also owns all
//   the GUI components that are created to handle user input.
//
// Revision History:
//   Initial Revision - 11/04/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "Renderer/Renderer.h"
#include "Renderer/camera.h"
#include "Renderer/light.h"
#include "qbt/QBT.h"
#include "QubeWindow.h"
#include "QubeSettings.h"

#include "nanovg/nanovg.h"
#include "nanovg/perf.h"

#include <nanogui/nanogui.h>
using namespace nanogui;


#ifdef __linux__
typedef struct POINT {
  float x;
  float y;
} POINT;
#endif //__linux__

// Game modes
enum GameMode
{
	GameMode_Debug = 0,
	GameMode_Loading,
	GameMode_FrontEnd,
	GameMode_Game,
};

// Camera modes
enum CameraMode
{
	CameraMode_Debug = 0,
	CameraMode_Frontend,
	CameraMode_MouseRotate,
};


class QubeGame
{
public:
	/* Public methods */
	static QubeGame* GetInstance();

	// Creation
	void Create(QubeSettings* pQubeSettings);
	
	// Destruction
	void Destroy();

	// Events
	void PollEvents();
	bool ShouldClose();

	// Window functionality
	int GetWindowCursorX();
	int GetWindowCursorY();
	void TurnCursorOn(bool resetCursorPosition, bool forceOn);
	void TurnCursorOff(bool forceOff);
	bool IsCursorOn();
	void ResizeWindow(int width, int height);
	int GetWindowWidth();
	int GetWindowHeight();
	void CloseWindow();
	void UpdateJoySticks();

	// Controls
	void UpdateControls(float dt);
	void UpdateKeyboardControls(float dt);
	void UpdateMouseControls(float dt);
	void UpdateGamePadControls(float dt);

	// Camera controls
	void UpdateCamera(float dt);
	void UpdateCameraZoom(float dt);

	// Input
	void KeyPressed(int key, int scancode, int mods);
	void KeyReleased(int key, int scancode, int mods);
	void CharacterEntered(int keyCode);
	void MouseLeftPressed();
	void MouseLeftReleased();
	void MouseRightPressed();
	void MouseRightReleased();
	void MouseMiddlePressed();
	void MouseMiddleReleased();
	void MouseScroll(double x, double y);
	void WrapCameraZoomValue();

	// Mouse controls
	void MouseCameraRotate();

	// Joystick controls
	void JoystickCameraMove(float dt);
	void JoystickCameraRotate(float dt);
	void JoystickCameraZoom(float dt);

	// GUI functionality
	void CreateGUI();
	void DestroyGUI();
	void UpdateGUI();
	bool IsInteractingWithGUI();

	// Game functions
	void QuitToFrontEnd();
	void SetupDataForGame();
	void SetupDataForFrontEnd();
	void StartGameFromFrontEnd();
	void SetGameMode(GameMode mode);
	GameMode GetGameMode();
	void SetCameraMode(CameraMode mode);
	CameraMode GetCameraMode();

	// Updating
	void Update();
	void UpdateLights(float dt);
	void UpdateNamePicking();
	void UpdateGameGUI(float dt);

	// Rendering
	void PreRender();
	void BeginShaderRender();
	void EndShaderRender();
	void Render();
	void RenderDebugInformation();
	void RenderNanoVG();
	void RenderNanoGUI();

	// Accessors
	QubeSettings* GetQubeSettings();
	Screen* GetNanoGUIScreen();
	QBT* GetQBTModel();

protected:
	/* Protected methods */
	QubeGame() {};
	QubeGame(const QubeGame&) {};
	QubeGame &operator=(const QubeGame&) {};
	
private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	QubeWindow* m_pQubeWindow;
	QubeSettings* m_pQubeSettings;

	// Renderer
	Renderer* m_pRenderer;

	// Mouse picking
	int m_pickedObject;
	bool m_bNamePickingSelected;

	// Game mode
	GameMode m_gameMode;
	bool m_allowToChangeToGame;
	bool m_allowToChangeToFrontend;

	// Camera mode
	CameraMode m_cameraMode;
	CameraMode m_previousCameraMode;

	// Window width and height
	int m_windowWidth;
	int m_windowHeight;

	// Quit message
	bool m_bGameQuit;

	// Paused
	bool m_bPaused;

	// FPS and deltatime
#ifdef _WIN32
	LARGE_INTEGER m_fpsPreviousTicks;
	LARGE_INTEGER m_fpsCurrentTicks;
	LARGE_INTEGER m_fpsTicksPerSecond;
#else
	double m_fpsPreviousTicks;
	double m_fpsCurrentTicks;
#endif //_WIN32
	float m_deltaTime;
	float m_fps;
	double m_glfwTime;

	// Initial starting wait timer
	float m_initialWaitTimer;
	float m_initialWaitTime;
	bool m_initialStartWait;

	// Keyboard flags
	bool m_bKeyboardForward;
	bool m_bKeyboardBackward;
	bool m_bKeyboardStrafeLeft;
	bool m_bKeyboardStrafeRight;
	bool m_bKeyboardLeft;
	bool m_bKeyboardRight;
	bool m_bKeyboardUp;
	bool m_bKeyboardDown;
	bool m_bKeyboardSpace;
	bool m_bKeyboardMenu;

	// Joystick flags
	bool m_bJoystickJump;

	// Camera movement
	bool m_bCameraRotate;
	int m_pressedX;
	int m_pressedY;
	int m_currentX;
	int m_currentY;
	float m_cameraDistance;
	float m_maxCameraDistance;

	// Movement
	bool m_keyboardMovement;
	bool m_gamepadMovement;

	// Game camera
	Camera* m_pGameCamera;

	// View ports
	Viewport* m_pDefaultViewport;

	// Lights
	Light* m_pDefaultLight;
	bool m_bLightMovement;
	float m_lightTimer;

	// Nanovg context
	NVGcontext* m_pNanovg;
	
	// Performance and metrics graphs
	PerfGraph m_fpsGraph;
	PerfGraph m_cpuGraph;
	PerfGraph m_gpuGraph;
	GPUtimer m_gpuTimer;
	double m_cpuTime;

	// NanoGUI root screen
	Screen *m_pNanoGUIScreen;
	Window *m_pControlsWindow;
	Window *m_pLightWindow;
	Window *m_pMatrixWindow;
	Label *m_pMatricesInformationLabel;
	Label *m_pVerticesInformationLabel;
	Label *m_pTrianglesInformationLabel;
	ComboBox* m_pMatricesCombo;
	PopupButton *m_pAmbientButton_Light;
	PopupButton *m_pDiffuseButton_Light;
	PopupButton *m_pSpecularButton_Light;
	PopupButton *m_pAmbientButton_Material;
	PopupButton *m_pDiffuseButton_Material;
	PopupButton *m_pSpecularButton_Material;
	PopupButton *m_pEmissionButton_Material;

	// QBT File
	QBT* m_pQBTFile;

	// Singleton instance
	static QubeGame *c_instance;
};
