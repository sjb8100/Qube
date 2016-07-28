// ******************************************************************************
// Filename:    QBT.h
// Project:     Qube
// Author:      Steven Ball
//
// Purpose:
// 
// Revision History:
//   Initial Revision - 28/07/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include <vector>
#include <string>
using namespace std;


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
};

typedef vector<QBTMatrix*> QBTMatrixList;

class QBT
{
public:
	/* Public methods */
	QBT();
	~QBT();

	bool LoadQBTFile(string filename);

	bool LoadNode(FILE* pQBTfile);
	bool LoadModel(FILE* pQBTfile);
	bool LoadMatrix(FILE* pQBTfile);
	bool LoadCompound(FILE* pQBTfile);
	bool SkipNode(FILE* pQBTfile);

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

	// Color map
	unsigned int m_numColors;
	char* m_pColors;

	// Matrices
	QBTMatrixList m_vpQBTMatrices;
};
