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
	m_useLighting = true;
	m_boundingBox = false;

	// Creation optimizations
	m_createInnerVoxels = false;
	m_createInnerFaces = false;
	m_mergeFaces = false;

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

		delete m_vpQBTMatrices[i]->m_pMaterial;

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

	// Material
	pNewMatrix->m_pMaterial = new Material();
	pNewMatrix->m_pMaterial->m_ambient = Colour(1.0f, 1.0f, 1.0f);
	pNewMatrix->m_pMaterial->m_diffuse = Colour(1.0f, 1.0f, 1.0f);
	pNewMatrix->m_pMaterial->m_specular = Colour(1.0f, 1.0f, 1.0f);
	pNewMatrix->m_pMaterial->m_emission = Colour(0.0f, 0.0f, 0.0f);
	pNewMatrix->m_pMaterial->m_shininess = 64.0f;

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

		pMatrix->m_numVertices = 0;
		pMatrix->m_numTriangles = 0;
		pMatrix->m_numIndices = 0;

		if (m_mergeFaces)
		{
			int cubeSize = pMatrix->m_sizeX * pMatrix->m_sizeY * pMatrix->m_sizeZ;
			int *l_merged;
			l_merged = new int[cubeSize];

			for (int i = 0; i < cubeSize; i++)
			{
				l_merged[i] = MergedSide_None;
			}

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

						float r = (float)(red / 255.0f);
						float g = (float)(green / 255.0f);
						float b = (float)(blue / 255.0f);

						if (mask == 0)
						{
							continue;
						}

						if (mask == 1 && m_createInnerVoxels == false)
						{
							continue;
						}

						int merged = l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

						// Back
						if ((mask & 32) == 32)
						{
							if ((merged & MergedSide_Z_Negative) != MergedSide_Z_Negative)
							{
								bool stopMerging = false;
								int increaseX = 0;
								for (unsigned int x1 = x + 1; x1 < pMatrix->m_sizeX && stopMerging == false; x1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Z_Negative) == MergedSide_Z_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 32) != 32)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseX++;
									l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)] |= MergedSide_Z_Negative;
								}

								stopMerging = false;
								int increaseY = 0;
								for (unsigned int y1 = y + 1; y1 < pMatrix->m_sizeY && stopMerging == false; y1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Z_Negative) == MergedSide_Z_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 32) != 32)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingX = false;
									for (int xAdd = 1; xAdd <= increaseX && stopMergingX == false; xAdd++)
									{
										unsigned int x1 = x + xAdd;

										unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
										int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

										if ((merged1 & MergedSide_Z_Negative) == MergedSide_Z_Negative)
										{
											stopMergingX = true;
											continue;
										}
										if ((mask1 & 32) != 32)
										{
											stopMergingX = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingX = true;
											continue;
										}
									}

									if (stopMergingX == true)
									{
										stopMerging = true;
										continue;
									}

									increaseY++;
									l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_Z_Negative;

									for (int xAdd = 1; xAdd <= increaseX; xAdd++)
									{
										unsigned int x1 = x + xAdd;
										l_merged[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_Z_Negative;
									}
								}

								pMatrix->m_numVertices += 4;
								pMatrix->m_numTriangles += 2;
							}
						}

						// Front
						if ((mask & 64) == 64)
						{
							if ((merged & MergedSide_Z_Positive) != MergedSide_Z_Positive)
							{
								bool stopMerging = false;
								int increaseX = 0;
								for (unsigned int x1 = x + 1; x1 < pMatrix->m_sizeX && stopMerging == false; x1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Z_Positive) == MergedSide_Z_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 64) != 64)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseX++;
									l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)] |= MergedSide_Z_Positive;
								}

								stopMerging = false;
								int increaseY = 0;
								for (unsigned int y1 = y + 1; y1 < pMatrix->m_sizeY && stopMerging == false; y1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Z_Positive) == MergedSide_Z_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 64) != 64)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingX = false;
									for (int xAdd = 1; xAdd <= increaseX && stopMergingX == false; xAdd++)
									{
										unsigned int x1 = x + xAdd;

										unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
										int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

										if ((merged1 & MergedSide_Z_Positive) == MergedSide_Z_Positive)
										{
											stopMergingX = true;
											continue;
										}
										if ((mask1 & 64) != 64)
										{
											stopMergingX = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingX = true;
											continue;
										}
									}

									if (stopMergingX == true)
									{
										stopMerging = true;
										continue;
									}

									increaseY++;
									l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_Z_Positive;

									for (int xAdd = 1; xAdd <= increaseX; xAdd++)
									{
										unsigned int x1 = x + xAdd;
										l_merged[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_Z_Positive;
									}
								}

								pMatrix->m_numVertices += 4;
								pMatrix->m_numTriangles += 2;
							}
						}

						// Left
						if ((mask & 4) == 4)
						{
							if ((merged & MergedSide_X_Negative) != MergedSide_X_Negative)
							{
								bool stopMerging = false;
								int increaseZ = 0;
								for (unsigned int z1 = z + 1; z1 < pMatrix->m_sizeZ && stopMerging == false; z1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

									if ((merged1 & MergedSide_X_Negative) == MergedSide_X_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 4) != 4)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseZ++;
									l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_X_Negative;
								}

								stopMerging = false;
								int increaseY = 0;
								for (unsigned int y1 = y + 1; y1 < pMatrix->m_sizeY && stopMerging == false; y1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_X_Negative) == MergedSide_X_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 4) != 4)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingZ = false;
									for (int zAdd = 1; zAdd <= increaseZ && stopMergingZ == false; zAdd++)
									{
										unsigned int z1 = z + zAdd;

										unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];
										int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];

										if ((merged1 & MergedSide_X_Negative) == MergedSide_X_Negative)
										{
											stopMergingZ = true;
											continue;
										}
										if ((mask1 & 4) != 4)
										{
											stopMergingZ = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingZ = true;
											continue;
										}
									}

									if (stopMergingZ == true)
									{
										stopMerging = true;
										continue;
									}

									increaseY++;
									l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_X_Negative;

									for (int zAdd = 1; zAdd <= increaseZ; zAdd++)
									{
										unsigned int z1 = z + zAdd;
										l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)] |= MergedSide_X_Negative;
									}
								}

								pMatrix->m_numVertices += 4;
								pMatrix->m_numTriangles += 2;
							}
						}

						// Right
						if ((mask & 2) == 2)
						{
							if ((merged & MergedSide_X_Positive) != MergedSide_X_Positive)
							{
								bool stopMerging = false;
								int increaseZ = 0;
								for (unsigned int z1 = z + 1; z1 < pMatrix->m_sizeZ && stopMerging == false; z1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

									if ((merged1 & MergedSide_X_Positive) == MergedSide_X_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 2) != 2)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseZ++;
									l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_X_Positive;
								}

								stopMerging = false;
								int increaseY = 0;
								for (unsigned int y1 = y + 1; y1 < pMatrix->m_sizeY && stopMerging == false; y1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_X_Positive) == MergedSide_X_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 2) != 2)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingZ = false;
									for (int zAdd = 1; zAdd <= increaseZ && stopMergingZ == false; zAdd++)
									{
										unsigned int z1 = z + zAdd;

										unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];
										int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];

										if ((merged1 & MergedSide_X_Positive) == MergedSide_X_Positive)
										{
											stopMergingZ = true;
											continue;
										}
										if ((mask1 & 2) != 2)
										{
											stopMergingZ = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingZ = true;
											continue;
										}
									}

									if (stopMergingZ == true)
									{
										stopMerging = true;
										continue;
									}

									increaseY++;
									l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_X_Positive;

									for (int zAdd = 1; zAdd <= increaseZ; zAdd++)
									{
										unsigned int z1 = z + zAdd;
										l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)] |= MergedSide_X_Positive;
									}
								}

								pMatrix->m_numVertices += 4;
								pMatrix->m_numTriangles += 2;
							}
						}

						// Top
						if ((mask & 8) == 8)
						{
							if ((merged & MergedSide_Y_Positive) != MergedSide_Y_Positive)
							{
								bool stopMerging = false;
								int increaseZ = 0;
								for (unsigned int z1 = z + 1; z1 < pMatrix->m_sizeZ && stopMerging == false; z1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

									if ((merged1 & MergedSide_Y_Positive) == MergedSide_Y_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 8) != 8)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseZ++;
									l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_Y_Positive;
								}

								stopMerging = false;
								int increaseX = 0;
								for (unsigned int x1 = x + 1; x1 < pMatrix->m_sizeX && stopMerging == false; x1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Y_Positive) == MergedSide_Y_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 8) != 8)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingZ = false;
									for (int zAdd = 1; zAdd <= increaseZ && stopMergingZ == false; zAdd++)
									{
										unsigned int z1 = z + zAdd;

										unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
										int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

										if ((merged1 & MergedSide_Y_Positive) == MergedSide_Y_Positive)
										{
											stopMergingZ = true;
											continue;
										}
										if ((mask1 & 8) != 8)
										{
											stopMergingZ = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingZ = true;
											continue;
										}
									}

									if (stopMergingZ == true)
									{
										stopMerging = true;
										continue;
									}

									increaseX++;
									l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)] |= MergedSide_Y_Positive;

									for (int zAdd = 1; zAdd <= increaseZ; zAdd++)
									{
										unsigned int z1 = z + zAdd;
										l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_Y_Positive;
									}
								}

								pMatrix->m_numVertices += 4;
								pMatrix->m_numTriangles += 2;
							}
						}

						// Bottom
						if ((mask & 16) == 16)
						{
							if ((merged & MergedSide_Y_Negative) != MergedSide_Y_Negative)
							{
								bool stopMerging = false;
								int increaseZ = 0;
								for (unsigned int z1 = z + 1; z1 < pMatrix->m_sizeZ && stopMerging == false; z1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

									if ((merged1 & MergedSide_Y_Negative) == MergedSide_Y_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 16) != 16)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseZ++;
									l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_Y_Negative;
								}

								stopMerging = false;
								int increaseX = 0;
								for (unsigned int x1 = x + 1; x1 < pMatrix->m_sizeX && stopMerging == false; x1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Y_Negative) == MergedSide_Y_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 16) != 16)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingZ = false;
									for (int zAdd = 1; zAdd <= increaseZ && stopMergingZ == false; zAdd++)
									{
										unsigned int z1 = z + zAdd;

										unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
										int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

										if ((merged1 & MergedSide_Y_Negative) == MergedSide_Y_Negative)
										{
											stopMergingZ = true;
											continue;
										}
										if ((mask1 & 16) != 16)
										{
											stopMergingZ = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingZ = true;
											continue;
										}
									}

									if (stopMergingZ == true)
									{
										stopMerging = true;
										continue;
									}

									increaseX++;
									l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)] |= MergedSide_Y_Negative;

									for (int zAdd = 1; zAdd <= increaseZ; zAdd++)
									{
										unsigned int z1 = z + zAdd;
										l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_Y_Negative;
									}
								}

								pMatrix->m_numVertices += 4;
								pMatrix->m_numTriangles += 2;
							}
						}
					}
				}
			}
		}
		else
		{
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
								// Back
								if (m_createInnerFaces == true || (mask & 32) == 32)
								{
									pMatrix->m_numVertices += 4;
									pMatrix->m_numTriangles += 2;
								}

								// Front
								if (m_createInnerFaces == true || (mask & 64) == 64)
								{
									pMatrix->m_numVertices += 4;
									pMatrix->m_numTriangles += 2;
								}

								// Left
								if (m_createInnerFaces == true || (mask & 4) == 4)
								{
									pMatrix->m_numVertices += 4;
									pMatrix->m_numTriangles += 2;
								}

								// Right
								if (m_createInnerFaces == true || (mask & 2) == 2)
								{
									pMatrix->m_numVertices += 4;
									pMatrix->m_numTriangles += 2;
								}

								// Top
								if (m_createInnerFaces == true || (mask & 8) == 8)
								{
									pMatrix->m_numVertices += 4;
									pMatrix->m_numTriangles += 2;
								}

								// Bottom
								if (m_createInnerFaces == true || (mask & 16) == 16)
								{
									pMatrix->m_numVertices += 4;
									pMatrix->m_numTriangles += 2;
								}
							}
						}
					}
				}
			}
		}

		// Indices
		pMatrix->m_numIndices = (unsigned int)pMatrix->m_numTriangles * 3;
	}
}

