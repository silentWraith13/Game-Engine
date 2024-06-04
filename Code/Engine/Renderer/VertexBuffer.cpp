#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/vertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include <d3d11.h>



//--------------------------------------------------------------------------------------------------------------------------------------------------------
// VertexBuffer::VertexBuffer(size_t size, unsigned int stride)
// 	:m_size(size), m_stride(stride)
// {
// 	/*m_stride = sizeof(Vertex_PCU);*/
// }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer(size_t size, unsigned int stride)
	:m_size(size), m_stride(stride)
{
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{
	if (m_buffer)
	{
		DX_SAFE_RELEASE(m_buffer);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned int const VertexBuffer::GetStride() 
{
	return m_stride;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
