#include <d3d11.h>
#include <d3d12.h>

#include "Engine/Renderer/ShaderD12.hpp"
#include "Engine/Renderer/RendererD12.hpp"
#include <dxcapi.h>


ShaderD12::ShaderD12(const ShaderConfigD12& config, RendererD12* renderer)
{
	m_config = config;
	m_renderer = renderer;
}

void ShaderD12::CreateShaderObjects()
{
	if (m_config.m_name.find("3D") != std::string::npos)
	{
		m_config.m_shaderType = ShaderDetails::Shader3D;
		if (m_config.m_name.find("WireFrame") != std::string::npos)
		{
			m_config.m_fillMode = FillModeD12::WIREFRAME;
		}
		Create3DRootSignature();
		Create3DPipelineStateObject();
	}
	else
	{
		m_config.m_shaderType = ShaderDetails::Shader2D;
		CreateRootSignature();
		CreatePipelineStateObject();
	}
}
void ShaderD12::Create3DRootSignature()
{
	//-------------------CREATING ROOT SIGNATURE-----------------------------
	CD3DX12_DESCRIPTOR_RANGE ranges[(int)Default3DRootSignatureParams::Count]; // Perfomance TIP: Order from most frequent to least frequent.

		//---------------------------SRV---------------------------------------
	ranges[(int)Default3DRootSignatureParams::DiffuseTexture].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // 1 output texture
	ranges[(int)Default3DRootSignatureParams::ShadowMapTexture].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);  // 1 output texture

	CD3DX12_ROOT_PARAMETER rootParameters[(UINT)Default3DRootSignatureParams::Count] = {};
	rootParameters[(int)Default3DRootSignatureParams::DiffuseTexture].InitAsDescriptorTable(1, &ranges[(int)Default3DRootSignatureParams::DiffuseTexture]);
	rootParameters[(int)Default3DRootSignatureParams::ShadowMapTexture].InitAsDescriptorTable(1, &ranges[(int)Default3DRootSignatureParams::ShadowMapTexture]);
	rootParameters[(int)Default3DRootSignatureParams::CameraConstantBuffer].InitAsConstantBufferView(0);
	rootParameters[(int)Default3DRootSignatureParams::GameConstantBuffer].InitAsConstantBufferView(1);

	CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
	{
		// LinearWrapSampler
		CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT),
		//CD3DX12_STATIC_SAMPLER_DESC(1, D3D12_FILTER_ANISOTROPIC),
	};
	CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, ARRAYSIZE(staticSamplers), staticSamplers , D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	m_renderer->SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_rootSignature);
	m_rootSignature->SetName(L"Vertex and pixel shader Root signature");
}

void ShaderD12::Create3DPipelineStateObject()
{
	//ComPtr<IDxcBlob> vsBlob = nullptr;
	//ComPtr<IDxcBlob> psBlob = nullptr;
	ComPtr<ID3DBlob> vsByteCode  = nullptr;  
	ComPtr<ID3DBlob> psByteCode  = nullptr;
	ComPtr<ID3DBlob> dsByteCode  = nullptr;
	ComPtr<ID3DBlob> hsByteCode  = nullptr;
	const char* shaderFilePath = m_config.m_shaderFilePath.c_str();
	const char* vsEntry = m_vertexEntryPoint.c_str();
	const char* psEntry = m_pixelEntryPoint.c_str();
	vsByteCode = m_renderer->m_shaderCompiler->CompileVsPs(shaderFilePath, nullptr, vsEntry, "vs_5_0");
	psByteCode = m_renderer->m_shaderCompiler->CompileVsPs(shaderFilePath, nullptr, psEntry, "ps_5_0");

	//-------CREATING THE INPUT LAYOUT------------------
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	inputLayout.push_back(inputElementDesc[0]);
	inputLayout.push_back(inputElementDesc[1]);
	inputLayout.push_back(inputElementDesc[2]);
	inputLayout.push_back(inputElementDesc[3]);
	inputLayout.push_back(inputElementDesc[4]);
	inputLayout.push_back(inputElementDesc[5]);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	D3D12_DEPTH_STENCIL_DESC depthDesc = m_renderer->m_depthStencilBuffer.depthDesc;
	//depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	D3D12_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.DepthClipEnable = true;
	m_config.m_fillMode == FillModeD12::SOLID ? rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID :
		rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.InputLayout = { inputLayout.data() , (UINT)inputLayout.size() };
	psoDesc.VS = {reinterpret_cast<BYTE*>(vsByteCode->GetBufferPointer()),vsByteCode->GetBufferSize()};
	psoDesc.PS = {reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer()),psByteCode->GetBufferSize()};

	if (m_config.m_isTesselated)
	{
		const char* hsEntry = m_hsEntryPoint.c_str();
		const char* dsEntry = m_dsEntryPoint.c_str();
		hsByteCode = m_renderer->m_shaderCompiler->CompileVsPs(shaderFilePath, nullptr, hsEntry, "hs_5_0");
		dsByteCode = m_renderer->m_shaderCompiler->CompileVsPs(shaderFilePath, nullptr, dsEntry, "ds_5_0");

		psoDesc.HS = { reinterpret_cast<BYTE*>(hsByteCode->GetBufferPointer()),hsByteCode->GetBufferSize() };
		psoDesc.DS = { reinterpret_cast<BYTE*>(dsByteCode->GetBufferPointer()),dsByteCode->GetBufferSize() };
	}
	psoDesc.RasterizerState = rasterizerDesc;
	psoDesc.BlendState =  CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = depthDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType =	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_renderer->m_backBufferFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = m_renderer->m_depthBufferFormat;

	HRESULT res = m_renderer->m_Rdevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateObject));
	if (!SUCCEEDED(res))
	{
		ERROR_AND_DIE("Failed while creating pipeline state object");
	}
	m_pipelineStateObject->SetName(L"3D pipeline state object");
}


