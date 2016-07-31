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

bool wireframe = false;
bool lighting = true;
bool shadows = true;
int ivar = 12345678;
double dvar = 3.1415926;
float fvar = (float)dvar;
std::string strval = "A string";
test_enum enumval = Item2;
Color colval(0.5f, 0.5f, 0.7f, 1.f);

void QubeGame::CreateGUI()
{
	Window *window = new Window(m_pNanoGUIScreen, "Controls");
	window->setSize(Vector2i(175, 300));
	window->setPosition(Vector2i(10, 150));
	//window->setLayout(new GroupLayout());

	Label *l = new Label(window, "Information", "arial");
	l->setPosition(Vector2i(10, 33));
	m_matricesInformationLabel = new Label(window, "[MATRICES]", "arial");
	m_matricesInformationLabel->setFontSize(13);
	m_matricesInformationLabel->setPosition(Vector2i(20, 50));
	m_verticesInformationLabel = new Label(window, "[VERTICEES]", "arial");
	m_verticesInformationLabel->setFontSize(13);
	m_verticesInformationLabel->setPosition(Vector2i(20, 63));
	m_trianglesInformationLabel = new Label(window, "[TRIANGLES]", "arial");
	m_trianglesInformationLabel->setFontSize(13);
	m_trianglesInformationLabel->setPosition(Vector2i(20, 76));

	l = new Label(window, "Rendering", "arial");
	l->setPosition(Vector2i(10, 108));
	CheckBox *cb = new CheckBox(window, "Wireframe", [](bool state) { wireframe = state; });
	cb->setTooltip("Wireframe rendering.");
	cb->setPosition(Vector2i(20, 125));
	cb = new CheckBox(window, "Lighting", [](bool state) { lighting = state; });
	cb->setTooltip("Lighting rendering.");
	cb->setPosition(Vector2i(20, 147));
	cb = new CheckBox(window, "Shadow", [](bool state) { shadows = state; });
	cb->setTooltip("Shadows rendering.");
	cb->setPosition(Vector2i(20, 169));

	l = new Label(window, "File Operations", "arial");
	l->setPosition(Vector2i(10, 201));
	Button *b = new Button(window, "Open");
	b->setPosition(Vector2i(20, 222));
	b->setCallback([&] {
		string fileName = file_dialog({ { "qbt", "Qubicle Binary Tree" } }, false);
		if (fileName != "")
		{
			m_pQBTFile->Unload();
			m_pQBTFile->LoadQBTFile(fileName);
		}
	});
	b = new Button(window, "Save");
	b->setPosition(Vector2i(90, 222));
	b->setCallback([&] {
		string fileName = file_dialog({ { "qbt", "Qubicle Binary Tree" }, }, true);
	});

	window->setVisible(true);

	m_pNanoGUIScreen->setVisible(true);
	m_pNanoGUIScreen->performLayout();
}

void QubeGame::DestroyGUI()
{
	delete m_pNanoGUIScreen;
}

void QubeGame::UpdateGUI()
{
	m_pQBTFile->SetWireframeMode(wireframe);

	string matrices = "Number of matrices: " + to_string(m_pQBTFile->GetNumMatrices());
	m_matricesInformationLabel->setCaption(matrices);

	string vertices = "Number of vertices: " + to_string(m_pQBTFile->GetNumVertices());
	m_verticesInformationLabel->setCaption(vertices);

	string triangles = "Number of triangles: " + to_string(m_pQBTFile->GetNumTriangles());
	m_trianglesInformationLabel->setCaption(triangles);
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
