#include "Engine/Renderer/Texture.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Texture::Texture()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Texture::~Texture()
{
	if (m_texture)
	{
		DX_SAFE_RELEASE(m_texture);
	}
	if (m_shaderResourceView)
	{
		DX_SAFE_RELEASE(m_shaderResourceView);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