void QBT::RecreateStaticBuffers()
{
	DestroyStaticBuffers();
	SetVisibilityInformation();
	CreateStaticRenderBuffers();
}

void QBT::CreateStaticRenderBuffers()
{
	for (unsigned int matrixIndex = 0; matrixIndex < m_vpQBTMatrices.size(); matrixIndex++)
	{
		QBTMatrix* pMatrix = m_vpQBTMatrices[matrixIndex];

		// Vertices
		PositionColorNormalVertex* verticesBuffer = new PositionColorNormalVertex[pMatrix->m_numVertices];

		// Indices
		GLuint* indicesBuffer = new GLuint[pMatrix->m_numIndices];

		if(m_mergeFaces)
		{
			int cubeSize = pMatrix->m_sizeX * pMatrix->m_sizeY * pMatrix->m_sizeZ;
			int *l_merged;
			l_merged = new int[cubeSize];

			for (int i = 0; i < cubeSize; i++)
			{
				l_merged[i] = MergedSide_None;
			}

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

						float r = (float)(red / 255.0f);
						float g = (float)(green / 255.0f);
						float b = (float)(blue / 255.0f);

						if (mask == 0)
						{
							continue;
						}

						if (mask == 1 && m_createInnerVoxels == false)
						{
							continue;
						}

						int merged = l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

						// Back
						if ((mask & 32) == 32)
						{
							if ((merged & MergedSide_Z_Negative) != MergedSide_Z_Negative)
							{
								bool stopMerging = false;
								int increaseX = 0;
								for (unsigned int x1 = x + 1; x1 < pMatrix->m_sizeX && stopMerging == false; x1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Z_Negative) == MergedSide_Z_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 32) != 32)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseX++;
									l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)] |= MergedSide_Z_Negative;
								}

								stopMerging = false;
								int increaseY = 0;
								for (unsigned int y1 = y + 1; y1 < pMatrix->m_sizeY && stopMerging == false; y1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Z_Negative) == MergedSide_Z_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 32) != 32)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingX = false;
									for (int xAdd = 1; xAdd <= increaseX && stopMergingX == false; xAdd++)
									{
										unsigned int x1 = x + xAdd;

										unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
										int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

										if ((merged1 & MergedSide_Z_Negative) == MergedSide_Z_Negative)
										{
											stopMergingX = true;
											continue;
										}
										if ((mask1 & 32) != 32)
										{
											stopMergingX = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingX = true;
											continue;
										}
									}

									if (stopMergingX == true)
									{
										stopMerging = true;
										continue;
									}

									increaseY++;
									l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_Z_Negative;

									for (int xAdd = 1; xAdd <= increaseX; xAdd++)
									{
										unsigned int x1 = x + xAdd;
										l_merged[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_Z_Negative;
									}
								}

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

								verticesBuffer[verticesCounter + 1].x = x + 0.5f + (1.0f*increaseX);
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
								verticesBuffer[verticesCounter + 2].y = y + 0.5f + (1.0f*increaseY);
								verticesBuffer[verticesCounter + 2].z = z + -0.5f;
								verticesBuffer[verticesCounter + 2].r = r;
								verticesBuffer[verticesCounter + 2].g = g;
								verticesBuffer[verticesCounter + 2].b = b;
								verticesBuffer[verticesCounter + 2].a = 1.0f;
								verticesBuffer[verticesCounter + 2].nx = 0.0f;
								verticesBuffer[verticesCounter + 2].ny = 0.0f;
								verticesBuffer[verticesCounter + 2].nz = -1.0f;

								verticesBuffer[verticesCounter + 3].x = x + 0.5f + (1.0f*increaseX);
								verticesBuffer[verticesCounter + 3].y = y + 0.5f + (1.0f*increaseY);
								verticesBuffer[verticesCounter + 3].z = z + -0.5f;
								verticesBuffer[verticesCounter + 3].r = r;
								verticesBuffer[verticesCounter + 3].g = g;
								verticesBuffer[verticesCounter + 3].b = b;
								verticesBuffer[verticesCounter + 3].a = 1.0f;
								verticesBuffer[verticesCounter + 3].nx = 0.0f;
								verticesBuffer[verticesCounter + 3].ny = 0.0f;
								verticesBuffer[verticesCounter + 3].nz = -1.0f;

								indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
								indicesBuffer[indicesCounter + 1] = verticesCounter + 2;
								indicesBuffer[indicesCounter + 2] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 4] = verticesCounter + 2;
								indicesBuffer[indicesCounter + 5] = verticesCounter + 3;

								indicesCounter += 6;
								verticesCounter += 4;
							}
						}

						// Front
						if ((mask & 64) == 64)
						{
							if ((merged & MergedSide_Z_Positive) != MergedSide_Z_Positive)
							{
								bool stopMerging = false;
								int increaseX = 0;
								for (unsigned int x1 = x + 1; x1 < pMatrix->m_sizeX && stopMerging == false; x1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Z_Positive) == MergedSide_Z_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 64) != 64)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseX++;
									l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)] |= MergedSide_Z_Positive;
								}

								stopMerging = false;
								int increaseY = 0;
								for (unsigned int y1 = y + 1; y1 < pMatrix->m_sizeY && stopMerging == false; y1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Z_Positive) == MergedSide_Z_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 64) != 64)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingX = false;
									for (int xAdd = 1; xAdd <= increaseX && stopMergingX == false; xAdd++)
									{
										unsigned int x1 = x + xAdd;

										unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
										int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

										if ((merged1 & MergedSide_Z_Positive) == MergedSide_Z_Positive)
										{
											stopMergingX = true;
											continue;
										}
										if ((mask1 & 64) != 64)
										{
											stopMergingX = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingX = true;
											continue;
										}
									}

									if (stopMergingX == true)
									{
										stopMerging = true;
										continue;
									}

									increaseY++;
									l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_Z_Positive;

									for (int xAdd = 1; xAdd <= increaseX; xAdd++)
									{
										unsigned int x1 = x + xAdd;
										l_merged[x1 + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_Z_Positive;
									}
								}

								verticesBuffer[verticesCounter + 0].x = x + -0.5f;
								verticesBuffer[verticesCounter + 0].y = y + -0.5f;
								verticesBuffer[verticesCounter + 0].z = z + 0.5f;
								verticesBuffer[verticesCounter + 0].r = r;
								verticesBuffer[verticesCounter + 0].g = g;
								verticesBuffer[verticesCounter + 0].b = b;
								verticesBuffer[verticesCounter + 0].a = 1.0f;
								verticesBuffer[verticesCounter + 0].nx = 0.0f;
								verticesBuffer[verticesCounter + 0].ny = 0.0f;
								verticesBuffer[verticesCounter + 0].nz = 1.0f;

								verticesBuffer[verticesCounter + 1].x = x + 0.5f + (1.0f*increaseX);
								verticesBuffer[verticesCounter + 1].y = y + -0.5f;
								verticesBuffer[verticesCounter + 1].z = z + 0.5f;
								verticesBuffer[verticesCounter + 1].r = r;
								verticesBuffer[verticesCounter + 1].g = g;
								verticesBuffer[verticesCounter + 1].b = b;
								verticesBuffer[verticesCounter + 1].a = 1.0f;
								verticesBuffer[verticesCounter + 1].nx = 0.0f;
								verticesBuffer[verticesCounter + 1].ny = 0.0f;
								verticesBuffer[verticesCounter + 1].nz = 1.0f;

								verticesBuffer[verticesCounter + 2].x = x + -0.5f;
								verticesBuffer[verticesCounter + 2].y = y + 0.5f + (1.0f*increaseY);
								verticesBuffer[verticesCounter + 2].z = z + 0.5f;
								verticesBuffer[verticesCounter + 2].r = r;
								verticesBuffer[verticesCounter + 2].g = g;
								verticesBuffer[verticesCounter + 2].b = b;
								verticesBuffer[verticesCounter + 2].a = 1.0f;
								verticesBuffer[verticesCounter + 2].nx = 0.0f;
								verticesBuffer[verticesCounter + 2].ny = 0.0f;
								verticesBuffer[verticesCounter + 2].nz = 1.0f;

								verticesBuffer[verticesCounter + 3].x = x + 0.5f + (1.0f*increaseX);
								verticesBuffer[verticesCounter + 3].y = y + 0.5f + (1.0f*increaseY);
								verticesBuffer[verticesCounter + 3].z = z + 0.5f;
								verticesBuffer[verticesCounter + 3].r = r;
								verticesBuffer[verticesCounter + 3].g = g;
								verticesBuffer[verticesCounter + 3].b = b;
								verticesBuffer[verticesCounter + 3].a = 1.0f;
								verticesBuffer[verticesCounter + 3].nx = 0.0f;
								verticesBuffer[verticesCounter + 3].ny = 0.0f;
								verticesBuffer[verticesCounter + 3].nz = 1.0f;

								indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
								indicesBuffer[indicesCounter + 1] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 2] = verticesCounter + 2;
								indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 4] = verticesCounter + 3;
								indicesBuffer[indicesCounter + 5] = verticesCounter + 2;

								indicesCounter += 6;
								verticesCounter += 4;
							}
						}

						// Left
						if ((mask & 4) == 4)
						{
							if ((merged & MergedSide_X_Negative) != MergedSide_X_Negative)
							{
								bool stopMerging = false;
								int increaseZ = 0;
								for (unsigned int z1 = z + 1; z1 < pMatrix->m_sizeZ && stopMerging == false; z1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

									if ((merged1 & MergedSide_X_Negative) == MergedSide_X_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 4) != 4)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseZ++;
									l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_X_Negative;
								}

								stopMerging = false;
								int increaseY = 0;
								for (unsigned int y1 = y + 1; y1 < pMatrix->m_sizeY && stopMerging == false; y1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_X_Negative) == MergedSide_X_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 4) != 4)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingZ = false;
									for (int zAdd = 1; zAdd <= increaseZ && stopMergingZ == false; zAdd++)
									{
										unsigned int z1 = z + zAdd;

										unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];
										int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];

										if ((merged1 & MergedSide_X_Negative) == MergedSide_X_Negative)
										{
											stopMergingZ = true;
											continue;
										}
										if ((mask1 & 4) != 4)
										{
											stopMergingZ = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingZ = true;
											continue;
										}
									}

									if (stopMergingZ == true)
									{
										stopMerging = true;
										continue;
									}

									increaseY++;
									l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_X_Negative;

									for (int zAdd = 1; zAdd <= increaseZ; zAdd++)
									{
										unsigned int z1 = z + zAdd;
										l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)] |= MergedSide_X_Negative;
									}
								}

								verticesBuffer[verticesCounter + 0].x = x + -0.5f;
								verticesBuffer[verticesCounter + 0].y = y + -0.5f;
								verticesBuffer[verticesCounter + 0].z = z + -0.5f;
								verticesBuffer[verticesCounter + 0].r = r;
								verticesBuffer[verticesCounter + 0].g = g;
								verticesBuffer[verticesCounter + 0].b = b;
								verticesBuffer[verticesCounter + 0].a = 1.0f;
								verticesBuffer[verticesCounter + 0].nx = -1.0f;
								verticesBuffer[verticesCounter + 0].ny = 0.0f;
								verticesBuffer[verticesCounter + 0].nz = 0.0f;

								verticesBuffer[verticesCounter + 1].x = x + -0.5f;
								verticesBuffer[verticesCounter + 1].y = y + -0.5f;
								verticesBuffer[verticesCounter + 1].z = z + 0.5f + (1.0f*increaseZ);
								verticesBuffer[verticesCounter + 1].r = r;
								verticesBuffer[verticesCounter + 1].g = g;
								verticesBuffer[verticesCounter + 1].b = b;
								verticesBuffer[verticesCounter + 1].a = 1.0f;
								verticesBuffer[verticesCounter + 1].nx = -1.0f;
								verticesBuffer[verticesCounter + 1].ny = 0.0f;
								verticesBuffer[verticesCounter + 1].nz = 0.0f;

								verticesBuffer[verticesCounter + 2].x = x + -0.5f;
								verticesBuffer[verticesCounter + 2].y = y + 0.5f + (1.0f*increaseY);
								verticesBuffer[verticesCounter + 2].z = z + -0.5f;
								verticesBuffer[verticesCounter + 2].r = r;
								verticesBuffer[verticesCounter + 2].g = g;
								verticesBuffer[verticesCounter + 2].b = b;
								verticesBuffer[verticesCounter + 2].a = 1.0f;
								verticesBuffer[verticesCounter + 2].nx = -1.0f;
								verticesBuffer[verticesCounter + 2].ny = 0.0f;
								verticesBuffer[verticesCounter + 2].nz = 0.0f;

								verticesBuffer[verticesCounter + 3].x = x + -0.5f;
								verticesBuffer[verticesCounter + 3].y = y + 0.5f + (1.0f*increaseY);
								verticesBuffer[verticesCounter + 3].z = z + 0.5f + (1.0f*increaseZ);
								verticesBuffer[verticesCounter + 3].r = r;
								verticesBuffer[verticesCounter + 3].g = g;
								verticesBuffer[verticesCounter + 3].b = b;
								verticesBuffer[verticesCounter + 3].a = 1.0f;
								verticesBuffer[verticesCounter + 3].nx = -1.0f;
								verticesBuffer[verticesCounter + 3].ny = 0.0f;
								verticesBuffer[verticesCounter + 3].nz = 0.0f;

								indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
								indicesBuffer[indicesCounter + 1] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 2] = verticesCounter + 2;
								indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 4] = verticesCounter + 3;
								indicesBuffer[indicesCounter + 5] = verticesCounter + 2;

								indicesCounter += 6;
								verticesCounter += 4;
							}
						}

						// Right
						if ((mask & 2) == 2)
						{
							if ((merged & MergedSide_X_Positive) != MergedSide_X_Positive)
							{
								bool stopMerging = false;
								int increaseZ = 0;
								for (unsigned int z1 = z + 1; z1 < pMatrix->m_sizeZ && stopMerging == false; z1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

									if ((merged1 & MergedSide_X_Positive) == MergedSide_X_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 2) != 2)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseZ++;
									l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_X_Positive;
								}

								stopMerging = false;
								int increaseY = 0;
								for (unsigned int y1 = y + 1; y1 < pMatrix->m_sizeY && stopMerging == false; y1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_X_Positive) == MergedSide_X_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 2) != 2)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingZ = false;
									for (int zAdd = 1; zAdd <= increaseZ && stopMergingZ == false; zAdd++)
									{
										unsigned int z1 = z + zAdd;

										unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];
										int merged1 = l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)];

										if ((merged1 & MergedSide_X_Positive) == MergedSide_X_Positive)
										{
											stopMergingZ = true;
											continue;
										}
										if ((mask1 & 2) != 2)
										{
											stopMergingZ = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingZ = true;
											continue;
										}
									}

									if (stopMergingZ == true)
									{
										stopMerging = true;
										continue;
									}

									increaseY++;
									l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z)] |= MergedSide_X_Positive;

									for (int zAdd = 1; zAdd <= increaseZ; zAdd++)
									{
										unsigned int z1 = z + zAdd;
										l_merged[x + pMatrix->m_sizeX * (y1 + pMatrix->m_sizeY * z1)] |= MergedSide_X_Positive;
									}
								}

								verticesBuffer[verticesCounter + 0].x = x + 0.5f;
								verticesBuffer[verticesCounter + 0].y = y + -0.5f;
								verticesBuffer[verticesCounter + 0].z = z + -0.5f;
								verticesBuffer[verticesCounter + 0].r = r;
								verticesBuffer[verticesCounter + 0].g = g;
								verticesBuffer[verticesCounter + 0].b = b;
								verticesBuffer[verticesCounter + 0].a = 1.0f;
								verticesBuffer[verticesCounter + 0].nx = 1.0f;
								verticesBuffer[verticesCounter + 0].ny = 0.0f;
								verticesBuffer[verticesCounter + 0].nz = 0.0f;

								verticesBuffer[verticesCounter + 1].x = x + 0.5f;
								verticesBuffer[verticesCounter + 1].y = y + -0.5f;
								verticesBuffer[verticesCounter + 1].z = z + 0.5f + (1.0f*increaseZ);
								verticesBuffer[verticesCounter + 1].r = r;
								verticesBuffer[verticesCounter + 1].g = g;
								verticesBuffer[verticesCounter + 1].b = b;
								verticesBuffer[verticesCounter + 1].a = 1.0f;
								verticesBuffer[verticesCounter + 1].nx = 1.0f;
								verticesBuffer[verticesCounter + 1].ny = 0.0f;
								verticesBuffer[verticesCounter + 1].nz = 0.0f;

								verticesBuffer[verticesCounter + 2].x = x + 0.5f;
								verticesBuffer[verticesCounter + 2].y = y + 0.5f + (1.0f*increaseY);
								verticesBuffer[verticesCounter + 2].z = z + -0.5f;
								verticesBuffer[verticesCounter + 2].r = r;
								verticesBuffer[verticesCounter + 2].g = g;
								verticesBuffer[verticesCounter + 2].b = b;
								verticesBuffer[verticesCounter + 2].a = 1.0f;
								verticesBuffer[verticesCounter + 2].nx = 1.0f;
								verticesBuffer[verticesCounter + 2].ny = 0.0f;
								verticesBuffer[verticesCounter + 2].nz = 0.0f;

								verticesBuffer[verticesCounter + 3].x = x + 0.5f;
								verticesBuffer[verticesCounter + 3].y = y + 0.5f + (1.0f*increaseY);
								verticesBuffer[verticesCounter + 3].z = z + 0.5f + (1.0f*increaseZ);
								verticesBuffer[verticesCounter + 3].r = r;
								verticesBuffer[verticesCounter + 3].g = g;
								verticesBuffer[verticesCounter + 3].b = b;
								verticesBuffer[verticesCounter + 3].a = 1.0f;
								verticesBuffer[verticesCounter + 3].nx = 1.0f;
								verticesBuffer[verticesCounter + 3].ny = 0.0f;
								verticesBuffer[verticesCounter + 3].nz = 0.0f;

								indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
								indicesBuffer[indicesCounter + 1] = verticesCounter + 2;
								indicesBuffer[indicesCounter + 2] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 4] = verticesCounter + 2;
								indicesBuffer[indicesCounter + 5] = verticesCounter + 3;

								indicesCounter += 6;
								verticesCounter += 4;
							}
						}

						// Top
						if ((mask & 8) == 8)
						{
							if ((merged & MergedSide_Y_Positive) != MergedSide_Y_Positive)
							{
								bool stopMerging = false;
								int increaseZ = 0;
								for (unsigned int z1 = z + 1; z1 < pMatrix->m_sizeZ && stopMerging == false; z1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

									if ((merged1 & MergedSide_Y_Positive) == MergedSide_Y_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 8) != 8)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseZ++;
									l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_Y_Positive;
								}

								stopMerging = false;
								int increaseX = 0;
								for (unsigned int x1 = x + 1; x1 < pMatrix->m_sizeX && stopMerging == false; x1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Y_Positive) == MergedSide_Y_Positive)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 8) != 8)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingZ = false;
									for (int zAdd = 1; zAdd <= increaseZ && stopMergingZ == false; zAdd++)
									{
										unsigned int z1 = z + zAdd;

										unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
										int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

										if ((merged1 & MergedSide_Y_Positive) == MergedSide_Y_Positive)
										{
											stopMergingZ = true;
											continue;
										}
										if ((mask1 & 8) != 8)
										{
											stopMergingZ = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingZ = true;
											continue;
										}
									}

									if (stopMergingZ == true)
									{
										stopMerging = true;
										continue;
									}

									increaseX++;
									l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)] |= MergedSide_Y_Positive;

									for (int zAdd = 1; zAdd <= increaseZ; zAdd++)
									{
										unsigned int z1 = z + zAdd;
										l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_Y_Positive;
									}
								}

								verticesBuffer[verticesCounter + 0].x = x + -0.5f;
								verticesBuffer[verticesCounter + 0].y = y + 0.5f;
								verticesBuffer[verticesCounter + 0].z = z + 0.5f + (1.0f*increaseZ);
								verticesBuffer[verticesCounter + 0].r = r;
								verticesBuffer[verticesCounter + 0].g = g;
								verticesBuffer[verticesCounter + 0].b = b;
								verticesBuffer[verticesCounter + 0].a = 1.0f;
								verticesBuffer[verticesCounter + 0].nx = 0.0f;
								verticesBuffer[verticesCounter + 0].ny = 1.0f;
								verticesBuffer[verticesCounter + 0].nz = 0.0f;

								verticesBuffer[verticesCounter + 1].x = x + 0.5f + (1.0f*increaseX);
								verticesBuffer[verticesCounter + 1].y = y + 0.5f;
								verticesBuffer[verticesCounter + 1].z = z + 0.5f + (1.0f*increaseZ);
								verticesBuffer[verticesCounter + 1].r = r;
								verticesBuffer[verticesCounter + 1].g = g;
								verticesBuffer[verticesCounter + 1].b = b;
								verticesBuffer[verticesCounter + 1].a = 1.0f;
								verticesBuffer[verticesCounter + 1].nx = 0.0f;
								verticesBuffer[verticesCounter + 1].ny = 1.0f;
								verticesBuffer[verticesCounter + 1].nz = 0.0f;

								verticesBuffer[verticesCounter + 2].x = x + -0.5f;
								verticesBuffer[verticesCounter + 2].y = y + 0.5f;
								verticesBuffer[verticesCounter + 2].z = z + -0.5f;
								verticesBuffer[verticesCounter + 2].r = r;
								verticesBuffer[verticesCounter + 2].g = g;
								verticesBuffer[verticesCounter + 2].b = b;
								verticesBuffer[verticesCounter + 2].a = 1.0f;
								verticesBuffer[verticesCounter + 2].nx = 0.0f;
								verticesBuffer[verticesCounter + 2].ny = 1.0f;
								verticesBuffer[verticesCounter + 2].nz = 0.0f;

								verticesBuffer[verticesCounter + 3].x = x + 0.5f + (1.0f*increaseX);
								verticesBuffer[verticesCounter + 3].y = y + 0.5f;
								verticesBuffer[verticesCounter + 3].z = z + -0.5f;
								verticesBuffer[verticesCounter + 3].r = r;
								verticesBuffer[verticesCounter + 3].g = g;
								verticesBuffer[verticesCounter + 3].b = b;
								verticesBuffer[verticesCounter + 3].a = 1.0f;
								verticesBuffer[verticesCounter + 3].nx = 0.0f;
								verticesBuffer[verticesCounter + 3].ny = 1.0f;
								verticesBuffer[verticesCounter + 3].nz = 0.0f;

								indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
								indicesBuffer[indicesCounter + 1] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 2] = verticesCounter + 2;
								indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 4] = verticesCounter + 3;
								indicesBuffer[indicesCounter + 5] = verticesCounter + 2;

								indicesCounter += 6;
								verticesCounter += 4;
							}
						}

						// Bottom
						if ((mask & 16) == 16)
						{
							if ((merged & MergedSide_Y_Negative) != MergedSide_Y_Negative)
							{
								bool stopMerging = false;
								int increaseZ = 0;
								for (unsigned int z1 = z + 1; z1 < pMatrix->m_sizeZ && stopMerging == false; z1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
									int merged1 = l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

									if ((merged1 & MergedSide_Y_Negative) == MergedSide_Y_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 16) != 16)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									increaseZ++;
									l_merged[x + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_Y_Negative;
								}

								stopMerging = false;
								int increaseX = 0;
								for (unsigned int x1 = x + 1; x1 < pMatrix->m_sizeX && stopMerging == false; x1++)
								{
									unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];
									int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)];

									if ((merged1 & MergedSide_Y_Negative) == MergedSide_Y_Negative)
									{
										stopMerging = true;
										continue;
									}
									if ((mask1 & 16) != 16)
									{
										stopMerging = true;
										continue;
									}
									if (colour1 != colour)
									{
										stopMerging = true;
										continue;
									}

									bool stopMergingZ = false;
									for (int zAdd = 1; zAdd <= increaseZ && stopMergingZ == false; zAdd++)
									{
										unsigned int z1 = z + zAdd;

										unsigned int colour1 = pMatrix->m_pColour[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
										unsigned int mask1 = pMatrix->m_pVisibilityMask[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];
										int merged1 = l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)];

										if ((merged1 & MergedSide_Y_Negative) == MergedSide_Y_Negative)
										{
											stopMergingZ = true;
											continue;
										}
										if ((mask1 & 16) != 16)
										{
											stopMergingZ = true;
											continue;
										}
										if (colour1 != colour)
										{
											stopMergingZ = true;
											continue;
										}
									}

									if (stopMergingZ == true)
									{
										stopMerging = true;
										continue;
									}

									increaseX++;
									l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z)] |= MergedSide_Y_Negative;

									for (int zAdd = 1; zAdd <= increaseZ; zAdd++)
									{
										unsigned int z1 = z + zAdd;
										l_merged[x1 + pMatrix->m_sizeX * (y + pMatrix->m_sizeY * z1)] |= MergedSide_Y_Negative;
									}
								}

								verticesBuffer[verticesCounter + 0].x = x + -0.5f;
								verticesBuffer[verticesCounter + 0].y = y + -0.5f;
								verticesBuffer[verticesCounter + 0].z = z + 0.5f + (1.0f*increaseZ);
								verticesBuffer[verticesCounter + 0].r = r;
								verticesBuffer[verticesCounter + 0].g = g;
								verticesBuffer[verticesCounter + 0].b = b;
								verticesBuffer[verticesCounter + 0].a = 1.0f;
								verticesBuffer[verticesCounter + 0].nx = 0.0f;
								verticesBuffer[verticesCounter + 0].ny = -1.0f;
								verticesBuffer[verticesCounter + 0].nz = 0.0f;

								verticesBuffer[verticesCounter + 1].x = x + 0.5f + (1.0f*increaseX);
								verticesBuffer[verticesCounter + 1].y = y + -0.5f;
								verticesBuffer[verticesCounter + 1].z = z + 0.5f + (1.0f*increaseZ);
								verticesBuffer[verticesCounter + 1].r = r;
								verticesBuffer[verticesCounter + 1].g = g;
								verticesBuffer[verticesCounter + 1].b = b;
								verticesBuffer[verticesCounter + 1].a = 1.0f;
								verticesBuffer[verticesCounter + 1].nx = 0.0f;
								verticesBuffer[verticesCounter + 1].ny = -1.0f;
								verticesBuffer[verticesCounter + 1].nz = 0.0f;

								verticesBuffer[verticesCounter + 2].x = x + -0.5f;
								verticesBuffer[verticesCounter + 2].y = y + -0.5f;
								verticesBuffer[verticesCounter + 2].z = z + -0.5f;
								verticesBuffer[verticesCounter + 2].r = r;
								verticesBuffer[verticesCounter + 2].g = g;
								verticesBuffer[verticesCounter + 2].b = b;
								verticesBuffer[verticesCounter + 2].a = 1.0f;
								verticesBuffer[verticesCounter + 2].nx = 0.0f;
								verticesBuffer[verticesCounter + 2].ny = -1.0f;
								verticesBuffer[verticesCounter + 2].nz = 0.0f;

								verticesBuffer[verticesCounter + 3].x = x + 0.5f + (1.0f*increaseX);
								verticesBuffer[verticesCounter + 3].y = y + -0.5f;
								verticesBuffer[verticesCounter + 3].z = z + -0.5f;
								verticesBuffer[verticesCounter + 3].r = r;
								verticesBuffer[verticesCounter + 3].g = g;
								verticesBuffer[verticesCounter + 3].b = b;
								verticesBuffer[verticesCounter + 3].a = 1.0f;
								verticesBuffer[verticesCounter + 3].nx = 0.0f;
								verticesBuffer[verticesCounter + 3].ny = -1.0f;
								verticesBuffer[verticesCounter + 3].nz = 0.0f;

								indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
								indicesBuffer[indicesCounter + 1] = verticesCounter + 2;
								indicesBuffer[indicesCounter + 2] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
								indicesBuffer[indicesCounter + 4] = verticesCounter + 2;
								indicesBuffer[indicesCounter + 5] = verticesCounter + 3;

								indicesCounter += 6;
								verticesCounter += 4;
							}
						}
					}
				}
			}
		}
		else
		{
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

						// Back
						if (m_createInnerFaces == true || (mask & 32) == 32)
						{
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

							indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
							indicesBuffer[indicesCounter + 1] = verticesCounter + 2;
							indicesBuffer[indicesCounter + 2] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 4] = verticesCounter + 2;
							indicesBuffer[indicesCounter + 5] = verticesCounter + 3;

							indicesCounter += 6;
							verticesCounter += 4;
						}

						// Front
						if (m_createInnerFaces == true || (mask & 64) == 64)
						{
							verticesBuffer[verticesCounter + 0].x = x + -0.5f;
							verticesBuffer[verticesCounter + 0].y = y + -0.5f;
							verticesBuffer[verticesCounter + 0].z = z + 0.5f;
							verticesBuffer[verticesCounter + 0].r = r;
							verticesBuffer[verticesCounter + 0].g = g;
							verticesBuffer[verticesCounter + 0].b = b;
							verticesBuffer[verticesCounter + 0].a = 1.0f;
							verticesBuffer[verticesCounter + 0].nx = 0.0f;
							verticesBuffer[verticesCounter + 0].ny = 0.0f;
							verticesBuffer[verticesCounter + 0].nz = 1.0f;

							verticesBuffer[verticesCounter + 1].x = x + 0.5f;
							verticesBuffer[verticesCounter + 1].y = y + -0.5f;
							verticesBuffer[verticesCounter + 1].z = z + 0.5f;
							verticesBuffer[verticesCounter + 1].r = r;
							verticesBuffer[verticesCounter + 1].g = g;
							verticesBuffer[verticesCounter + 1].b = b;
							verticesBuffer[verticesCounter + 1].a = 1.0f;
							verticesBuffer[verticesCounter + 1].nx = 0.0f;
							verticesBuffer[verticesCounter + 1].ny = 0.0f;
							verticesBuffer[verticesCounter + 1].nz = 1.0f;

							verticesBuffer[verticesCounter + 2].x = x + -0.5f;
							verticesBuffer[verticesCounter + 2].y = y + 0.5f;
							verticesBuffer[verticesCounter + 2].z = z + 0.5f;
							verticesBuffer[verticesCounter + 2].r = r;
							verticesBuffer[verticesCounter + 2].g = g;
							verticesBuffer[verticesCounter + 2].b = b;
							verticesBuffer[verticesCounter + 2].a = 1.0f;
							verticesBuffer[verticesCounter + 2].nx = 0.0f;
							verticesBuffer[verticesCounter + 2].ny = 0.0f;
							verticesBuffer[verticesCounter + 2].nz = 1.0f;

							verticesBuffer[verticesCounter + 3].x = x + 0.5f;
							verticesBuffer[verticesCounter + 3].y = y + 0.5f;
							verticesBuffer[verticesCounter + 3].z = z + 0.5f;
							verticesBuffer[verticesCounter + 3].r = r;
							verticesBuffer[verticesCounter + 3].g = g;
							verticesBuffer[verticesCounter + 3].b = b;
							verticesBuffer[verticesCounter + 3].a = 1.0f;
							verticesBuffer[verticesCounter + 3].nx = 0.0f;
							verticesBuffer[verticesCounter + 3].ny = 0.0f;
							verticesBuffer[verticesCounter + 3].nz = 1.0f;

							indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
							indicesBuffer[indicesCounter + 1] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 2] = verticesCounter + 2;
							indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 4] = verticesCounter + 3;
							indicesBuffer[indicesCounter + 5] = verticesCounter + 2;

							indicesCounter += 6;
							verticesCounter += 4;
						}

						// Left
						if (m_createInnerFaces == true || (mask & 4) == 4)
						{
							verticesBuffer[verticesCounter + 0].x = x + -0.5f;
							verticesBuffer[verticesCounter + 0].y = y + -0.5f;
							verticesBuffer[verticesCounter + 0].z = z + -0.5f;
							verticesBuffer[verticesCounter + 0].r = r;
							verticesBuffer[verticesCounter + 0].g = g;
							verticesBuffer[verticesCounter + 0].b = b;
							verticesBuffer[verticesCounter + 0].a = 1.0f;
							verticesBuffer[verticesCounter + 0].nx = -1.0f;
							verticesBuffer[verticesCounter + 0].ny = 0.0f;
							verticesBuffer[verticesCounter + 0].nz = 0.0f;

							verticesBuffer[verticesCounter + 1].x = x + -0.5f;
							verticesBuffer[verticesCounter + 1].y = y + -0.5f;
							verticesBuffer[verticesCounter + 1].z = z + 0.5f;
							verticesBuffer[verticesCounter + 1].r = r;
							verticesBuffer[verticesCounter + 1].g = g;
							verticesBuffer[verticesCounter + 1].b = b;
							verticesBuffer[verticesCounter + 1].a = 1.0f;
							verticesBuffer[verticesCounter + 1].nx = -1.0f;
							verticesBuffer[verticesCounter + 1].ny = 0.0f;
							verticesBuffer[verticesCounter + 1].nz = 0.0f;

							verticesBuffer[verticesCounter + 2].x = x + -0.5f;
							verticesBuffer[verticesCounter + 2].y = y + 0.5f;
							verticesBuffer[verticesCounter + 2].z = z + -0.5f;
							verticesBuffer[verticesCounter + 2].r = r;
							verticesBuffer[verticesCounter + 2].g = g;
							verticesBuffer[verticesCounter + 2].b = b;
							verticesBuffer[verticesCounter + 2].a = 1.0f;
							verticesBuffer[verticesCounter + 2].nx = -1.0f;
							verticesBuffer[verticesCounter + 2].ny = 0.0f;
							verticesBuffer[verticesCounter + 2].nz = 0.0f;

							verticesBuffer[verticesCounter + 3].x = x + -0.5f;
							verticesBuffer[verticesCounter + 3].y = y + 0.5f;
							verticesBuffer[verticesCounter + 3].z = z + 0.5f;
							verticesBuffer[verticesCounter + 3].r = r;
							verticesBuffer[verticesCounter + 3].g = g;
							verticesBuffer[verticesCounter + 3].b = b;
							verticesBuffer[verticesCounter + 3].a = 1.0f;
							verticesBuffer[verticesCounter + 3].nx = -1.0f;
							verticesBuffer[verticesCounter + 3].ny = 0.0f;
							verticesBuffer[verticesCounter + 3].nz = 0.0f;

							indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
							indicesBuffer[indicesCounter + 1] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 2] = verticesCounter + 2;
							indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 4] = verticesCounter + 3;
							indicesBuffer[indicesCounter + 5] = verticesCounter + 2;

							indicesCounter += 6;
							verticesCounter += 4;
						}

						// Right
						if (m_createInnerFaces == true || (mask & 2) == 2)
						{
							verticesBuffer[verticesCounter + 0].x = x + 0.5f;
							verticesBuffer[verticesCounter + 0].y = y + -0.5f;
							verticesBuffer[verticesCounter + 0].z = z + -0.5f;
							verticesBuffer[verticesCounter + 0].r = r;
							verticesBuffer[verticesCounter + 0].g = g;
							verticesBuffer[verticesCounter + 0].b = b;
							verticesBuffer[verticesCounter + 0].a = 1.0f;
							verticesBuffer[verticesCounter + 0].nx = 1.0f;
							verticesBuffer[verticesCounter + 0].ny = 0.0f;
							verticesBuffer[verticesCounter + 0].nz = 0.0f;

							verticesBuffer[verticesCounter + 1].x = x + 0.5f;
							verticesBuffer[verticesCounter + 1].y = y + -0.5f;
							verticesBuffer[verticesCounter + 1].z = z + 0.5f;
							verticesBuffer[verticesCounter + 1].r = r;
							verticesBuffer[verticesCounter + 1].g = g;
							verticesBuffer[verticesCounter + 1].b = b;
							verticesBuffer[verticesCounter + 1].a = 1.0f;
							verticesBuffer[verticesCounter + 1].nx = 1.0f;
							verticesBuffer[verticesCounter + 1].ny = 0.0f;
							verticesBuffer[verticesCounter + 1].nz = 0.0f;

							verticesBuffer[verticesCounter + 2].x = x + 0.5f;
							verticesBuffer[verticesCounter + 2].y = y + 0.5f;
							verticesBuffer[verticesCounter + 2].z = z + -0.5f;
							verticesBuffer[verticesCounter + 2].r = r;
							verticesBuffer[verticesCounter + 2].g = g;
							verticesBuffer[verticesCounter + 2].b = b;
							verticesBuffer[verticesCounter + 2].a = 1.0f;
							verticesBuffer[verticesCounter + 2].nx = 1.0f;
							verticesBuffer[verticesCounter + 2].ny = 0.0f;
							verticesBuffer[verticesCounter + 2].nz = 0.0f;

							verticesBuffer[verticesCounter + 3].x = x + 0.5f;
							verticesBuffer[verticesCounter + 3].y = y + 0.5f;
							verticesBuffer[verticesCounter + 3].z = z + 0.5f;
							verticesBuffer[verticesCounter + 3].r = r;
							verticesBuffer[verticesCounter + 3].g = g;
							verticesBuffer[verticesCounter + 3].b = b;
							verticesBuffer[verticesCounter + 3].a = 1.0f;
							verticesBuffer[verticesCounter + 3].nx = 1.0f;
							verticesBuffer[verticesCounter + 3].ny = 0.0f;
							verticesBuffer[verticesCounter + 3].nz = 0.0f;

							indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
							indicesBuffer[indicesCounter + 1] = verticesCounter + 2;
							indicesBuffer[indicesCounter + 2] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 4] = verticesCounter + 2;
							indicesBuffer[indicesCounter + 5] = verticesCounter + 3;

							indicesCounter += 6;
							verticesCounter += 4;
						}

						// Top
						if (m_createInnerFaces == true || (mask & 8) == 8)
						{
							verticesBuffer[verticesCounter + 0].x = x + -0.5f;
							verticesBuffer[verticesCounter + 0].y = y + 0.5f;
							verticesBuffer[verticesCounter + 0].z = z + 0.5f;
							verticesBuffer[verticesCounter + 0].r = r;
							verticesBuffer[verticesCounter + 0].g = g;
							verticesBuffer[verticesCounter + 0].b = b;
							verticesBuffer[verticesCounter + 0].a = 1.0f;
							verticesBuffer[verticesCounter + 0].nx = 0.0f;
							verticesBuffer[verticesCounter + 0].ny = 1.0f;
							verticesBuffer[verticesCounter + 0].nz = 0.0f;

							verticesBuffer[verticesCounter + 1].x = x + 0.5f;
							verticesBuffer[verticesCounter + 1].y = y + 0.5f;
							verticesBuffer[verticesCounter + 1].z = z + 0.5f;
							verticesBuffer[verticesCounter + 1].r = r;
							verticesBuffer[verticesCounter + 1].g = g;
							verticesBuffer[verticesCounter + 1].b = b;
							verticesBuffer[verticesCounter + 1].a = 1.0f;
							verticesBuffer[verticesCounter + 1].nx = 0.0f;
							verticesBuffer[verticesCounter + 1].ny = 1.0f;
							verticesBuffer[verticesCounter + 1].nz = 0.0f;

							verticesBuffer[verticesCounter + 2].x = x + -0.5f;
							verticesBuffer[verticesCounter + 2].y = y + 0.5f;
							verticesBuffer[verticesCounter + 2].z = z + -0.5f;
							verticesBuffer[verticesCounter + 2].r = r;
							verticesBuffer[verticesCounter + 2].g = g;
							verticesBuffer[verticesCounter + 2].b = b;
							verticesBuffer[verticesCounter + 2].a = 1.0f;
							verticesBuffer[verticesCounter + 2].nx = 0.0f;
							verticesBuffer[verticesCounter + 2].ny = 1.0f;
							verticesBuffer[verticesCounter + 2].nz = 0.0f;

							verticesBuffer[verticesCounter + 3].x = x + 0.5f;
							verticesBuffer[verticesCounter + 3].y = y + 0.5f;
							verticesBuffer[verticesCounter + 3].z = z + -0.5f;
							verticesBuffer[verticesCounter + 3].r = r;
							verticesBuffer[verticesCounter + 3].g = g;
							verticesBuffer[verticesCounter + 3].b = b;
							verticesBuffer[verticesCounter + 3].a = 1.0f;
							verticesBuffer[verticesCounter + 3].nx = 0.0f;
							verticesBuffer[verticesCounter + 3].ny = 1.0f;
							verticesBuffer[verticesCounter + 3].nz = 0.0f;

							indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
							indicesBuffer[indicesCounter + 1] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 2] = verticesCounter + 2;
							indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 4] = verticesCounter + 3;
							indicesBuffer[indicesCounter + 5] = verticesCounter + 2;

							indicesCounter += 6;
							verticesCounter += 4;
						}

						// Bottom
						if (m_createInnerFaces == true || (mask & 16) == 16)
						{
							verticesBuffer[verticesCounter + 0].x = x + -0.5f;
							verticesBuffer[verticesCounter + 0].y = y + -0.5f;
							verticesBuffer[verticesCounter + 0].z = z + 0.5f;
							verticesBuffer[verticesCounter + 0].r = r;
							verticesBuffer[verticesCounter + 0].g = g;
							verticesBuffer[verticesCounter + 0].b = b;
							verticesBuffer[verticesCounter + 0].a = 1.0f;
							verticesBuffer[verticesCounter + 0].nx = 0.0f;
							verticesBuffer[verticesCounter + 0].ny = -1.0f;
							verticesBuffer[verticesCounter + 0].nz = 0.0f;

							verticesBuffer[verticesCounter + 1].x = x + 0.5f;
							verticesBuffer[verticesCounter + 1].y = y + -0.5f;
							verticesBuffer[verticesCounter + 1].z = z + 0.5f;
							verticesBuffer[verticesCounter + 1].r = r;
							verticesBuffer[verticesCounter + 1].g = g;
							verticesBuffer[verticesCounter + 1].b = b;
							verticesBuffer[verticesCounter + 1].a = 1.0f;
							verticesBuffer[verticesCounter + 1].nx = 0.0f;
							verticesBuffer[verticesCounter + 1].ny = -1.0f;
							verticesBuffer[verticesCounter + 1].nz = 0.0f;

							verticesBuffer[verticesCounter + 2].x = x + -0.5f;
							verticesBuffer[verticesCounter + 2].y = y + -0.5f;
							verticesBuffer[verticesCounter + 2].z = z + -0.5f;
							verticesBuffer[verticesCounter + 2].r = r;
							verticesBuffer[verticesCounter + 2].g = g;
							verticesBuffer[verticesCounter + 2].b = b;
							verticesBuffer[verticesCounter + 2].a = 1.0f;
							verticesBuffer[verticesCounter + 2].nx = 0.0f;
							verticesBuffer[verticesCounter + 2].ny = -1.0f;
							verticesBuffer[verticesCounter + 2].nz = 0.0f;

							verticesBuffer[verticesCounter + 3].x = x + 0.5f;
							verticesBuffer[verticesCounter + 3].y = y + -0.5f;
							verticesBuffer[verticesCounter + 3].z = z + -0.5f;
							verticesBuffer[verticesCounter + 3].r = r;
							verticesBuffer[verticesCounter + 3].g = g;
							verticesBuffer[verticesCounter + 3].b = b;
							verticesBuffer[verticesCounter + 3].a = 1.0f;
							verticesBuffer[verticesCounter + 3].nx = 0.0f;
							verticesBuffer[verticesCounter + 3].ny = -1.0f;
							verticesBuffer[verticesCounter + 3].nz = 0.0f;

							indicesBuffer[indicesCounter + 0] = verticesCounter + 0;
							indicesBuffer[indicesCounter + 1] = verticesCounter + 2;
							indicesBuffer[indicesCounter + 2] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 3] = verticesCounter + 1;
							indicesBuffer[indicesCounter + 4] = verticesCounter + 2;
							indicesBuffer[indicesCounter + 5] = verticesCounter + 3;

							indicesCounter += 6;
							verticesCounter += 4;
						}
					}
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

