#include "Engine/Renderer/RenderTexture.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")
#include "Engine/Renderer/Renderer.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
RenderTexture::RenderTexture( int width, int height)
	: m_renderTargetTexture(nullptr),
	m_renderTargetView(nullptr),
	m_shaderResourceView(nullptr),
	m_width(width),
	m_height(height)
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
RenderTexture::~RenderTexture()
{
	if (m_shaderResourceView)
	{
		DX_SAFE_RELEASE(m_shaderResourceView);
	}
	if (m_renderTargetView)
	{
		DX_SAFE_RELEASE(m_renderTargetView);
	}
	if (m_renderTargetTexture)
	{
		DX_SAFE_RELEASE(m_renderTargetTexture);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
