#include "Engine/Renderer/ShadowMap.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <d3d11.h>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
ShadowMap::ShadowMap(int width, int height)
	:m_width(width),m_height(height)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
ShadowMap::~ShadowMap()
{
	if (m_texture)	DX_SAFE_RELEASE(m_texture);
	if (m_DSV)		DX_SAFE_RELEASE(m_DSV);
	if (m_SRV)		DX_SAFE_RELEASE(m_SRV);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