// Modifiers
void QBT::SetMaterialAmbient(Colour ambient)
{
	for (int i = 0; i < (int)m_vpQBTMatrices.size(); i++)
	{
		m_vpQBTMatrices[i]->m_pMaterial->m_ambient = ambient;
	}
}

void QBT::SetMaterialDiffuse(Colour diffuse)
{
	for (int i = 0; i < (int)m_vpQBTMatrices.size(); i++)
	{
		m_vpQBTMatrices[i]->m_pMaterial->m_diffuse = diffuse;
	}
}

void QBT::SetMaterialSpecular(Colour specular)
{
	for (int i = 0; i < (int)m_vpQBTMatrices.size(); i++)
	{
		m_vpQBTMatrices[i]->m_pMaterial->m_specular = specular;
	}
}

void QBT::SetMaterialEmission(Colour emission)
{
	for (int i = 0; i < (int)m_vpQBTMatrices.size(); i++)
	{
		m_vpQBTMatrices[i]->m_pMaterial->m_emission = emission;
	}
}

void QBT::SetMaterialShininess(float shininess)
{
	for (int i = 0; i < (int)m_vpQBTMatrices.size(); i++)
	{
		m_vpQBTMatrices[i]->m_pMaterial->m_shininess = shininess;
	}
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

void QBT::SetUseLighting(bool lighting)
{
	m_useLighting = lighting;
}

void QBT::SetBoundingBoxRendering(bool boundingBox)
{
	m_boundingBox = boundingBox;
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
void QBT::Render(Camera* pCamera, Light* pLight)
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

	GLint viewPosLoc = glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "viewPos");
	glUniform3f(viewPosLoc, pCamera->GetPosition().x, pCamera->GetPosition().y, pCamera->GetPosition().z);

	// Set light properties
	glUniform3f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "light.position"), pLight->m_position.x, pLight->m_position.y, pLight->m_position.z);
	glUniform3f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "light.ambient"), pLight->m_ambient.GetRed(), pLight->m_ambient.GetGreen(), pLight->m_ambient.GetBlue());
	glUniform3f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "light.diffuse"), pLight->m_diffuse.GetRed(), pLight->m_diffuse.GetGreen(), pLight->m_diffuse.GetBlue());
	glUniform3f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "light.specular"), pLight->m_specular.GetRed(), pLight->m_specular.GetGreen(), pLight->m_specular.GetBlue());
	glUniform1f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "light.constant"), pLight->m_constantAttenuation);
	glUniform1f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "light.linear"), pLight->m_linearAttenuation);
	glUniform1f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "light.quadratic"), pLight->m_quadraticAttenuation);

	glUniform1i(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "useLighting"), m_useLighting);

	for (unsigned int matrixIndex = 0; matrixIndex < m_vpQBTMatrices.size(); matrixIndex++)
	{
		QBTMatrix* pMatrix = m_vpQBTMatrices[matrixIndex];

		// Set material properties
		glUniform3f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "material.ambient"), pMatrix->m_pMaterial->m_ambient.GetRed(), pMatrix->m_pMaterial->m_ambient.GetGreen(), pMatrix->m_pMaterial->m_ambient.GetBlue());
		glUniform3f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "material.diffuse"), pMatrix->m_pMaterial->m_diffuse.GetRed(), pMatrix->m_pMaterial->m_diffuse.GetGreen(), pMatrix->m_pMaterial->m_diffuse.GetBlue());
		glUniform3f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "material.specular"), pMatrix->m_pMaterial->m_specular.GetRed(), pMatrix->m_pMaterial->m_specular.GetGreen(), pMatrix->m_pMaterial->m_specular.GetBlue());
		glUniform1f(glGetUniformLocation(m_pPositionColorNormalShader->GetShader(), "material.shininess"), pMatrix->m_pMaterial->m_shininess);

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
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

		glBindVertexArray(pMatrix->m_VAO);

		mat4 model;
		model = translate(model, vec3(pMatrix->m_positionX, pMatrix->m_positionY, pMatrix->m_positionZ));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

		glDrawElements(GL_TRIANGLES, pMatrix->m_numIndices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (m_boundingBox)
	{
		RenderBoundingBox(pCamera, pLight);
	}
}

