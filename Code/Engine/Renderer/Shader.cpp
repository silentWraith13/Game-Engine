#include "Engine/Renderer/Shader.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Shader::Shader(const ShaderConfig& config)
	:m_config(config)
{

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Shader::~Shader()
{
	if (m_inputLayout)
	{
		DX_SAFE_RELEASE(m_inputLayout);
	}
	if (m_pixelShader)
	{
		DX_SAFE_RELEASE(m_pixelShader);
	}
	if (m_vertexShader)
	{
		DX_SAFE_RELEASE(m_vertexShader);
	}
	if (m_geometryShader)
	{
		DX_SAFE_RELEASE(m_geometryShader);
	}
	if (m_computeShader)
	{
		DX_SAFE_RELEASE(m_computeShader);
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const std::string& Shader::GetName() const
{
	return m_config.m_name;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------