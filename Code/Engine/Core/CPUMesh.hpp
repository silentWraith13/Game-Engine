#pragma once
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <string>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class CPUMesh
{
public:
	CPUMesh() {};
	~CPUMesh();

	CPUMesh(std::vector<Vertex_PCUTBN> vertices, std::vector< unsigned int> indices);

public:
	std::vector<Vertex_PCUTBN> m_vertices;
	std::vector< unsigned int> m_indices;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------