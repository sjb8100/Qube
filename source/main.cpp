// ******************************************************************************
// Filename:    main.cpp
// Project:     Qube
// Author:      Steven Ball
//
// Purpose:
//   The main entry point for the application. Creates the initial instances of
//   the container classes, iterates the game loop, polls events/input and 
//   detects for game closure to cleanup the with the destruction.
// 
// Revision History:
//   Initial Revision - 20/07/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "QubeGame.h"


int main(void)
{
	/* Load the settings */
	QubeSettings* m_pQubeSettings = new QubeSettings();
	m_pQubeSettings->LoadSettings();
	m_pQubeSettings->LoadOptions();

	/* Initialize and create the QubeGame object */
	QubeGame* pQubeGame = QubeGame::GetInstance();
	pQubeGame->Create(m_pQubeSettings);

	/* Loop until the user closes the window or application */
	while (!pQubeGame->ShouldClose())
	{
		/* Poll input events*/
		pQubeGame->PollEvents();

		/* Update joysticks */
		//pQubeGame->UpdateJoySticks();

		/* Update */
		pQubeGame->Update();

		/* PreRender */
		pQubeGame->PreRender();

		/* Render */
		pQubeGame->Render();
	}

	/* Cleanup */
	pQubeGame->Destroy();

	/* Exit */
	exit(EXIT_SUCCESS);
}