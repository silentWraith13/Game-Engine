#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/vertexUtils.hpp"
#include <vector>
#include <chrono>
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Vertex
{
	int v;
	int t;
	int n;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Face
{
	std::vector<Vertex> vertexes;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void OBJLoader::ImportFromOBJFile(const std::string& filename, Mat44 fixupTransform, std::vector<Vertex_PCUTBN>& verts, std::vector< unsigned int>& indexes)
{
	std::vector<Vec3> vertexPositions;
	vertexPositions.reserve(10000);
	std::vector<Vec3> vertexNormals;
	vertexNormals.reserve(10000);
	std::vector<Vec2> vertexUVs;
	vertexUVs.reserve(10000);
	std::vector<Face> faces;
	faces.reserve(10000);
	std::string fileContents;
	FileReadToString(fileContents, filename);
	verts.clear();
	indexes.clear();
	
	//Split lines into separate lines based on /n or /r
	std::vector<std::string> lines;
	lines = SplitStringOnDelimiter(fileContents, '\n');
	for (int i = 0; i < lines.size(); i++)
	{
		if (lines[i].find('\r') != std::string::npos)
		{
			lines[i].erase(remove(lines[i].begin(), lines[i].end(), '\r'));
		}
	}

	//peek at first two characters to determine elements
	for (int i = 0; i < lines.size(); i++)
	{
		//split lines based on element exlcuding extra spaces
		if (lines[i].substr(0, 2) == "v ")
		{
			std::vector<std::string> adjustedLines;
			adjustedLines = SplitStringOnDelimiterExcludeConsecutives(lines[i], ' ');

			//construct intermediate vec3 to hold data from parsed obj lines
			Vec3 geometricVertexData;
			geometricVertexData.x = static_cast<float>(atof(adjustedLines[1].c_str()));
			geometricVertexData.y = static_cast<float>(atof(adjustedLines[2].c_str()));
			geometricVertexData.z = static_cast<float>(atof(adjustedLines[3].c_str()));
			vertexPositions.push_back(geometricVertexData);
		}
		else if (lines[i].substr(0, 2) == "f ")
		{
			std::vector<std::string> faceValuesElement;
			faceValuesElement = SplitStringOnDelimiterExcludeConsecutives(lines[i], ' ');
			
			Face face;

			// For each vertex in the face definition (starting from index 1 because index 0 is "f")
			for (int j = 1; j < faceValuesElement.size(); j++)
			{
				Vertex vertex;

				// Split the vertex definition into its components
				std::vector<std::string> vertexComponents = SplitStringOnDelimiter(faceValuesElement[j], '/');

				// If only the vertex position index is provided
				if (vertexComponents.size() == 1)
				{
					vertex.v = atoi(vertexComponents[0].c_str());
					vertex.t = -1;
					vertex.n = -1;
				}
				// If vertex position and texture coordinates are provided (f v/vt)
				else if (vertexComponents.size() == 2)
				{
					vertex.v = atoi(vertexComponents[0].c_str());
					vertex.t = atoi(vertexComponents[1].c_str());
					vertex.n = -1;
				}
				// If vertex position and normal indices are provided without texture (f v//vn)
				else if (vertexComponents.size() == 3 && vertexComponents[1].empty())
				{
					vertex.v = atoi(vertexComponents[0].c_str());
					vertex.t = -1;
					vertex.n = atoi(vertexComponents[2].c_str());
				}
				// If all indices (position, texture, and normal) are provided (f v/vt/vn)
				else if (vertexComponents.size() == 3)
				{
					vertex.v = atoi(vertexComponents[0].c_str());
					vertex.t = atoi(vertexComponents[1].c_str());
					vertex.n = atoi(vertexComponents[2].c_str());
				}
				
				else
				{
					vertex.v = -1;
					vertex.t = -1;
					vertex.n = -1;
					
				}
				face.vertexes.push_back(vertex);	
			}
			
			for (int k = 1; k < face.vertexes.size() - 1; k++)
			{
				Face triangle;

				triangle.vertexes.push_back(face.vertexes[0]);
				triangle.vertexes.push_back(face.vertexes[k]);
				triangle.vertexes.push_back(face.vertexes[k + 1]);

				faces.push_back(triangle);
			}
			
		}
		
		else if (lines[i].substr(0, 2) == "vn")
		{
			std::vector<std::string> vertexNormalLines;
			vertexNormalLines = SplitStringOnDelimiterExcludeConsecutives(lines[i], ' ');

			Vec3 normalsData;
			normalsData.x = static_cast<float>(atof(vertexNormalLines[1].c_str()));
			normalsData.y = static_cast<float>(atof(vertexNormalLines[2].c_str()));
			normalsData.z = static_cast<float>(atof(vertexNormalLines[3].c_str()));
			vertexNormals.push_back(normalsData);
		}
		else if (lines[i].substr(0, 2) == "vt")
		{
			std::vector<std::string> texCoordLines;
			texCoordLines = SplitStringOnDelimiterExcludeConsecutives(lines[i], ' ');

			Vec2 texCoordData;
			texCoordData.x = static_cast<float>(atof(texCoordLines[1].c_str()));
			texCoordData.y = static_cast<float>(atof(texCoordLines[2].c_str()));
			vertexUVs.push_back(texCoordData);
		}
	}
	
	//If no faces data, then construct using geometric positions instead
	if (faces.size() == 0)
	{
		//first create all PNCUs based on vertices and also set values for P and U
		for (int i = 0; i < vertexPositions.size(); i++)
		{
			Vertex_PCUTBN temp;
			temp.m_position = vertexPositions[i];
			verts.push_back(temp);
		}
	}
	
	//if there is face data, then construct using the provided indices
	else if (faces.size() != 0)
	{
		for (int i = 0; i < faces.size(); i++)
		{
			for (int j = 0; j < faces[i].vertexes.size(); j++)
			{
				int vertexIndex = faces[i].vertexes[j].v - 1; //needs to be -1 otherwise will be in range of 1-8 instead of 0-7
				Vertex_PCUTBN temp;
				temp.m_position = vertexPositions[vertexIndex];
				verts.push_back(temp);
			}
		}
	}

	//calculate normals if none in the obj
	if (vertexNormals.size() == 0)
	{
		//Set N values of PNCUs after creating by calculating the normals and storing them in proper PNCU elements that have been created
		for (int i = 0; i  < verts.size(); i += 3)
		{
			Vec3 triPosA = verts[i].m_position;
			Vec3 triPosB = verts[i + 1].m_position;
			Vec3 triPosC = verts[i + 2].m_position;

			Vec3 dispAToB = triPosB - triPosA;
			Vec3 dispBToC = triPosC - triPosB;

			verts[i].m_normal = CrossProduct3D(dispAToB, dispBToC).GetNormalized();
			verts[i + 1].m_normal = CrossProduct3D(dispAToB, dispBToC).GetNormalized();
			verts[i + 2].m_normal = CrossProduct3D(dispAToB, dispBToC).GetNormalized();
		}
	}

	//Use provided normals if they exist in the OBJ
	else if (vertexNormals.size() != 0)
	{
		std::vector<int> normalIndices;
		for (int i = 0; i < faces.size(); i++)
		{
			for (int j = 0; j < faces[i].vertexes.size(); j++)
			{
				int normalIndex = faces[i].vertexes[j].n - 1;   //needs to be -1 otherwise will be in range of 1-8 instead of 0-7
				normalIndices.push_back(normalIndex);
			}
		}

		for (int i = 0; i < verts.size(); i++)
		{
			if (normalIndices[i] >= 0 && normalIndices[i] < vertexNormals.size())
			{
				verts[i].m_normal = vertexNormals[normalIndices[i]];
			}

		}
	}

	//Provide default texcoord values if none
	if (vertexUVs.empty())
	{
		for (int i = 0; i < verts.size(); i++)
		{
			verts[i].m_uvTexCoords = Vec2(0.f, 0.f);
		}
	}

	else if (!vertexUVs.empty())
	{
		std::vector<int> texCoordIndices;
		for (int i = 0; i < faces.size(); i++)
		{
			for (int j = 0; j < faces[i].vertexes.size(); j++)
			{
				int textCoordIndex = faces[i].vertexes[j].t - 1;  //needs to be -1 otherwise will be in range of 1-8 instead of 0-7
				texCoordIndices.push_back(textCoordIndex);
			}
		}

		for (int i = 0; i < verts.size(); i++)
		{
			if (texCoordIndices[i] >= 0 && texCoordIndices[i] < vertexUVs.size())
			{
				verts[i].m_uvTexCoords = vertexUVs[texCoordIndices[i]];
			}
		}
	}

	for (int i = 0; i < verts.size(); i++)
	{
		verts[i].m_binormal = Vec3(0.f, 0.f, 0.f);
		verts[i].m_tangent = Vec3(0.f, 0.f, 0.f);
	}

	for (int i = 0; i < verts.size(); i++)
	{
		indexes.push_back(i);
	}
	TransformVertexArray3D(verts, fixupTransform);
	CalculateTangentsAndBiTangents(verts, indexes);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------