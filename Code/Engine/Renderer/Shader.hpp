#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11GeometryShader;
struct ID3D11ComputeShader;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct ShaderConfig
{
	std::string m_name;
	std::string m_vertexEntryPoint = "VertexMain";
	std::string m_pixelEntryPoint = "PixelMain";
	std::string m_geometryEntryPoint = "GeometryMain";
	std::string m_computeEntryPoint = "ComputeMain";
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Shader
{
	friend class Renderer;

public:
	Shader(const ShaderConfig& config);
	Shader(const Shader& copy) = delete;
	virtual ~Shader();

	const std::string& GetName() const;

	ShaderConfig m_config;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader*  m_pixelShader = nullptr;
	ID3D11GeometryShader* m_geometryShader = nullptr;
	ID3D11InputLayout*  m_inputLayout = nullptr;
	ID3D11ComputeShader* m_computeShader = nullptr;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------