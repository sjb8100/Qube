// ******************************************************************************
// Filename:    QBT.cpp
// Project:     Vogue
// Author:      Steven Ball
// 
// Revision History:
//   Initial Revision - 28/07/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "QBT.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../zlib/zlib.h"


QBT::QBT()
{

}

QBT::~QBT()
{

}

bool QBT::LoadQBTFile(string filename)
{
	FILE* pQBTfile = NULL;
	fopen_s(&pQBTfile, filename.c_str(), "rb");

	if (pQBTfile != NULL)
	{
		int ok = 0;

		// Header
		ok = fread(&m_magic, sizeof(char), 4, pQBTfile) == 1;
		ok = fread(&m_major, sizeof(char), 1, pQBTfile) == 1;
		ok = fread(&m_minor, sizeof(char), 1, pQBTfile) == 1;
		ok = fread((void*)(&m_globalScaleX), sizeof(m_globalScaleX), 1, pQBTfile) == 1;
		ok = fread((void*)(&m_globalScaleY), sizeof(m_globalScaleY), 1, pQBTfile) == 1;
		ok = fread((void*)(&m_globalScaleZ), sizeof(m_globalScaleZ), 1, pQBTfile) == 1;

		// Color map
		char sectionCaption[8];
		ok = fread(&sectionCaption, sizeof(char), 8, pQBTfile) == 1;
		ok = fread(&m_numColors, sizeof(unsigned int), 1, pQBTfile) == 1;
		m_pColors = new char[m_numColors * 4];
		for (unsigned int i = 0; i < m_numColors; i++)
		{
			ok = fread(&m_pColors[(i*4)+0], sizeof(char), 1, pQBTfile) == 1;
			ok = fread(&m_pColors[(i*4)+1], sizeof(char), 1, pQBTfile) == 1;
			ok = fread(&m_pColors[(i*4)+2], sizeof(char), 1, pQBTfile) == 1;
			ok = fread(&m_pColors[(i*4)+3], sizeof(char), 1, pQBTfile) == 1;
		}

		// Data tree
		ok = fread(&sectionCaption, sizeof(char), 8, pQBTfile) == 1;
		LoadNode(pQBTfile);

		fclose(pQBTfile);

		return true;
	}

	return false;
}


bool QBT::LoadNode(FILE* pQBTfile)
{
	unsigned int nodeTypeID;
	unsigned int dataSize;
	int ok = 0;
	ok = fread(&nodeTypeID, sizeof(unsigned int), 1, pQBTfile) == 1;
	ok = fread(&dataSize, sizeof(unsigned int), 1, pQBTfile) == 1;

	switch (nodeTypeID)
	{
		case 0:
		{
			LoadMatrix(pQBTfile);
			break;
		}
		case 1:
		{
			LoadModel(pQBTfile);
			break;
		}
		case 2:
		{
			LoadCompound(pQBTfile);
			break;
		}
		default:
		{
			char* skipData = new char[dataSize];
			ok = fread(&skipData[0], sizeof(char), dataSize, pQBTfile) == 1;
			break;
		}
	}

	return true;
}

bool QBT::LoadModel(FILE* pQBTfile)
{
	unsigned int childCount;
	int ok = fread(&childCount, sizeof(unsigned int), 1, pQBTfile) == 1;
	for (unsigned int i = 0; i < childCount; i++)
	{
		LoadNode(pQBTfile);
	}

	return true;
}

