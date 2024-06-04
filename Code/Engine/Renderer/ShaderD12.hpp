#include <string>
#include <dxcapi.h>
#include "ThirdParty/D3D12DXR/d3dx12.h"
enum class ShaderDetails
{
	Shader2D,
	Shader3D,
	Raytracing,
	ShaderTypeCount
};


enum class CullModeD12
{
	NONE,
	FRONT,
	BACK,
};
enum class FillModeD12
{
	SOLID,
	WIREFRAME,
};
enum class WindingOrderD12
{
	CLOCKWISE,
	COUNTERCLOCKWISE,

};

struct ShaderConfigD12
{
	std::string					m_name;
	std::string					m_shaderFilePath;
	bool						m_isTesselated = false;
	ShaderDetails				m_shaderType;
	FillModeD12					m_fillMode = FillModeD12::SOLID;
};

class ShaderD12
{
	friend class RendererD12;
public:
	ShaderD12(const ShaderConfigD12& config, RendererD12* renderer);
	ShaderD12(const ShaderD12& copy) = delete;
	~ShaderD12();
	void Create3DPipelineStateObject();

	void CreateShaderObjects();
	void Create3DRootSignature();
	void CreateRootSignature();
	void CreatePipelineStateObject();
	const std::string& GetName() const;
	ShaderDetails GetShaderType() {return m_config.m_shaderType;}
public:
	std::string											m_vertexEntryPoint = "VertexMain";
	std::string											m_pixelEntryPoint = "PixelMain";
	std::string											m_dsEntryPoint = "DomainMain";
	std::string											m_hsEntryPoint = "HullMain";
	RendererD12*										m_renderer;	
	ShaderConfigD12										m_config;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>         m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>         m_pipelineStateObject;

};
