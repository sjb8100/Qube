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
bool innerVoxels = false;
bool innerFaces = false;
bool mergeFaces = true;
bool lightMovement = false;
bool lightColorLock = true;

void QubeGame::CreateGUI()
{
	// Controls window
	m_pControlsWindow = new Window(m_pNanoGUIScreen, "Controls");
	m_pControlsWindow->setSize(Vector2i(175, 350));
	m_pControlsWindow->setPosition(Vector2i(10, 125));

	// Information
	Label *l = new Label(m_pControlsWindow, "Information", "arial");
	l->setPosition(Vector2i(10, 33));
	m_matricesInformationLabel = new Label(m_pControlsWindow, "[MATRICES]", "arial");
	m_matricesInformationLabel->setFontSize(13);
	m_matricesInformationLabel->setPosition(Vector2i(20, 50));
	m_verticesInformationLabel = new Label(m_pControlsWindow, "[VERTICEES]", "arial");
	m_verticesInformationLabel->setFontSize(13);
	m_verticesInformationLabel->setPosition(Vector2i(20, 63));
	m_trianglesInformationLabel = new Label(m_pControlsWindow, "[TRIANGLES]", "arial");
	m_trianglesInformationLabel->setFontSize(13);
	m_trianglesInformationLabel->setPosition(Vector2i(20, 76));

	// Rendering
	l = new Label(m_pControlsWindow, "Rendering", "arial");
	l->setPosition(Vector2i(10, 108));
	CheckBox *cb = new CheckBox(m_pControlsWindow, "Wireframe", [](bool state) { wireframe = state; });
	cb->setChecked(wireframe);
	cb->setTooltip("Wireframe rendering.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 125));
	cb = new CheckBox(m_pControlsWindow, "Lighting", [](bool state) { lighting = state; });
	cb->setChecked(lighting);
	cb->setTooltip("Lighting rendering.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 147));
	cb = new CheckBox(m_pControlsWindow, "Shadow", [](bool state) { shadows = state; });
	cb->setChecked(shadows);
	cb->setTooltip("Shadows rendering.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 169));
	cb = new CheckBox(m_pControlsWindow, "Inner Voxels");
	cb->setChecked(innerVoxels);
	cb->setCallback([&](bool state)
	{
		innerVoxels = state;
		m_pQBTFile->SetCreateInnerVoxels(innerVoxels);
		m_pQBTFile->RecreateStaticBuffers();
	});
	cb->setTooltip("Render the inner voxels.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 191));
	cb = new CheckBox(m_pControlsWindow, "Inner Faces");
	cb->setChecked(innerFaces);
	cb->setCallback([&](bool state)
	{
		innerFaces = state;
		m_pQBTFile->SetCreateInnerFaces(innerFaces);
		m_pQBTFile->RecreateStaticBuffers();
	});
	cb->setTooltip("Render the inner faces.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 213));
	cb = new CheckBox(m_pControlsWindow, "Face Merging");
	cb->setChecked(mergeFaces);
	cb->setCallback([&](bool state)
	{
		mergeFaces = state;
		m_pQBTFile->SetMergeFaces(mergeFaces);
		m_pQBTFile->RecreateStaticBuffers();
	});
	cb->setTooltip("Voxel face merging.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 235));

	l = new Label(m_pControlsWindow, "File Operations", "arial");
	l->setPosition(Vector2i(10, 267));
	Button *b = new Button(m_pControlsWindow, "Open");
	b->setFontSize(18);
	b->setPosition(Vector2i(20, 288));
	b->setCallback([&]
	{
		string fileName = file_dialog({ { "qbt", "Qubicle Binary Tree" } }, false);
		if (fileName != "")
		{
			m_pQBTFile->Unload();
			m_pQBTFile->LoadQBTFile(fileName);
		}
	});
	b = new Button(m_pControlsWindow, "Save");
	b->setFontSize(18);
	b->setPosition(Vector2i(85, 288));
	b->setCallback([&]
	{
		string fileName = file_dialog({ { "qbt", "Qubicle Binary Tree" }, }, true);
	});


	// Light window
	m_pLightWindow = new Window(m_pNanoGUIScreen, "Light");
	m_pLightWindow->setSize(Vector2i(175, 350));
	m_pLightWindow->setPosition(Vector2i(10, 500));

	cb = new CheckBox(m_pLightWindow, "Movement", [](bool state) { lightMovement = state; });
	cb->setChecked(lightMovement);
	cb->setTooltip("Light movement.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 33));

	cb = new CheckBox(m_pLightWindow, "Color Lock", [](bool state) { lightColorLock = state; });
	cb->setChecked(lightColorLock);
	cb->setTooltip("Lock the light colour for ambient, diffuse and specular to be the same.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 55));

	// Ambient
	l = new Label(m_pLightWindow, "Ambient:", "arial");
	l->setPosition(Vector2i(20, 84));
	l->setFontSize(14);
	PopupButton *popupBtn = new PopupButton(m_pLightWindow, "", 0);
	popupBtn->setBackgroundColor(Color(255, 255, 255, 255));
	popupBtn->setFontSize(16);
	popupBtn->setFixedSize(Vector2i(80, 20));
	popupBtn->setPosition(Vector2i(80, 82));
	Popup *popup = popupBtn->popup();
	popup->setLayout(new GroupLayout());

	ColorWheel *colorwheel = new ColorWheel(popup);
	colorwheel->setColor(popupBtn->backgroundColor());

	colorwheel->setCallback([this, popupBtn](const Color &value)
	{
		popupBtn->setBackgroundColor(value);
		m_pDefaultLight->m_ambient = Colour(value.r(), value.g(), value.b());
		if (lightColorLock)
		{
			m_pDefaultLight->m_diffuse = Colour(value.r(), value.g(), value.b());
			m_pDefaultLight->m_specular = Colour(value.r(), value.g(), value.b());
		}
	});

	// Diffuse
	l = new Label(m_pLightWindow, "Diffuse:", "arial");
	l->setPosition(Vector2i(20, 108));
	l->setFontSize(14);
	popupBtn = new PopupButton(m_pLightWindow, "", 0);
	popupBtn->setBackgroundColor(Color(255, 255, 255, 255));
	popupBtn->setFontSize(16);
	popupBtn->setFixedSize(Vector2i(80, 20));
	popupBtn->setPosition(Vector2i(80, 106));
	popup = popupBtn->popup();
	popup->setLayout(new GroupLayout());

	colorwheel = new ColorWheel(popup);
	colorwheel->setColor(popupBtn->backgroundColor());

	colorwheel->setCallback([this, popupBtn](const Color &value)
	{
		popupBtn->setBackgroundColor(value);
		m_pDefaultLight->m_diffuse = Colour(value.r(), value.g(), value.b());
		if (lightColorLock)
		{
			m_pDefaultLight->m_ambient = Colour(value.r(), value.g(), value.b());
			m_pDefaultLight->m_specular = Colour(value.r(), value.g(), value.b());
		}
	});

	// Specular
	l = new Label(m_pLightWindow, "Specular:", "arial");
	l->setPosition(Vector2i(20, 132));
	l->setFontSize(14);
	popupBtn = new PopupButton(m_pLightWindow, "", 0);
	popupBtn->setBackgroundColor(Color(255, 255, 255, 255));
	popupBtn->setFontSize(16);
	popupBtn->setFixedSize(Vector2i(80, 20));
	popupBtn->setPosition(Vector2i(80, 130));
	popup = popupBtn->popup();
	popup->setLayout(new GroupLayout());

	colorwheel = new ColorWheel(popup);
	colorwheel->setColor(popupBtn->backgroundColor());

	colorwheel->setCallback([this, popupBtn](const Color &value)
	{
		popupBtn->setBackgroundColor(value);
		m_pDefaultLight->m_specular = Colour(value.r(), value.g(), value.b());
		if (lightColorLock)
		{
			m_pDefaultLight->m_ambient = Colour(value.r(), value.g(), value.b());
			m_pDefaultLight->m_diffuse = Colour(value.r(), value.g(), value.b());
		}
	});

	m_pControlsWindow->setVisible(true);
	m_pLightWindow->setVisible(true);
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
	m_pQBTFile->SetUseLighting(lighting);
	m_pQBTFile->SetCreateInnerVoxels(innerVoxels);
	m_pQBTFile->SetCreateInnerFaces(innerFaces);
	m_pQBTFile->SetMergeFaces(mergeFaces);

	m_pControlsWindow->setTitle(m_pQBTFile->GetFilename());

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
