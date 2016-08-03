// ******************************************************************************
// Filename:    QBT.cpp
// Project:     Qube
// Author:      Steven Ball
// 
// Revision History:
//   Initial Revision - 28/07/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "QBT.h"
#include "../QubeGame.h"
#include "../zlib/zlib.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;


QBT::QBT(Renderer* pRenderer)
{
	m_pRenderer = pRenderer;

	// Render modes
	m_wireframeRender = false;

	// Creation optimizations
	m_createInnerVoxels = false;
	m_createInnerFaces = false;
	m_mergeFaces = true;

	// Shader
	m_pPositionColorNormalShader = new Shader("media/shaders/PositionColorNormal.vertex", "media/shaders/PositionColorNormal.fragment");
}

QBT::~QBT()
{
	Unload();
}

// Unloading
void QBT::Unload()
{
	DestroyStaticBuffers();

	for (unsigned int i = 0; i < m_vpQBTMatrices.size(); i++)
	{
		delete[] m_vpQBTMatrices[i]->m_pColour;
		delete[] m_vpQBTMatrices[i]->m_pVisibilityMask;

		delete m_vpQBTMatrices[i];
		m_vpQBTMatrices[i] = NULL;
	}
	m_vpQBTMatrices.clear();
}

void QBT::DestroyStaticBuffers()
{
	for (unsigned int i = 0; i < m_vpQBTMatrices.size(); i++)
	{
		glDeleteBuffers(1, &m_vpQBTMatrices[i]->m_VBO);
		glDeleteBuffers(1, &m_vpQBTMatrices[i]->m_EBO);
		glDeleteVertexArrays(1, &m_vpQBTMatrices[i]->m_VAO);
	}
}

