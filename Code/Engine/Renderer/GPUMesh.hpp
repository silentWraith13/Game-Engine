#pragma once
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/CPUMesh.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class GPUMesh
{
public:
	GPUMesh();
	~GPUMesh();

	void SetVertexAndIndexBuffers(CPUMesh* cpuMesh);
	void Render();

	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	int m_indexCount = 0;
	CPUMesh* m_cpuMesh = nullptr;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------