void QBT::RenderBoundingBox(Camera* pCamera, Light* pLight)
{
	for (unsigned int matrixIndex = 0; matrixIndex < m_vpQBTMatrices.size(); matrixIndex++)
	{
		QBTMatrix* pMatrix = m_vpQBTMatrices[matrixIndex];

		float x1 = 0.0f;
		float x2 = (float)pMatrix->m_sizeX;
		float y1 = 0.0f;
		float y2 = (float)pMatrix->m_sizeY;
		float z1 = 0.0f;
		float z2 = (float)pMatrix->m_sizeZ;
		vec3 center(pMatrix->m_positionX - 0.5f, pMatrix->m_positionY - 0.5f, pMatrix->m_positionZ - 0.5f);
		vec3 pivot(pMatrix->m_pivotX, pMatrix->m_pivotY, pMatrix->m_pivotZ);

		m_pRenderer->DrawLine(center + vec3(x1, y1, z1), center + vec3(x2, y1, z1), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));
		m_pRenderer->DrawLine(center + vec3(x1, y2, z1), center + vec3(x2, y2, z1), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));
		m_pRenderer->DrawLine(center + vec3(x1, y1, z2), center + vec3(x2, y1, z2), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));
		m_pRenderer->DrawLine(center + vec3(x1, y2, z2), center + vec3(x2, y2, z2), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));

		m_pRenderer->DrawLine(center + vec3(x1, y1, z1), center + vec3(x1, y2, z1), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));
		m_pRenderer->DrawLine(center + vec3(x2, y1, z1), center + vec3(x2, y2, z1), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));
		m_pRenderer->DrawLine(center + vec3(x1, y1, z2), center + vec3(x1, y2, z2), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));
		m_pRenderer->DrawLine(center + vec3(x2, y1, z2), center + vec3(x2, y2, z2), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));

		m_pRenderer->DrawLine(center + vec3(x1, y1, z1), center + vec3(x1, y1, z2), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));
		m_pRenderer->DrawLine(center + vec3(x2, y1, z1), center + vec3(x2, y1, z2), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));
		m_pRenderer->DrawLine(center + vec3(x1, y2, z1), center + vec3(x1, y2, z2), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));
		m_pRenderer->DrawLine(center + vec3(x2, y2, z1), center + vec3(x2, y2, z2), Colour(1.0f, 1.0f, 0.0f), Colour(1.0f, 1.0f, 0.0f));
	}
}