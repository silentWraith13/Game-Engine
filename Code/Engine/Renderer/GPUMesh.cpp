#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
GPUMesh::GPUMesh()
{
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
GPUMesh::~GPUMesh()
{
	delete m_indexBuffer;
	m_indexBuffer = nullptr;

	delete m_vertexBuffer;
	m_indexBuffer = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void GPUMesh::SetVertexAndIndexBuffers(CPUMesh* cpuMesh)
{
	g_theRenderer->CopyCPUToGPU(cpuMesh->m_vertices.data(), sizeof(cpuMesh->m_vertices[0]) * (int)cpuMesh->m_vertices.size(), m_vertexBuffer);
	g_theRenderer->CopyCPUToGPU(cpuMesh->m_indices.data(), sizeof(cpuMesh->m_indices[0]) * (int)cpuMesh->m_indices.size(), m_indexBuffer);
	m_indexCount = (int)cpuMesh->m_indices.size();
	m_cpuMesh = cpuMesh;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void GPUMesh::Render()
{
	g_theRenderer->DrawVertexBuffer(m_vertexBuffer, sizeof(Vertex_PCUTBN), m_indexBuffer, m_indexCount);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------