// Loading
bool QBT::LoadQBTFile(string filename)
{
	FILE* pQBTfile = NULL;
	fopen_s(&pQBTfile, filename.c_str(), "rb");

	if (pQBTfile != NULL)
	{
		int lastindex = (int)filename.find_last_of("/");
		if (lastindex == -1)
		{
			lastindex = (int)filename.find_last_of("\\");
		}
		m_filename = filename.substr(lastindex+1);

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

		SetVisibilityInformation();
		CreateStaticRenderBuffers();

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
	pNewMatrix->m_pVisibilityMask = new unsigned int[pNewMatrix->m_sizeX * pNewMatrix->m_sizeY * pNewMatrix->m_sizeZ];
	pNewMatrix->m_numVisibleVoxels = 0;

	unsigned int byteCounter = 0;
	for (unsigned int x = 0; x < pNewMatrix->m_sizeX; x++)
	{
		for (unsigned int z = 0; z < pNewMatrix->m_sizeZ; z++)
		{
			for (unsigned int y = 0; y < pNewMatrix->m_sizeY; y++)
			{
				int r = pNewMatrix->m_voxelDataDecompressed[byteCounter+0];
				int g = pNewMatrix->m_voxelDataDecompressed[byteCounter+1];
				int b = pNewMatrix->m_voxelDataDecompressed[byteCounter+2];
				int mask = pNewMatrix->m_voxelDataDecompressed[byteCounter+3]; // Visibility mask
				byteCounter += 4;

				unsigned int colour = 0;

				// If mask is 0, this is an invisible voxel, not active
				if (mask != 0)
				{
					// Squish the rgba into a single unsigned int for storage in the matrix structure
					unsigned int alpha = (int)(mask == 0 ? 0 : 255) << 24;
					unsigned int blue = (int)(b) << 16;
					unsigned int green = (int)(g) << 8;
					unsigned int red = (int)(r);

					colour = red + green + blue + alpha;
				}

				pNewMatrix->m_pColour[x + pNewMatrix->m_sizeX * (y + pNewMatrix->m_sizeY * z)] = colour;
				pNewMatrix->m_pVisibilityMask[x + pNewMatrix->m_sizeX * (y + pNewMatrix->m_sizeY * z)] = mask;
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

// Setup
void QBT::SetVisibilityInformation()
{
	for (unsigned int i = 0; i < m_vpQBTMatrices.size(); i++)
	{
		QBTMatrix* pMatrix = m_vpQBTMatrices[i];

		pMatrix->m_numVisibleVoxels = 0;
		pMatrix->m_numTriangles = 0;

		for (unsigned int x = 0; x < pMatrix->m_sizeX; x++)
		{
			for (unsigned int z = 0; z < pMatrix->m_sizeZ; z++)
			{
				for (unsigned int y = 0; y < pMatrix->m_sizeY; y++)
				{
					unsigned int mask = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

					// If mask is 0, this is an invisible voxel, not active
					if (mask != 0)
					{
						if (mask != 1 || m_createInnerVoxels == true)
						{
							pMatrix->m_numVisibleVoxels += 1;

							// Back
							if (m_createInnerFaces == true || (mask & 32) == 32)
							{
								pMatrix->m_numTriangles += 2;
							}

							// Front
							if (m_createInnerFaces == true || (mask & 64) == 64)
							{
								pMatrix->m_numTriangles += 2;
							}

							// Left
							if (m_createInnerFaces == true || (mask & 4) == 4)
							{
								pMatrix->m_numTriangles += 2;
							}

							// Right
							if (m_createInnerFaces == true || (mask & 2) == 2)
							{
								pMatrix->m_numTriangles += 2;
							}

							// Top
							if (m_createInnerFaces == true || (mask & 8) == 8)
							{
								pMatrix->m_numTriangles += 2;
							}

							// Bottom
							if (m_createInnerFaces == true || (mask & 16) == 16)
							{
								pMatrix->m_numTriangles += 2;
							}
						}
					}
				}
			}
		}
	}
}

void QBT::RecreateStaticBuffers()
{
	DestroyStaticBuffers();
	CreateStaticRenderBuffers();
}

void QBT::CreateStaticRenderBuffers()
{
	for (unsigned int matrixIndex = 0; matrixIndex < m_vpQBTMatrices.size(); matrixIndex++)
	{
		QBTMatrix* pMatrix = m_vpQBTMatrices[matrixIndex];

		// Vertices
		pMatrix->m_numVertices = (unsigned int)pMatrix->m_numVisibleVoxels * 24;
		PositionColorNormalVertex* verticesBuffer = new PositionColorNormalVertex[pMatrix->m_numVertices];

		// Indices
		pMatrix->m_numIndices = (unsigned int)pMatrix->m_numTriangles * 3;
		GLuint* indicesBuffer = new GLuint[pMatrix->m_numIndices];

		unsigned int verticesCounter = 0;
		unsigned int indicesCounter = 0;
		for (unsigned int x = 0; x < pMatrix->m_sizeX; x++)
		{
			for (unsigned int y = 0; y < pMatrix->m_sizeY; y++)
			{
				for (unsigned int z = 0; z < pMatrix->m_sizeZ; z++)
				{
					unsigned int colour = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
					unsigned int mask = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
					unsigned int alpha = (colour & 0xFF000000) >> 24;
					unsigned int blue = (colour & 0x00FF0000) >> 16;
					unsigned int green = (colour & 0x0000FF00) >> 8;
					unsigned int red = (colour & 0x000000FF);

					if (mask == 0)
					{
						continue;
					}

					if (mask == 1 && m_createInnerVoxels == false)
					{
						continue;
					}

					float r = (float)(red / 255.0f);
					float g = (float)(green / 255.0f);
					float b = (float)(blue / 255.0f);

					// Back 0 1 2 3
					verticesBuffer[verticesCounter + 0].x = x + -0.5f;
					verticesBuffer[verticesCounter + 0].y = y + -0.5f;
					verticesBuffer[verticesCounter + 0].z = z + -0.5f;
					verticesBuffer[verticesCounter + 0].r = r;
					verticesBuffer[verticesCounter + 0].g = g;
					verticesBuffer[verticesCounter + 0].b = b;
					verticesBuffer[verticesCounter + 0].a = 1.0f;
					verticesBuffer[verticesCounter + 0].nx = 0.0f;
					verticesBuffer[verticesCounter + 0].ny = 0.0f;
					verticesBuffer[verticesCounter + 0].nz = -1.0f;

					verticesBuffer[verticesCounter + 1].x = x + 0.5f;
					verticesBuffer[verticesCounter + 1].y = y + -0.5f;
					verticesBuffer[verticesCounter + 1].z = z + -0.5f;
					verticesBuffer[verticesCounter + 1].r = r;
					verticesBuffer[verticesCounter + 1].g = g;
					verticesBuffer[verticesCounter + 1].b = b;
					verticesBuffer[verticesCounter + 1].a = 1.0f;
					verticesBuffer[verticesCounter + 1].nx = 0.0f;
					verticesBuffer[verticesCounter + 1].ny = 0.0f;
					verticesBuffer[verticesCounter + 1].nz = -1.0f;

					verticesBuffer[verticesCounter + 2].x = x + -0.5f;
					verticesBuffer[verticesCounter + 2].y = y + 0.5f;
					verticesBuffer[verticesCounter + 2].z = z + -0.5f;
					verticesBuffer[verticesCounter + 2].r = r;
					verticesBuffer[verticesCounter + 2].g = g;
					verticesBuffer[verticesCounter + 2].b = b;
					verticesBuffer[verticesCounter + 2].a = 1.0f;
					verticesBuffer[verticesCounter + 2].nx = 0.0f;
					verticesBuffer[verticesCounter + 2].ny = 0.0f;
					verticesBuffer[verticesCounter + 2].nz = -1.0f;

					verticesBuffer[verticesCounter + 3].x = x + 0.5f;
					verticesBuffer[verticesCounter + 3].y = y + 0.5f;
					verticesBuffer[verticesCounter + 3].z = z + -0.5f;
					verticesBuffer[verticesCounter + 3].r = r;
					verticesBuffer[verticesCounter + 3].g = g;
					verticesBuffer[verticesCounter + 3].b = b;
					verticesBuffer[verticesCounter + 3].a = 1.0f;
					verticesBuffer[verticesCounter + 3].nx = 0.0f;
					verticesBuffer[verticesCounter + 3].ny = 0.0f;
					verticesBuffer[verticesCounter + 3].nz = -1.0f;

					// Front 4 5 6 7
					verticesBuffer[verticesCounter + 4].x = x + -0.5f;
					verticesBuffer[verticesCounter + 4].y = y + -0.5f;
					verticesBuffer[verticesCounter + 4].z = z + 0.5f;
					verticesBuffer[verticesCounter + 4].r = r;
					verticesBuffer[verticesCounter + 4].g = g;
					verticesBuffer[verticesCounter + 4].b = b;
					verticesBuffer[verticesCounter + 4].a = 1.0f;
					verticesBuffer[verticesCounter + 4].nx = 0.0f;
					verticesBuffer[verticesCounter + 4].ny = 0.0f;
					verticesBuffer[verticesCounter + 4].nz = 1.0f;

					verticesBuffer[verticesCounter + 5].x = x + 0.5f;
					verticesBuffer[verticesCounter + 5].y = y + -0.5f;
					verticesBuffer[verticesCounter + 5].z = z + 0.5f;
					verticesBuffer[verticesCounter + 5].r = r;
					verticesBuffer[verticesCounter + 5].g = g;
					verticesBuffer[verticesCounter + 5].b = b;
					verticesBuffer[verticesCounter + 5].a = 1.0f;
					verticesBuffer[verticesCounter + 5].nx = 0.0f;
					verticesBuffer[verticesCounter + 5].ny = 0.0f;
					verticesBuffer[verticesCounter + 5].nz = 1.0f;

					verticesBuffer[verticesCounter + 6].x = x + -0.5f;
					verticesBuffer[verticesCounter + 6].y = y + 0.5f;
					verticesBuffer[verticesCounter + 6].z = z + 0.5f;
					verticesBuffer[verticesCounter + 6].r = r;
					verticesBuffer[verticesCounter + 6].g = g;
					verticesBuffer[verticesCounter + 6].b = b;
					verticesBuffer[verticesCounter + 6].a = 1.0f;
					verticesBuffer[verticesCounter + 6].nx = 0.0f;
					verticesBuffer[verticesCounter + 6].ny = 0.0f;
					verticesBuffer[verticesCounter + 6].nz = 1.0f;

					verticesBuffer[verticesCounter + 7].x = x + 0.5f;
					verticesBuffer[verticesCounter + 7].y = y + 0.5f;
					verticesBuffer[verticesCounter + 7].z = z + 0.5f;
					verticesBuffer[verticesCounter + 7].r = r;
					verticesBuffer[verticesCounter + 7].g = g;
					verticesBuffer[verticesCounter + 7].b = b;
					verticesBuffer[verticesCounter + 7].a = 1.0f;
					verticesBuffer[verticesCounter + 7].nx = 0.0f;
					verticesBuffer[verticesCounter + 7].ny = 0.0f;
					verticesBuffer[verticesCounter + 7].nz = 1.0f;

					// Left 8 9 10 11
					verticesBuffer[verticesCounter + 8].x = x + -0.5f;
					verticesBuffer[verticesCounter + 8].y = y + -0.5f;
					verticesBuffer[verticesCounter + 8].z = z + -0.5f;
					verticesBuffer[verticesCounter + 8].r = r;
					verticesBuffer[verticesCounter + 8].g = g;
					verticesBuffer[verticesCounter + 8].b = b;
					verticesBuffer[verticesCounter + 8].a = 1.0f;
					verticesBuffer[verticesCounter + 8].nx = -1.0f;
					verticesBuffer[verticesCounter + 8].ny = 0.0f;
					verticesBuffer[verticesCounter + 8].nz = 0.0f;

					verticesBuffer[verticesCounter + 9].x = x + -0.5f;
					verticesBuffer[verticesCounter + 9].y = y + -0.5f;
					verticesBuffer[verticesCounter + 9].z = z + 0.5f;
					verticesBuffer[verticesCounter + 9].r = r;
					verticesBuffer[verticesCounter + 9].g = g;
					verticesBuffer[verticesCounter + 9].b = b;
					verticesBuffer[verticesCounter + 9].a = 1.0f;
					verticesBuffer[verticesCounter + 9].nx = -1.0f;
					verticesBuffer[verticesCounter + 9].ny = 0.0f;
					verticesBuffer[verticesCounter + 9].nz = 0.0f;

					verticesBuffer[verticesCounter + 10].x = x + -0.5f;
					verticesBuffer[verticesCounter + 10].y = y + 0.5f;
					verticesBuffer[verticesCounter + 10].z = z + -0.5f;
					verticesBuffer[verticesCounter + 10].r = r;
					verticesBuffer[verticesCounter + 10].g = g;
					verticesBuffer[verticesCounter + 10].b = b;
					verticesBuffer[verticesCounter + 10].a = 1.0f;
					verticesBuffer[verticesCounter + 10].nx = -1.0f;
					verticesBuffer[verticesCounter + 10].ny = 0.0f;
					verticesBuffer[verticesCounter + 10].nz = 0.0f;

					verticesBuffer[verticesCounter + 11].x = x + -0.5f;
					verticesBuffer[verticesCounter + 11].y = y + 0.5f;
					verticesBuffer[verticesCounter + 11].z = z + 0.5f;
					verticesBuffer[verticesCounter + 11].r = r;
					verticesBuffer[verticesCounter + 11].g = g;
					verticesBuffer[verticesCounter + 11].b = b;
					verticesBuffer[verticesCounter + 11].a = 1.0f;
					verticesBuffer[verticesCounter + 11].nx = -1.0f;
					verticesBuffer[verticesCounter + 11].ny = 0.0f;
					verticesBuffer[verticesCounter + 11].nz = 0.0f;

					// Right 12 13 14 15
					verticesBuffer[verticesCounter + 12].x = x + 0.5f;
					verticesBuffer[verticesCounter + 12].y = y + -0.5f;
					verticesBuffer[verticesCounter + 12].z = z + -0.5f;
					verticesBuffer[verticesCounter + 12].r = r;
					verticesBuffer[verticesCounter + 12].g = g;
					verticesBuffer[verticesCounter + 12].b = b;
					verticesBuffer[verticesCounter + 12].a = 1.0f;
					verticesBuffer[verticesCounter + 12].nx = 1.0f;
					verticesBuffer[verticesCounter + 12].ny = 0.0f;
					verticesBuffer[verticesCounter + 12].nz = 0.0f;

					verticesBuffer[verticesCounter + 13].x = x + 0.5f;
					verticesBuffer[verticesCounter + 13].y = y + -0.5f;
					verticesBuffer[verticesCounter + 13].z = z + 0.5f;
					verticesBuffer[verticesCounter + 13].r = r;
					verticesBuffer[verticesCounter + 13].g = g;
					verticesBuffer[verticesCounter + 13].b = b;
					verticesBuffer[verticesCounter + 13].a = 1.0f;
					verticesBuffer[verticesCounter + 13].nx = 1.0f;
					verticesBuffer[verticesCounter + 13].ny = 0.0f;
					verticesBuffer[verticesCounter + 13].nz = 0.0f;

					verticesBuffer[verticesCounter + 14].x = x + 0.5f;
					verticesBuffer[verticesCounter + 14].y = y + 0.5f;
					verticesBuffer[verticesCounter + 14].z = z + -0.5f;
					verticesBuffer[verticesCounter + 14].r = r;
					verticesBuffer[verticesCounter + 14].g = g;
					verticesBuffer[verticesCounter + 14].b = b;
					verticesBuffer[verticesCounter + 14].a = 1.0f;
					verticesBuffer[verticesCounter + 14].nx = 1.0f;
					verticesBuffer[verticesCounter + 14].ny = 0.0f;
					verticesBuffer[verticesCounter + 14].nz = 0.0f;

					verticesBuffer[verticesCounter + 15].x = x + 0.5f;
					verticesBuffer[verticesCounter + 15].y = y + 0.5f;
					verticesBuffer[verticesCounter + 15].z = z + 0.5f;
					verticesBuffer[verticesCounter + 15].r = r;
					verticesBuffer[verticesCounter + 15].g = g;
					verticesBuffer[verticesCounter + 15].b = b;
					verticesBuffer[verticesCounter + 15].a = 1.0f;
					verticesBuffer[verticesCounter + 15].nx = 1.0f;
					verticesBuffer[verticesCounter + 15].ny = 0.0f;
					verticesBuffer[verticesCounter + 15].nz = 0.0f;

					// Top 16 17 18 19
					verticesBuffer[verticesCounter + 16].x = x + -0.5f;
					verticesBuffer[verticesCounter + 16].y = y + 0.5f;
					verticesBuffer[verticesCounter + 16].z = z + 0.5f;
					verticesBuffer[verticesCounter + 16].r = r;
					verticesBuffer[verticesCounter + 16].g = g;
					verticesBuffer[verticesCounter + 16].b = b;
					verticesBuffer[verticesCounter + 16].a = 1.0f;
					verticesBuffer[verticesCounter + 16].nx = 0.0f;
					verticesBuffer[verticesCounter + 16].ny = 1.0f;
					verticesBuffer[verticesCounter + 16].nz = 0.0f;

					verticesBuffer[verticesCounter + 17].x = x + 0.5f;
					verticesBuffer[verticesCounter + 17].y = y + 0.5f;
					verticesBuffer[verticesCounter + 17].z = z + 0.5f;
					verticesBuffer[verticesCounter + 17].r = r;
					verticesBuffer[verticesCounter + 17].g = g;
					verticesBuffer[verticesCounter + 17].b = b;
					verticesBuffer[verticesCounter + 17].a = 1.0f;
					verticesBuffer[verticesCounter + 17].nx = 0.0f;
					verticesBuffer[verticesCounter + 17].ny = 1.0f;
					verticesBuffer[verticesCounter + 17].nz = 0.0f;

					verticesBuffer[verticesCounter + 18].x = x + -0.5f;
					verticesBuffer[verticesCounter + 18].y = y + 0.5f;
					verticesBuffer[verticesCounter + 18].z = z + -0.5f;
					verticesBuffer[verticesCounter + 18].r = r;
					verticesBuffer[verticesCounter + 18].g = g;
					verticesBuffer[verticesCounter + 18].b = b;
					verticesBuffer[verticesCounter + 18].a = 1.0f;
					verticesBuffer[verticesCounter + 18].nx = 0.0f;
					verticesBuffer[verticesCounter + 18].ny = 1.0f;
					verticesBuffer[verticesCounter + 18].nz = 0.0f;

					verticesBuffer[verticesCounter + 19].x = x + 0.5f;
					verticesBuffer[verticesCounter + 19].y = y + 0.5f;
					verticesBuffer[verticesCounter + 19].z = z + -0.5f;
					verticesBuffer[verticesCounter + 19].r = r;
					verticesBuffer[verticesCounter + 19].g = g;
					verticesBuffer[verticesCounter + 19].b = b;
					verticesBuffer[verticesCounter + 19].a = 1.0f;
					verticesBuffer[verticesCounter + 19].nx = 0.0f;
					verticesBuffer[verticesCounter + 19].ny = 1.0f;
					verticesBuffer[verticesCounter + 19].nz = 0.0f;

					// Bottom 20 21 22 23
					verticesBuffer[verticesCounter + 20].x = x + -0.5f;
					verticesBuffer[verticesCounter + 20].y = y + -0.5f;
					verticesBuffer[verticesCounter + 20].z = z + 0.5f;
					verticesBuffer[verticesCounter + 20].r = r;
					verticesBuffer[verticesCounter + 20].g = g;
					verticesBuffer[verticesCounter + 20].b = b;
					verticesBuffer[verticesCounter + 20].a = 1.0f;
					verticesBuffer[verticesCounter + 20].nx = 0.0f;
					verticesBuffer[verticesCounter + 20].ny = -1.0f;
					verticesBuffer[verticesCounter + 20].nz = 0.0f;

					verticesBuffer[verticesCounter + 21].x = x + 0.5f;
					verticesBuffer[verticesCounter + 21].y = y + -0.5f;
					verticesBuffer[verticesCounter + 21].z = z + 0.5f;
					verticesBuffer[verticesCounter + 21].r = r;
					verticesBuffer[verticesCounter + 21].g = g;
					verticesBuffer[verticesCounter + 21].b = b;
					verticesBuffer[verticesCounter + 21].a = 1.0f;
					verticesBuffer[verticesCounter + 21].nx = 0.0f;
					verticesBuffer[verticesCounter + 21].ny = -1.0f;
					verticesBuffer[verticesCounter + 21].nz = 0.0f;

					verticesBuffer[verticesCounter + 22].x = x + -0.5f;
					verticesBuffer[verticesCounter + 22].y = y + -0.5f;
					verticesBuffer[verticesCounter + 22].z = z + -0.5f;
					verticesBuffer[verticesCounter + 22].r = r;
					verticesBuffer[verticesCounter + 22].g = g;
					verticesBuffer[verticesCounter + 22].b = b;
					verticesBuffer[verticesCounter + 22].a = 1.0f;
					verticesBuffer[verticesCounter + 22].nx = 0.0f;
					verticesBuffer[verticesCounter + 22].ny = -1.0f;
					verticesBuffer[verticesCounter + 22].nz = 0.0f;

					verticesBuffer[verticesCounter + 23].x = x + 0.5f;
					verticesBuffer[verticesCounter + 23].y = y + -0.5f;
					verticesBuffer[verticesCounter + 23].z = z + -0.5f;
					verticesBuffer[verticesCounter + 23].r = r;
					verticesBuffer[verticesCounter + 23].g = g;
					verticesBuffer[verticesCounter + 23].b = b;
					verticesBuffer[verticesCounter + 23].a = 1.0f;
					verticesBuffer[verticesCounter + 23].nx = 0.0f;
					verticesBuffer[verticesCounter + 23].ny = -1.0f;
					verticesBuffer[verticesCounter + 23].nz = 0.0f;

					// Back
					if (m_createInnerFaces == true || (mask & 32) == 32)
					{
						indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
						indicesBuffer[indicesCounter + 1] = verticesCounter + 2;
						indicesBuffer[indicesCounter + 2] = verticesCounter + 1;
						indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
						indicesBuffer[indicesCounter + 4] = verticesCounter + 2;
						indicesBuffer[indicesCounter + 5] = verticesCounter + 3;

						indicesCounter += 6;
					}

					// Front
					if (m_createInnerFaces == true || (mask & 64) == 64)
					{
						indicesBuffer[indicesCounter + 0] = verticesCounter + 4;
						indicesBuffer[indicesCounter + 1] = verticesCounter + 5;
						indicesBuffer[indicesCounter + 2] = verticesCounter + 6;
						indicesBuffer[indicesCounter + 3] = verticesCounter + 5;
						indicesBuffer[indicesCounter + 4] = verticesCounter + 7;
						indicesBuffer[indicesCounter + 5] = verticesCounter + 6;

						indicesCounter += 6;
					}

					// Left
					if (m_createInnerFaces == true || (mask & 4) == 4)
					{
						indicesBuffer[indicesCounter + 0] = verticesCounter + 9;
						indicesBuffer[indicesCounter + 1] = verticesCounter + 10;
						indicesBuffer[indicesCounter + 2] = verticesCounter + 8;
						indicesBuffer[indicesCounter + 3] = verticesCounter + 9;
						indicesBuffer[indicesCounter + 4] = verticesCounter + 11;
						indicesBuffer[indicesCounter + 5] = verticesCounter + 10;

						indicesCounter += 6;
					}

					// Right
					if (m_createInnerFaces == true || (mask & 2) == 2)
					{
						indicesBuffer[indicesCounter + 0] = verticesCounter + 12;
						indicesBuffer[indicesCounter + 1] = verticesCounter + 14;
						indicesBuffer[indicesCounter + 2] = verticesCounter + 13;
						indicesBuffer[indicesCounter + 3] = verticesCounter + 13;
						indicesBuffer[indicesCounter + 4] = verticesCounter + 14;
						indicesBuffer[indicesCounter + 5] = verticesCounter + 15;

						indicesCounter += 6;
					}

					// Top
					if (m_createInnerFaces == true || (mask & 8) == 8)
					{
						indicesBuffer[indicesCounter + 0] = verticesCounter + 16;
						indicesBuffer[indicesCounter + 1] = verticesCounter + 17;
						indicesBuffer[indicesCounter + 2] = verticesCounter + 18;
						indicesBuffer[indicesCounter + 3] = verticesCounter + 17;
						indicesBuffer[indicesCounter + 4] = verticesCounter + 19;
						indicesBuffer[indicesCounter + 5] = verticesCounter + 18;

						indicesCounter += 6;
					}

					// Bottom
					if (m_createInnerFaces == true || (mask & 16) == 16)
					{
						indicesBuffer[indicesCounter + 0] = verticesCounter + 20;
						indicesBuffer[indicesCounter + 1] = verticesCounter + 22;
						indicesBuffer[indicesCounter + 2] = verticesCounter + 21;
						indicesBuffer[indicesCounter + 3] = verticesCounter + 21;
						indicesBuffer[indicesCounter + 4] = verticesCounter + 22;
						indicesBuffer[indicesCounter + 5] = verticesCounter + 23;

						indicesCounter += 6;
					}

					verticesCounter += 24;
				}
			}
		}

		glGenVertexArrays(1, &pMatrix->m_VAO);
		glGenBuffers(1, &pMatrix->m_VBO);
		glGenBuffers(1, &pMatrix->m_EBO);

		// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
		glBindVertexArray(pMatrix->m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, pMatrix->m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(PositionColorNormalVertex)*pMatrix->m_numVertices, verticesBuffer, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMatrix->m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*pMatrix->m_numIndices, indicesBuffer, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 10, (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 10, (GLvoid*)(sizeof(GLfloat) * 3));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 10, (GLvoid*)(sizeof(GLfloat) * 7));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

		glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
	}
}

// Accessors
string QBT::GetFilename()
{
	return m_filename;
}

int QBT::GetNumMatrices()
{
	int numMatrices = (int)m_vpQBTMatrices.size();
	return numMatrices;
}

int QBT::GetNumVertices()
{
	int numVertices = 0;
	for (int i = 0; i < (int)m_vpQBTMatrices.size(); i++)
	{
		numVertices += m_vpQBTMatrices[i]->m_numVertices;
	}
	return numVertices;
}

int QBT::GetNumTriangles()
{
	int numTriangles = 0;
	for (int i = 0; i < (int)m_vpQBTMatrices.size(); i++)
	{
		numTriangles += m_vpQBTMatrices[i]->m_numTriangles;
	}
	return numTriangles;
}

// Render modes
void QBT::SetWireframeMode(bool wireframe)
{
	m_wireframeRender = wireframe;
}

bool QBT::GetWireframeMode()
{
	return m_wireframeRender;
}

// Creation optimizations
void QBT::SetCreateInnerVoxels(bool innerVoxels)
{
	m_createInnerVoxels = innerVoxels;
}

void QBT::SetCreateInnerFaces(bool innerFaces)
{
	m_createInnerFaces = innerFaces;
}

void QBT::SetMergeFaces(bool mergeFaces)
{
	m_mergeFaces = mergeFaces;
}

// Render
void QBT::Render(Camera* pCamera)
{
	if (m_wireframeRender)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Use shader
	m_pPositionColorNormalShader->UseShader();

	glm::vec3 lightPos(0.0f, 15.0f, 15.0f);
	GLint lightColorLoc = glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "lightColor");
	GLint lightPosLoc = glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "lightPos");
	GLint viewPosLoc = glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "viewPos");
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(viewPosLoc, pCamera->GetPosition().x, pCamera->GetPosition().y, pCamera->GetPosition().z);

	for (unsigned int matrixIndex = 0; matrixIndex < m_vpQBTMatrices.size(); matrixIndex++)
	{
		QBTMatrix* pMatrix = m_vpQBTMatrices[matrixIndex];

		// Create transformations
		mat4 view;
		mat4 projection;
		view = lookAt(pCamera->GetPosition(), pCamera->GetView(), pCamera->GetUp());
		projection = perspective(45.0f, (GLfloat)QubeGame::GetInstance()->GetWindowWidth() / (GLfloat)QubeGame::GetInstance()->GetWindowHeight(), 0.01f, 1000.0f);

		// Get their uniform location
		GLint modelLoc = glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "model");
		GLint viewLoc = glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "view");
		GLint projLoc = glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
		// Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

		glBindVertexArray(pMatrix->m_VAO);

		mat4 model;
		model = translate(model, vec3(pMatrix->m_positionX, pMatrix->m_positionY, pMatrix->m_positionZ));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

		glDrawElements(GL_TRIANGLES, pMatrix->m_numIndices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}