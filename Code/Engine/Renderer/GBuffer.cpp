#include "Engine/Renderer/GBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <d3d11.h>


//--------------------------------------------------------------------------------------------------------------------------------------------------------
GBuffer::GBuffer(int width, int height)
	:m_width(width), m_height(height)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
GBuffer::~GBuffer()
{
	for (int i = 0; i < 3; ++i)
	{
		if (m_textures[i])	DX_SAFE_RELEASE(m_textures[i]);
		if (m_RTV[i])		DX_SAFE_RELEASE(m_RTV[i]);
		if (m_SRV[i])		DX_SAFE_RELEASE(m_SRV[i]);
	}

	if (m_DSV)				DX_SAFE_RELEASE(m_DSV);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