bool QBT::LoadMatrix(FILE* pQBTfile)
{
	int ok = 0;

	QBTMatrix* pNewMatrix = new QBTMatrix();

	// Name
	ok = fread(&pNewMatrix->m_nameLength, sizeof(unsigned int), 1, pQBTfile) == 1;
	pNewMatrix->m_name = new char[pNewMatrix->m_nameLength + 1];
	ok = fread(&pNewMatrix->m_name[0], sizeof(char), pNewMatrix->m_nameLength, pQBTfile) == 1;
	pNewMatrix->m_name[pNewMatrix->m_nameLength] = 0;

	// Position
	ok = fread(&pNewMatrix->m_positionX, sizeof(int), 1, pQBTfile) == 1;
	ok = fread(&pNewMatrix->m_positionY, sizeof(int), 1, pQBTfile) == 1;
	ok = fread(&pNewMatrix->m_positionZ, sizeof(int), 1, pQBTfile) == 1;

	// Local scale
	ok = fread(&pNewMatrix->m_localScaleX, sizeof(unsigned int), 1, pQBTfile) == 1;
	ok = fread(&pNewMatrix->m_localScaleY, sizeof(unsigned int), 1, pQBTfile) == 1;
	ok = fread(&pNewMatrix->m_localScaleZ, sizeof(unsigned int), 1, pQBTfile) == 1;

	// Pivot
	ok = fread((void*)(&pNewMatrix->m_pivotX), sizeof(pNewMatrix->m_pivotX), 1, pQBTfile) == 1;
	ok = fread((void*)(&pNewMatrix->m_pivotY), sizeof(pNewMatrix->m_pivotY), 1, pQBTfile) == 1;
	ok = fread((void*)(&pNewMatrix->m_pivotZ), sizeof(pNewMatrix->m_pivotZ), 1, pQBTfile) == 1;

	// Size
	ok = fread(&pNewMatrix->m_sizeX, sizeof(unsigned int), 1, pQBTfile) == 1;
	ok = fread(&pNewMatrix->m_sizeY, sizeof(unsigned int), 1, pQBTfile) == 1;
	ok = fread(&pNewMatrix->m_sizeZ, sizeof(unsigned int), 1, pQBTfile) == 1;

	// Voxel data
	ok = fread(&pNewMatrix->m_voxelDataSize, sizeof(unsigned int), 1, pQBTfile) == 1;	
	pNewMatrix->m_voxelData = new unsigned char[pNewMatrix->m_voxelDataSize];
	ok = fread(&pNewMatrix->m_voxelData[0], sizeof(unsigned char)*pNewMatrix->m_voxelDataSize, 1, pQBTfile) == 1;

	// DEBUG OUTPUT
	//printf("Compressed voxel data size is: %lu\n", pNewMatrix->m_voxelDataSize);
	//printf("Compressed voxel data is: ");
	//for (unsigned int i = 0; i < pNewMatrix->m_voxelDataSize; i++)
	//{
	//	printf("%c", pNewMatrix->m_voxelData[i]);
	//}
	//printf("\n");

	pNewMatrix->m_voxelDataSizeDecompressed = (pNewMatrix->m_sizeX * pNewMatrix->m_sizeY * pNewMatrix->m_sizeZ) * 4;
	pNewMatrix->m_voxelDataDecompressed = new unsigned char[pNewMatrix->m_voxelDataSizeDecompressed];

	// Setup zlib buffers
	z_stream infstream;
	infstream.zalloc = Z_NULL;
	infstream.zfree = Z_NULL;
	infstream.opaque = Z_NULL;
	infstream.avail_in = (uInt)pNewMatrix->m_voxelDataSize; // size of input
	infstream.next_in = (Bytef *)pNewMatrix->m_voxelData; // input char array
	infstream.avail_out = (uInt)pNewMatrix->m_voxelDataSizeDecompressed; // size of output
	infstream.next_out = (Bytef *)pNewMatrix->m_voxelDataDecompressed; // output char array

	// Decompression
	inflateInit(&infstream);
	inflate(&infstream, Z_NO_FLUSH);
	inflateEnd(&infstream);

	// DEBUG OUTPUT
	//printf("Decompressed voxel data size is: %lu\n", pNewMatrix->m_voxelDataSizeDecompressed);
	//printf("Decompressed voxel data is: ");
	//for (unsigned int i = 0; i < pNewMatrix->m_voxelDataSizeDecompressed; i++)
	//{
	//	printf("%c", pNewMatrix->m_voxelDataDecompressed[i]);
	//}
	//printf("\n");

	pNewMatrix->m_pColour = new unsigned int[pNewMatrix->m_sizeX * pNewMatrix->m_sizeY * pNewMatrix->m_sizeZ];

	unsigned int byteCounter = 0;
	for (unsigned int x = 0; x < pNewMatrix->m_sizeX; x++)
	{
		for (unsigned int z = 0; z < pNewMatrix->m_sizeZ; z++)
		{
			for (unsigned int y = 0; y < pNewMatrix->m_sizeY; y++)
			{
				unsigned int r = pNewMatrix->m_voxelDataDecompressed[byteCounter+0];
				unsigned int g = pNewMatrix->m_voxelDataDecompressed[byteCounter+1];
				unsigned int b = pNewMatrix->m_voxelDataDecompressed[byteCounter+2];
				unsigned int mask = pNewMatrix->m_voxelDataDecompressed[byteCounter+3]; // Visibility mask
				byteCounter += 4;

				// Compress the rgba into a single unsigned int for storage in the matrix structure
				unsigned int alpha = (int)(255) << 24;
				unsigned int blue = (int)(b) << 16;
				unsigned int green = (int)(g) << 8;
				unsigned int red = (int)(r);

				unsigned int colour = red + green + blue + alpha;
				pNewMatrix->m_pColour[x + pNewMatrix->m_sizeX * (y + pNewMatrix->m_sizeY * z)] = colour;
			}
		}
	}

	m_vpQBTMatrices.push_back(pNewMatrix);

	return true;
}

bool QBT::LoadCompound(FILE* pQBTfile)
{
	// Compounds are not supported atm...
	return true;
}

bool QBT::SkipNode(FILE* pQBTfile)
{
	unsigned int dataSize;
	int ok = 0;

	ok = fread(&dataSize, sizeof(unsigned int), 1, pQBTfile) == 1;
	char* skipData = new char[dataSize];
	ok = fread(&skipData[0], sizeof(char), dataSize, pQBTfile) == 1;
	return true;
}