// ******************************************************************************
// Filename:    QBT.h
// Project:     Qube
// Author:      Steven Ball
//
// Purpose:
//   A Qubicle Binary Tree (.qbt) file loader. Also handles setting up the x, y, z
//   voxel grid array with color information for each voxel.
//
// Revision History:
//   Initial Revision - 28/07/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "../Renderer/Renderer.h"
#include "../Renderer/light.h"
#include "../Renderer/material.h"

#include <vector>
#include <string>
using namespace std;

enum MergedSide
{
	MergedSide_None = 0,

	MergedSide_X_Positive = 1,
	MergedSide_X_Negative = 2,
	MergedSide_Y_Positive = 4,
	MergedSide_Y_Negative = 8,
	MergedSide_Z_Positive = 16,
	MergedSide_Z_Negative = 32,
};

class QBTMatrix
{
public:
	unsigned int m_nameLength;
	char *m_name;

	int m_positionX;
	int m_positionY;
	int m_positionZ;

	unsigned int m_localScaleX;
	unsigned int m_localScaleY;
	unsigned int m_localScaleZ;

	float m_pivotX;
	float m_pivotY;
	float m_pivotZ;

	unsigned int m_sizeX;
	unsigned int m_sizeY;
	unsigned int m_sizeZ;

	unsigned int m_voxelDataSize;
	unsigned char* m_voxelData;
	unsigned int m_voxelDataSizeDecompressed;
	unsigned char* m_voxelDataDecompressed;

	unsigned int *m_pColour;
	unsigned int *m_pVisibilityMask;

	unsigned int m_numVertices;
	unsigned int m_numTriangles;
	unsigned int m_numIndices;

	// Material
	Material* m_pMaterial;

	// Rendering
	GLuint m_VBO;
	GLuint m_VAO;
	GLuint m_EBO;
};

typedef vector<QBTMatrix*> QBTMatrixList;

class QBT
{
public:
	/* Public methods */
	QBT(Renderer* pRenderer);
	~QBT();

	// Unloading
	void Unload();
	void DestroyStaticBuffers();

	// Loading
	bool LoadQBTFile(string filename);
	bool LoadNode(FILE* pQBTfile);
	bool LoadModel(FILE* pQBTfile);
	bool LoadMatrix(FILE* pQBTfile);
	bool LoadCompound(FILE* pQBTfile);
	bool SkipNode(FILE* pQBTfile);

	// Setup
	void SetVisibilityInformation();
	void RecreateStaticBuffers();
	void CreateStaticRenderBuffers();

	// Accessors
	string GetFilename();
	int GetNumMatrices();
	int GetNumVertices();
	int GetNumTriangles();

	// Modifiers
	void SetMaterialAmbient(Colour ambient);
	void SetMaterialDiffuse(Colour diffuse);
	void SetMaterialSpecular(Colour specular);
	void SetMaterialEmission(Colour emission);
	void SetMaterialShininess(float shininess);

	// Render modes
	void SetWireframeMode(bool wireframe);
	bool GetWireframeMode();
	void SetUseLighting(bool lighting);
	void SetBoundingBoxRendering(bool boundingBox);

	// Creation optimizations
	void SetCreateInnerVoxels(bool innerVoxels);
	void SetCreateInnerFaces(bool innerFaces);
	void SetMergeFaces(bool mergeFaces);

	// Render
	void Render(Camera* pCamera, Light* pLight);
	void RenderBoundingBox(Camera* pCamera, Light* pLight);

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
	// Header
	char m_magic[4];
	char m_major;
	char m_minor;
	float m_globalScaleX;
	float m_globalScaleY;
	float m_globalScaleZ;

	// Filename
	string m_filename;

	// Color map
	unsigned int m_numColors;
	char* m_pColors;

	// Matrices
	QBTMatrixList m_vpQBTMatrices;

	// Rendering modes
	bool m_wireframeRender;
	bool m_useLighting;
	bool m_boundingBox;

	// Creation optimizations
	bool m_createInnerVoxels;
	bool m_createInnerFaces;
	bool m_mergeFaces;

	// Shader
	Shader* m_pPositionColorNormalShader;

	// Renderer
	Renderer* m_pRenderer;
};
