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
bool boundingBox = false;
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
	cb = new CheckBox(m_pControlsWindow, "Bounding Box", [](bool state) { boundingBox = state; });
	cb->setChecked(shadows);
	cb->setTooltip("Bounding box rendering.");
	cb->setFontSize(14);
	cb->setPosition(Vector2i(20, 191));	
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
	cb->setPosition(Vector2i(20, 213));
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
	cb->setPosition(Vector2i(20, 235));
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
	cb->setPosition(Vector2i(20, 257));

	l = new Label(m_pControlsWindow, "File Operations", "arial");
	l->setPosition(Vector2i(10, 287));
	Button *b = new Button(m_pControlsWindow, "Open");
	b->setFontSize(18);
	b->setPosition(Vector2i(20, 308));
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
	b->setPosition(Vector2i(85, 308));
	b->setCallback([&]
	{
		string fileName = file_dialog({ { "qbt", "Qubicle Binary Tree" }, }, true);
	});


	// Light
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
	m_pAmbientButton_Light = new PopupButton(m_pLightWindow, "", 0);
	m_pAmbientButton_Light->setBackgroundColor(Color(50, 50, 50, 255));
	m_pAmbientButton_Light->setFontSize(16);
	m_pAmbientButton_Light->setFixedSize(Vector2i(80, 20));
	m_pAmbientButton_Light->setPosition(Vector2i(80, 82));
	Popup *popup = m_pAmbientButton_Light->popup();
	popup->setLayout(new GroupLayout());

	ColorWheel *colorwheel = new ColorWheel(popup);
	colorwheel->setColor(m_pAmbientButton_Light->backgroundColor());

	colorwheel->setCallback([this](const Color &value)
	{
		m_pAmbientButton_Light->setBackgroundColor(value);
		m_pDefaultLight->m_ambient = Colour(value.r(), value.g(), value.b());
		if (lightColorLock)
		{
			m_pDiffuseButton_Light->setBackgroundColor(value);
			m_pSpecularButton_Light->setBackgroundColor(value);
			m_pDefaultLight->m_diffuse = Colour(value.r(), value.g(), value.b());
			m_pDefaultLight->m_specular = Colour(value.r(), value.g(), value.b());
		}
	});

	// Diffuse
	l = new Label(m_pLightWindow, "Diffuse:", "arial");
	l->setPosition(Vector2i(20, 108));
	l->setFontSize(14);
	m_pDiffuseButton_Light = new PopupButton(m_pLightWindow, "", 0);
	m_pDiffuseButton_Light->setBackgroundColor(Color(204, 204, 204, 255));
	m_pDiffuseButton_Light->setFontSize(16);
	m_pDiffuseButton_Light->setFixedSize(Vector2i(80, 20));
	m_pDiffuseButton_Light->setPosition(Vector2i(80, 106));
	popup = m_pDiffuseButton_Light->popup();
	popup->setLayout(new GroupLayout());

	colorwheel = new ColorWheel(popup);
	colorwheel->setColor(m_pDiffuseButton_Light->backgroundColor());

	colorwheel->setCallback([this](const Color &value)
	{
		m_pDiffuseButton_Light->setBackgroundColor(value);
		m_pDefaultLight->m_diffuse = Colour(value.r(), value.g(), value.b());
		if (lightColorLock)
		{
			m_pAmbientButton_Light->setBackgroundColor(value);
			m_pSpecularButton_Light->setBackgroundColor(value);
			m_pDefaultLight->m_ambient = Colour(value.r(), value.g(), value.b());
			m_pDefaultLight->m_specular = Colour(value.r(), value.g(), value.b());
		}
	});

	// Specular
	l = new Label(m_pLightWindow, "Specular:", "arial");
	l->setPosition(Vector2i(20, 132));
	l->setFontSize(14);
	m_pSpecularButton_Light = new PopupButton(m_pLightWindow, "", 0);
	m_pSpecularButton_Light->setBackgroundColor(Color(200, 200, 200, 255));
	m_pSpecularButton_Light->setFontSize(16);
	m_pSpecularButton_Light->setFixedSize(Vector2i(80, 20));
	m_pSpecularButton_Light->setPosition(Vector2i(80, 130));
	popup = m_pSpecularButton_Light->popup();
	popup->setLayout(new GroupLayout());

	colorwheel = new ColorWheel(popup);
	colorwheel->setColor(m_pSpecularButton_Light->backgroundColor());

	colorwheel->setCallback([this](const Color &value)
	{
		m_pSpecularButton_Light->setBackgroundColor(value);
		m_pDefaultLight->m_specular = Colour(value.r(), value.g(), value.b());
		if (lightColorLock)
		{
			m_pAmbientButton_Light->setBackgroundColor(value);
			m_pDiffuseButton_Light->setBackgroundColor(value);
			m_pDefaultLight->m_ambient = Colour(value.r(), value.g(), value.b());
			m_pDefaultLight->m_diffuse = Colour(value.r(), value.g(), value.b());
		}
	});

	// Set initial light colours
	m_pDefaultLight->m_ambient = Colour(m_pAmbientButton_Light->backgroundColor().r(), m_pAmbientButton_Light->backgroundColor().g(), m_pAmbientButton_Light->backgroundColor().b());
	m_pDefaultLight->m_diffuse = Colour(m_pDiffuseButton_Light->backgroundColor().r(), m_pDiffuseButton_Light->backgroundColor().g(), m_pDiffuseButton_Light->backgroundColor().b());
	m_pDefaultLight->m_specular = Colour(m_pSpecularButton_Light->backgroundColor().r(), m_pSpecularButton_Light->backgroundColor().g(), m_pSpecularButton_Light->backgroundColor().b());


	// Material
	l = new Label(m_pLightWindow, "Material", "arial");
	l->setPosition(Vector2i(10, 161));

	// Ambient
	l = new Label(m_pLightWindow, "Ambient:", "arial");
	l->setPosition(Vector2i(20, 185));
	l->setFontSize(14);
	m_pAmbientButton_Material = new PopupButton(m_pLightWindow, "", 0);
	m_pAmbientButton_Material->setBackgroundColor(Color(255, 255, 255, 255));
	m_pAmbientButton_Material->setFontSize(16);
	m_pAmbientButton_Material->setFixedSize(Vector2i(80, 20));
	m_pAmbientButton_Material->setPosition(Vector2i(80, 183));
	popup = m_pAmbientButton_Material->popup();
	popup->setLayout(new GroupLayout());

	colorwheel = new ColorWheel(popup);
	colorwheel->setColor(m_pAmbientButton_Material->backgroundColor());

	colorwheel->setCallback([this](const Color &value)
	{
		m_pAmbientButton_Material->setBackgroundColor(value);
		m_pQBTFile->SetMaterialAmbient(Colour(value.r(), value.g(), value.b()));
	});

	// Diffuse
	l = new Label(m_pLightWindow, "Diffuse:", "arial");
	l->setPosition(Vector2i(20, 209));
	l->setFontSize(14);
	m_pDiffuseButton_Material = new PopupButton(m_pLightWindow, "", 0);
	m_pDiffuseButton_Material->setBackgroundColor(Color(255, 255, 255, 255));
	m_pDiffuseButton_Material->setFontSize(16);
	m_pDiffuseButton_Material->setFixedSize(Vector2i(80, 20));
	m_pDiffuseButton_Material->setPosition(Vector2i(80, 207));
	popup = m_pDiffuseButton_Material->popup();
	popup->setLayout(new GroupLayout());

	colorwheel = new ColorWheel(popup);
	colorwheel->setColor(m_pDiffuseButton_Material->backgroundColor());

	colorwheel->setCallback([this](const Color &value)
	{
		m_pDiffuseButton_Material->setBackgroundColor(value);
		m_pQBTFile->SetMaterialDiffuse(Colour(value.r(), value.g(), value.b()));
	});

	// Specular
	l = new Label(m_pLightWindow, "Specular:", "arial");
	l->setPosition(Vector2i(20, 233));
	l->setFontSize(14);
	m_pSpecularButton_Material = new PopupButton(m_pLightWindow, "", 0);
	m_pSpecularButton_Material->setBackgroundColor(Color(255, 255, 255, 255));
	m_pSpecularButton_Material->setFontSize(16);
	m_pSpecularButton_Material->setFixedSize(Vector2i(80, 20));
	m_pSpecularButton_Material->setPosition(Vector2i(80, 231));
	popup = m_pSpecularButton_Material->popup();
	popup->setLayout(new GroupLayout());

	colorwheel = new ColorWheel(popup);
	colorwheel->setColor(m_pSpecularButton_Material->backgroundColor());

	colorwheel->setCallback([this](const Color &value)
	{
		m_pSpecularButton_Material->setBackgroundColor(value);
		m_pQBTFile->SetMaterialSpecular(Colour(value.r(), value.g(), value.b()));
	});

	// Emission
	l = new Label(m_pLightWindow, "Emission:", "arial");
	l->setPosition(Vector2i(20, 257));
	l->setFontSize(14);
	m_pEmissionButton_Material = new PopupButton(m_pLightWindow, "", 0);
	m_pEmissionButton_Material->setBackgroundColor(Color(255, 255, 255, 255));
	m_pEmissionButton_Material->setFontSize(16);
	m_pEmissionButton_Material->setFixedSize(Vector2i(80, 20));
	m_pEmissionButton_Material->setPosition(Vector2i(80, 255));
	popup = m_pEmissionButton_Material->popup();
	popup->setLayout(new GroupLayout());

	colorwheel = new ColorWheel(popup);
	colorwheel->setColor(m_pEmissionButton_Material->backgroundColor());

	colorwheel->setCallback([this](const Color &value)
	{
		m_pEmissionButton_Material->setBackgroundColor(value);
		m_pQBTFile->SetMaterialEmission(Colour(value.r(), value.g(), value.b()));
	});

	l = new Label(m_pLightWindow, "Shininess", "arial");
	l->setPosition(Vector2i(10, 291));

	Slider *slider = new Slider(m_pLightWindow);
	slider->setValue(0.0625f);
	slider->setFixedWidth(80);
	slider->setPosition(Vector2i(20, 312));

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
	textBox->setPosition(Vector2i(115, 306));

	// Set initial material colours
	//m_pQBTFile->SetMaterialAmbient(Colour(m_pAmbientButton_Material->backgroundColor().r(), m_pAmbientButton_Material->backgroundColor().g(), m_pAmbientButton_Material->backgroundColor().b()));
	//m_pQBTFile->SetMaterialDiffuse(Colour(m_pDiffuseButton_Material->backgroundColor().r(), m_pDiffuseButton_Material->backgroundColor().g(), m_pDiffuseButton_Material->backgroundColor().b()));
	//m_pQBTFile->SetMaterialSpecular(Colour(m_pSpecularButton_Light->backgroundColor().r(), m_pSpecularButton_Light->backgroundColor().g(), m_pSpecularButton_Light->backgroundColor().b()));
	//m_pQBTFile->SetMaterialEmission(Colour(m_pEmissionButton_Material->backgroundColor().r(), m_pEmissionButton_Material->backgroundColor().g(), m_pEmissionButton_Material->backgroundColor().b()));
	//m_pQBTFile->SetMaterialShininess(64.0f);

	m_pMatrixWindow = new Window(m_pNanoGUIScreen, "Matrix");
	m_pMatrixWindow->setSize(Vector2i(175, 350));
	m_pMatrixWindow->setPosition(Vector2i(210, 500));

	m_pMatricesCombo = new ComboBox(m_pMatrixWindow);
	m_pMatricesCombo->setItems({ "Item 1", "Item 2", "Item 3" });
	m_pMatricesCombo->setPosition(Vector2i(5, 33));

	// Initial visibility for GUI
	m_pControlsWindow->setVisible(true);
	m_pLightWindow->setVisible(true);
	m_pMatrixWindow->setVisible(true);
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
	m_pQBTFile->SetBoundingBoxRendering(boundingBox);
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
