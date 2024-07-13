#include "Engine/Renderer/GBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <d3d11.h>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
GBuffer::GBuffer(ID3D11Texture2D* textures[3], ID3D11RenderTargetView* RTV[3], ID3D11ShaderResourceView* SRV[3], ID3D11DepthStencilView* DSV)
{
	for (int i = 0; i < 3; ++i)
	{
		m_textures[i]	= textures[i];
		m_RTV[i]		= RTV[i];
		m_SRV[i]		= SRV[i];
	}
	
	m_DSV				= DSV;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
GBuffer::~GBuffer()
{
	for (int i = 0; i < 3; ++i)
	{
		if (m_textures[i])
		{
			DX_SAFE_RELEASE(m_textures[i]);
		}
			
		if (m_RTV[i])
		{
			DX_SAFE_RELEASE(m_RTV[i]);
		}

		if (m_SRV[i])
		{
			DX_SAFE_RELEASE(m_SRV[i]);
		}
	}
	
	if (m_DSV)
	{
		DX_SAFE_RELEASE(m_DSV);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
