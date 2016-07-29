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

#include "QubeGame.h"

enum test_enum {
	Item1 = 0,
	Item2,
	Item3
};

bool bvar = true;
int ivar = 12345678;
double dvar = 3.1415926;
float fvar = (float)dvar;
std::string strval = "A string";
test_enum enumval = Item2;
Color colval(0.5f, 0.5f, 0.7f, 1.f);

void QubeGame::CreateGUI()
{
	FormHelper *gui = new FormHelper(m_pNanoGUIScreen);
	nanogui::ref<Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(10, 10), "Test NanoGUI");

	gui->addGroup("Basic types");
	gui->addVariable("wireframe", bvar)->setTooltip("This is a test tooltip, use this at your own risk.");
	gui->addVariable("string", strval);

	gui->addGroup("Validating fields");
	gui->addVariable("int", ivar)->setSpinnable(true);
	gui->addVariable("float", fvar);
	gui->addVariable("double", dvar)->setSpinnable(true);

	gui->addGroup("Complex types");
	gui->addVariable("Enumeration", enumval, true)->setItems({ "Item 1", "Item 2", "Item 3" });
	gui->addVariable("Color", colval);

	gui->addGroup("Other widgets");
	gui->addButton("A button", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Press this button and see what happens.");

	m_pNanoGUIScreen->setVisible(true);
	m_pNanoGUIScreen->performLayout();
	nanoguiWindow->setPosition(Vector2i(10, 150));
}

void QubeGame::UpdateGUI()
{
	m_pQBTFile->SetWireframeMode(!bvar);
}

bool QubeGame::IsInteractingWithGUI()
{
	int currentX = m_pQubeWindow->GetCursorX();
	int currentY = m_pQubeWindow->GetCursorY();

	const Widget *widget = QubeGame::GetInstance()->GetNanoGUIScreen()->findWidget(Vector2i(currentX, currentY));
	if (widget == nullptr || widget == QubeGame::GetInstance()->GetNanoGUIScreen())
	{
		return false;
	}

	return true;
}