void ShaderD12::CreateRootSignature()
{
	//-------------------CREATING ROOT SIGNATURE-----------------------------
	CD3DX12_DESCRIPTOR_RANGE ranges[(int)DefaultRootSignatureParams::Count]; // Perfomance TIP: Order from most frequent to least frequent.

		//---------------------------SRV---------------------------------------
	ranges[(int)DefaultRootSignatureParams::DiffuseTexture].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // 1 output texture

	CD3DX12_ROOT_PARAMETER rootParameters[(UINT)DefaultRootSignatureParams::Count] = {};
	rootParameters[(int)DefaultRootSignatureParams::DiffuseTexture].InitAsDescriptorTable(1, &ranges[(int)Default3DRootSignatureParams::DiffuseTexture]);
	rootParameters[(int)DefaultRootSignatureParams::CameraConstantBuffer].InitAsConstantBufferView(0);

	CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
	{
		// LinearWrapSampler
		CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT),
		//CD3DX12_STATIC_SAMPLER_DESC(1, D3D12_FILTER_ANISOTROPIC),
	};
	CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, ARRAYSIZE(staticSamplers), staticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	m_renderer->SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_rootSignature);
	m_rootSignature->SetName(L"Vertex and pixel shader 2D Root signature");
}

void ShaderD12::CreatePipelineStateObject()
{
	//ComPtr<IDxcBlob> vsBlob = nullptr;
	//ComPtr<IDxcBlob> psBlob = nullptr;
	ComPtr<ID3DBlob> vsByteCode = nullptr;
	ComPtr<ID3DBlob> psByteCode = nullptr;
	const char* shaderFilePath = m_config.m_shaderFilePath.c_str();
	const char* vsEntry = m_vertexEntryPoint.c_str();
	const char* psEntry = m_pixelEntryPoint.c_str();
	vsByteCode = m_renderer->m_shaderCompiler->CompileVsPs(shaderFilePath, nullptr, vsEntry, "vs_5_0");
	psByteCode = m_renderer->m_shaderCompiler->CompileVsPs(shaderFilePath, nullptr, psEntry, "ps_5_0");
	//-------CREATING THE INPUT LAYOUT------------------
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0,  DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	inputLayout.push_back(inputElementDesc[0]);
	inputLayout.push_back(inputElementDesc[1]);
	inputLayout.push_back(inputElementDesc[2]);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	D3D12_DEPTH_STENCIL_DESC depthDesc = m_renderer->m_depthStencilBuffer.depthDesc;
	//depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	D3D12_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	m_config.m_fillMode == FillModeD12::SOLID ? rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID : 
		rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.DepthClipEnable = false;
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.InputLayout = { inputLayout.data() , (UINT)inputLayout.size() };
	psoDesc.VS = { reinterpret_cast<BYTE*>(vsByteCode->GetBufferPointer()),vsByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer()),psByteCode->GetBufferSize() };
	psoDesc.RasterizerState = rasterizerDesc;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = depthDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_renderer->m_backBufferFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = m_renderer->m_depthBufferFormat;

	HRESULT res = m_renderer->m_Rdevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateObject));
	if (!SUCCEEDED(res))
	{
		ERROR_AND_DIE("Failed while creating pipeline state object for 2d");
	}
	m_pipelineStateObject->SetName(L"2D pipeline State object");
}

const std::string& ShaderD12::GetName() const
{
	return m_config.m_name;
}


ShaderD12::~ShaderD12()
{
	m_rootSignature.Reset();
	m_pipelineStateObject.Reset();
}