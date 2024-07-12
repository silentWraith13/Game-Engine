#include "Engine/Renderer/UnorderedAccessBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <d3d11.h>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
UnorderedAccessBuffer::UnorderedAccessBuffer(size_t size, unsigned int stride, void* data)
	:m_numElements(size), m_stride(stride), m_data(data)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
UnorderedAccessBuffer::~UnorderedAccessBuffer()
{
	if (m_UAV)
	{
		DX_SAFE_RELEASE(m_UAV);
	}

	if (m_SRV)
	{
		DX_SAFE_RELEASE(m_SRV);
	}

	if (m_buffer)
	{
		DX_SAFE_RELEASE(m_buffer);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
