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
bool shadows = false;
bool innerVoxels = false;
bool innerFaces = false;
bool mergeFaces = false;
bool lightMovement = false;
bool lightColorLock = false;

void QubeGame::CreateGUI()
{
	// Controls window
	m_pControlsWindow = new Window(m_pNanoGUIScreen, "Controls");
	m_pControlsWindow->setSize(Vector2i(175, 350));
	m_pControlsWindow->setPosition(Vector2i(10, 125));

	// Information
	Label *l = new Label(m_pControlsWindow, "Information", "arial");
	l->setPosition(Vector2i(10, 33));
	m_pMatricesInformationLabel = new Label(m_pControlsWindow, "[MATRICES]", "arial");
	m_pMatricesInformationLabel->setFontSize(13);
	m_pMatricesInformationLabel->setPosition(Vector2i(20, 50));
	m_pVerticesInformationLabel = new Label(m_pControlsWindow, "[VERTICEES]", "arial");
	m_pVerticesInformationLabel->setFontSize(13);
	m_pVerticesInformationLabel->setPosition(Vector2i(20, 63));
	m_pTrianglesInformationLabel = new Label(m_pControlsWindow, "[TRIANGLES]", "arial");
	m_pTrianglesInformationLabel->setFontSize(13);
	m_pTrianglesInformationLabel->setPosition(Vector2i(20, 76));

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

	cb = new CheckBox(m_pLightWindow, "Light Movement", [](bool state) { lightMovement = state; });
	cb->setChecked(lightMovement);
	cb->setTooltip("Light movement.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 33));

	cb = new CheckBox(m_pLightWindow, "Light Color Lock", [](bool state) { lightColorLock = state; });
	cb->setChecked(lightColorLock);
	cb->setTooltip("Lock the light colour for ambient, diffuse and specular to be the same.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 55));

	// Ambient
	l = new Label(m_pLightWindow, "Ambient:", "arial");
	l->setPosition(Vector2i(20, 84));
	l->setFontSize(14);
	m_pAmbientButton = new PopupButton(m_pLightWindow, "", 0);
	m_pAmbientButton->setBackgroundColor(Color(50, 50, 50, 255));
	m_pAmbientButton->setFontSize(16);
	m_pAmbientButton->setFixedSize(Vector2i(80, 20));
	m_pAmbientButton->setPosition(Vector2i(80, 82));
	Popup *popup = m_pAmbientButton->popup();
	popup->setLayout(new GroupLayout());

	ColorWheel *colorwheel = new ColorWheel(popup);
	colorwheel->setColor(m_pAmbientButton->backgroundColor());

	colorwheel->setCallback([this](const Color &value)
	{
		m_pAmbientButton->setBackgroundColor(value);
		m_pDefaultLight->m_ambient = Colour(value.r(), value.g(), value.b());
		if (lightColorLock)
		{
			m_pDiffuseButton->setBackgroundColor(value);
			m_pSpecularButton->setBackgroundColor(value);
			m_pDefaultLight->m_diffuse = Colour(value.r(), value.g(), value.b());
			m_pDefaultLight->m_specular = Colour(value.r(), value.g(), value.b());
		}
	});

	// Diffuse
	l = new Label(m_pLightWindow, "Diffuse:", "arial");
	l->setPosition(Vector2i(20, 108));
	l->setFontSize(14);
	m_pDiffuseButton = new PopupButton(m_pLightWindow, "", 0);
	m_pDiffuseButton->setBackgroundColor(Color(204, 204, 204, 255));
	m_pDiffuseButton->setFontSize(16);
	m_pDiffuseButton->setFixedSize(Vector2i(80, 20));
	m_pDiffuseButton->setPosition(Vector2i(80, 106));
	popup = m_pDiffuseButton->popup();
	popup->setLayout(new GroupLayout());

	colorwheel = new ColorWheel(popup);
	colorwheel->setColor(m_pDiffuseButton->backgroundColor());

	colorwheel->setCallback([this](const Color &value)
	{
		m_pDiffuseButton->setBackgroundColor(value);
		m_pDefaultLight->m_diffuse = Colour(value.r(), value.g(), value.b());
		if (lightColorLock)
		{
			m_pAmbientButton->setBackgroundColor(value);
			m_pSpecularButton->setBackgroundColor(value);
			m_pDefaultLight->m_ambient = Colour(value.r(), value.g(), value.b());
			m_pDefaultLight->m_specular = Colour(value.r(), value.g(), value.b());
		}
	});

	// Specular
	l = new Label(m_pLightWindow, "Specular:", "arial");
	l->setPosition(Vector2i(20, 132));
	l->setFontSize(14);
	m_pSpecularButton = new PopupButton(m_pLightWindow, "", 0);
	m_pSpecularButton->setBackgroundColor(Color(200, 200, 200, 255));
	m_pSpecularButton->setFontSize(16);
	m_pSpecularButton->setFixedSize(Vector2i(80, 20));
	m_pSpecularButton->setPosition(Vector2i(80, 130));
	popup = m_pSpecularButton->popup();
	popup->setLayout(new GroupLayout());

	colorwheel = new ColorWheel(popup);
	colorwheel->setColor(m_pSpecularButton->backgroundColor());

	colorwheel->setCallback([this](const Color &value)
	{
		m_pSpecularButton->setBackgroundColor(value);
		m_pDefaultLight->m_specular = Colour(value.r(), value.g(), value.b());
		if (lightColorLock)
		{
			m_pAmbientButton->setBackgroundColor(value);
			m_pDiffuseButton->setBackgroundColor(value);
			m_pDefaultLight->m_ambient = Colour(value.r(), value.g(), value.b());
			m_pDefaultLight->m_diffuse = Colour(value.r(), value.g(), value.b());
		}
	});

	// Set initial colours
	m_pDefaultLight->m_ambient = Colour(m_pAmbientButton->backgroundColor().r(), m_pAmbientButton->backgroundColor().g(), m_pAmbientButton->backgroundColor().b());
	m_pDefaultLight->m_diffuse = Colour(m_pDiffuseButton->backgroundColor().r(), m_pDiffuseButton->backgroundColor().g(), m_pDiffuseButton->backgroundColor().b());
	m_pDefaultLight->m_specular = Colour(m_pSpecularButton->backgroundColor().r(), m_pSpecularButton->backgroundColor().g(), m_pSpecularButton->backgroundColor().b());

	Slider *slider = new Slider(m_pLightWindow);
	slider->setValue(0.0625f);
	slider->setFixedWidth(80);
	slider->setPosition(Vector2i(20, 185));

	TextBox *textBox = new TextBox(m_pLightWindow);
	textBox->setFixedSize(Vector2i(60, 25));
	textBox->setValue("64");
	slider->setCallback([textBox, this](float value) {
		textBox->setValue(std::to_string((int)(value * 1023) + 1));
		m_pQBTFile->SetMaterialShininess((value * 1023.0f) + 1.0f);
	});
	slider->setFinalCallback([&](float value) {
	});
	textBox->setFixedSize(Vector2i(50, 25));
	textBox->setFontSize(16);
	textBox->setPosition(Vector2i(115, 180));

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
	m_pMatricesInformationLabel->setCaption(matrices);

	string vertices = "Number of vertices: " + to_string(m_pQBTFile->GetNumVertices());
	m_pVerticesInformationLabel->setCaption(vertices);

	string triangles = "Number of triangles: " + to_string(m_pQBTFile->GetNumTriangles());
	m_pTrianglesInformationLabel->setCaption(triangles);

	m_bLightMovement = lightMovement;
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
