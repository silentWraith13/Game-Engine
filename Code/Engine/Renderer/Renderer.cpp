#include "ThirdParty/stb/stb_image.h"
#define WIN32_LEAN_AND_MEAN		
#include <windows.h>
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/UnorderedAccessBuffer.hpp"
#include "Engine/Renderer/RenderTexture.hpp"
#include "Engine/Renderer/Texture.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

//--------------------------------------------------------------------------------------------------------------------------------------------------------

#define ENGINE_DEBUG_RENDER
#if defined(ENGINE_DEBUG_RENDER)
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
#endif
//--------------------------------------------------------------------------------------------------------------------------------------------------------

struct CameraConstants
{
	Mat44 ProjectionMatrix;
	Mat44 ViewMatrix;
};

static const int k_cameraConstantsSlot = 2;

//--------------------------------------------------------------------------------------------------------------------------------------------------------

struct ModelConstants
{
	Mat44 ModelMatrix;
	float ModelColor[4];
};

static const int k_modelConstantsSlot = 3;

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct LightConstants
{
	Vec3 sunDirection;
	float sunIntensity;
	float ambientIntensity;
	Vec3  worldEyePosition;
	int   normalMode;
	int   SpecularMode;
	float specularIntensity;
	float specularPower;
};

static const int k_lightConstantsSlot = 1;

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct PointLights
{
	Vec3 PointLightPosition;
	float PointLightIntensity;
	float PointLightColor[4];
	float PointLightRadius;
	float PaddingLight[3];
};

static const int k_pointLightConstantsSlot = 4;

//--------------------------------------------------------------------------------------------------------------------------------------------------------

Renderer::Renderer(RendererConfig const& config)
	:m_config(config)
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Renderer::~Renderer()
{

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::Startup()
{
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	DWORD flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDER)
	flags = D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

	unsigned int deviceFlags = 0;
#if defined(ENGINE_DEBUG_RENDER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//--------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(ENGINE_DEBUG_RENDER)
	m_dxgiDebugModule = (void*) ::LoadLibraryA("dxgidebug.dll");
	if (m_dxgiDebugModule == nullptr)
	{
		ERROR_AND_DIE("Could not load dxgidebug.dll");
	}

	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB) ::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))(__uuidof(IDXGIDebug), &m_dxgiDebug);

	if (m_dxgiDebug == nullptr)
	{
		ERROR_AND_DIE("Could not load debug module");
	}
#endif
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	//Creating a device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferDesc.Width = g_theWindow->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height = g_theWindow->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0; 
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = ((HWND)g_theWindow->GetHwnd());;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, NULL, 0,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, &featureLevel, &m_deviceContext);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create D3D 11 device and swapchain.");
	}

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	//Get Back Buffer texture

	hr = m_swapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), (void**)&m_backbuffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not get swapchain buffer.");
	}

	hr = m_device->CreateRenderTargetView(m_backbuffer, NULL, &m_renderTargetView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create render target view for swap chain buffer.");
	}

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	//Creating and setting the rasterizer state
	CreateRasterizerModes();
	SetRasterizerModes(RasterizerMode::SOLID_CULL_BACK);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	//Creating a depth stencil view 
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = g_theWindow->GetClientDimensions().x;
	textureDesc.Height = g_theWindow->GetClientDimensions().y;
	textureDesc.MipLevels = 4;
	textureDesc.ArraySize = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.SampleDesc.Count = 1;

	hr = m_device->CreateTexture2D(&textureDesc, nullptr, &m_depthStencilTexture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create texture 2D.");
	}

	hr = m_device->CreateDepthStencilView(m_depthStencilTexture, nullptr, &m_depthStencilView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create depth stencil view.");
	}

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	//Creating and setting the depth stencil state
	CreateDepthModes();
	SetDepthModes(DepthMode::ENABLED);
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	CreateSamplerModes();
	SetSamplerModes(SamplerMode::BILINEAR_WRAP);
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	//Immediate vertex buffer
	m_immediateVBO = CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	// Immediate vertex buffer for PNCU
	m_immediatePNCUVBO = CreateVertexBuffer(sizeof(Vertex_PNCU), (sizeof(Vertex_PNCU)));
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	m_modelCBO = CreateConstantBuffer(sizeof(ModelConstants));
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	m_lightCBO = CreateConstantBuffer(sizeof(LightConstants));
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	m_pointLightCBO = CreateConstantBuffer(sizeof(PointLights));
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	//Creating and binding a shader with the name "Default"
    m_defaultShader = CreateShader("Default", shaderSource);
    BindShader(m_defaultShader);
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	IntVec2 imageDimensions(1, 1);
	Rgba8 imageColor(255, 255, 255, 255);
	Image newImage(imageDimensions,imageColor);
	m_defaultTexture = CreateTextureFromImage(newImage);
	BindTexture(m_defaultTexture);
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	//Creating and setting blend modes
	CreateBlendStates();
	SetBlendMode(BlendMode::ALPHA);
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	SetModelConstants();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::BeginFrame()
{
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::Endframe()
{
	m_swapChain->Present(0, 0);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::Shutdown()
{
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_device);
	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_backbuffer);
	DX_SAFE_RELEASE(m_immediateVBO->m_buffer);
	DX_SAFE_RELEASE(m_immediatePNCUVBO->m_buffer);
	DX_SAFE_RELEASE(m_cameraCBO->m_buffer);
	DX_SAFE_RELEASE(m_modelCBO->m_buffer);
	DX_SAFE_RELEASE(m_lightCBO->m_buffer);
	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_depthStencilTexture);
	DX_SAFE_RELEASE(m_pointLightCBO->m_buffer);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	for (int i = 0; i < static_cast<int>(BlendMode::COUNT); i++)
	{
		DX_SAFE_RELEASE(m_blendStates[i]);
	}

	for (int i = 0; i < static_cast<int>(SamplerMode::COUNT); i++)
	{
		DX_SAFE_RELEASE(m_samplerStates[i]);
	}

	for (int i = 0; i < static_cast<int>(RasterizerMode::COUNT); i++)
	{
		DX_SAFE_RELEASE(m_rasterizerStates[i]);
	}

	for (int i = 0; i < static_cast<int>(DepthMode::COUNT); i++)
	{
		DX_SAFE_RELEASE(m_depthStencilStates[i]);
	}
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	//Deleting the cache.4
	for (int i = 0; i < m_loadedShaders.size(); i++)
	{
		delete m_loadedShaders[i];
	}
	m_loadedShaders.clear();

	for (int i = 0; i < m_loadedTextures.size(); i++)
	{
		delete m_loadedTextures[i];
	}
	m_loadedTextures.clear();

#if defined(ENGINE_DEBUG_RENDER)
		((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;
	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::ClearScreen(const Rgba8& clearColor)
{
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	float colorRGBA[4];
	clearColor.GetAsFloats(colorRGBA);

	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, colorRGBA);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::BeginCamera(const Camera& camera)
{
	// Create and Set viewport
	CameraConstants newCamConstants; 
	newCamConstants.ViewMatrix = camera.GetViewMatrix();
 	newCamConstants.ProjectionMatrix = camera.GetProjectionMatrix();
 	CopyCPUtoGPU((void*)&newCamConstants, sizeof(newCamConstants), m_cameraCBO);
 	BindConstantBuffer(k_cameraConstantsSlot,m_cameraCBO);
	
	D3D11_VIEWPORT viewport = {};
	if (camera.GetViewport().GetDimensions().x > 0.f && camera.GetViewport().GetDimensions().y > 0.f)
	{
		viewport.TopLeftX = camera.GetViewport().m_mins.x;
		viewport.TopLeftY = camera.GetViewport().m_mins.y;
		viewport.Width = camera.GetViewport().GetDimensions().x;
		viewport.Height = camera.GetViewport().GetDimensions().y;
	}
	else
	{
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(g_theWindow->GetClientDimensions().x);
		viewport.Height = static_cast<float>(g_theWindow->GetClientDimensions().y);
	}
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	 m_deviceContext->RSSetViewports(1, &viewport);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::EndCamera(const Camera& camera)
{
	(void)camera;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::DrawVertexArray(int numVertexes, const Vertex_PCU* vertexes)
{
	CopyCPUToGPU(vertexes, numVertexes * sizeof(Vertex_PCU), m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, numVertexes, 0);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::DrawVertexArray(int numVertexes, const Vertex_PNCU* vertexes)
{
	CopyCPUToGPU(vertexes, numVertexes * sizeof(Vertex_PNCU), m_immediatePNCUVBO);
	DrawVertexBuffer(m_immediatePNCUVBO, numVertexes, 0);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::DrawVertexArrayUsingLinePrimitives(int numVertexes, VertexBuffer* vbo)
{
	SetStatesIfChanged();
	
	UINT stride = vbo->GetStride();
	UINT offset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &stride, &offset);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	
	m_deviceContext->Draw(numVertexes, 0);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::DrawVertexBuffer(int numVertexes, VertexBuffer* vbo, int numIndices, IndexBuffer* ibo)
{	
	DrawVertexBuffer(vbo, numVertexes, ibo, numIndices);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::SetBlendMode(BlendMode blendMode)
{
	m_desiredBlendMode = blendMode;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::CreateSamplerModes()
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//POINT CLAMP
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	HRESULT hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[static_cast<int>(SamplerMode::POINT_CLAMP)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create POINT_CLAMP sampler state");
	}

	//BILINEAR_WRAP
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[static_cast<int>(SamplerMode::BILINEAR_WRAP)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create BILINEAR_WRAP sampler state");
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::SetSamplerModes(SamplerMode samplerMode)
{
	m_desiredSamplerMode = samplerMode;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::CreateRasterizerModes()
{
	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.DepthClipEnable = true;
	rasterDesc.AntialiasedLineEnable = true;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0;
	rasterDesc.SlopeScaledDepthBias = 0;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	
	//SOLID_CULL_NONE
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	HRESULT hr = m_device->CreateRasterizerState(&rasterDesc, &m_rasterizerStates[static_cast<int>(RasterizerMode::SOLID_CULL_NONE)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create SOLID_CULL_NONErasterizing state.");
	}
	
	//SOLID_CULL_BACK
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	hr = m_device->CreateRasterizerState(&rasterDesc, &m_rasterizerStates[static_cast<int>(RasterizerMode::SOLID_CULL_BACK)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create SOLID_CULL_BACK rasterizing state.");
	}
	
	//WIREFRAME_CULL_NONE
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	hr = m_device->CreateRasterizerState(&rasterDesc, &m_rasterizerStates[static_cast<int>(RasterizerMode::WIREFRAME_CULL_NONE)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create WIREFRAME_CULL_NONE rasterizing state.");
	}
	
	//WIREFRAME_CULL_BACK
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	hr = m_device->CreateRasterizerState(&rasterDesc, &m_rasterizerStates[static_cast<int>(RasterizerMode::WIREFRAME_CULL_BACK)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create WIREFRAME_CULL_BACK rasterizing state.");
	}

	//WIREFRAME_CULL_BACK
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = false;
	hr = m_device->CreateRasterizerState(&rasterDesc, &m_rasterizerStates[static_cast<int>(RasterizerMode::SOLID_CULL_BACK_FRONT_CW)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create SOLID_CULL_BACK_FRONT_CW rasterizing state.");
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::SetRasterizerModes(RasterizerMode rasterizerMode)
{
	m_desiredRasterizerMode = rasterizerMode;

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::CreateDepthModes()
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
	depthStencilDesc.DepthEnable = true;

	//DISABLED
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	HRESULT hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[static_cast<int>(DepthMode::DISABLED)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create DISABLED depth stencil state.");
	}

	//ENABLED
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilStates[static_cast<int>(DepthMode::ENABLED)]);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create ENABLED depth stencil state.");
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::SetDepthModes(DepthMode depthMode)
{
	m_desiredDepthMode = depthMode;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
RenderTexture* Renderer::CreateRenderTexture(int width, int height)
{
	RenderTexture* newRenderTexture = new RenderTexture(width, height);

	// Setup texture description for render target
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// Create the render target texture
	HRESULT hr = m_device->CreateTexture2D(&textureDesc, nullptr, &newRenderTexture->m_renderTargetTexture);
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Failed to create render target texture.");
	}

	// Create the render target view
	hr = m_device->CreateRenderTargetView(newRenderTexture->m_renderTargetTexture, nullptr, &newRenderTexture->m_renderTargetView);
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Failed to create render target view.");
	}

	// Create the shader resource view
	hr = m_device->CreateShaderResourceView(newRenderTexture->m_renderTargetTexture, nullptr, &newRenderTexture->m_shaderResourceView);
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Failed to create shader resource view.");
	}

	return newRenderTexture;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::SetRenderTarget(RenderTexture* renderTexture)
{
	m_deviceContext->OMSetRenderTargets(1, &renderTexture->m_renderTargetView, m_depthStencilView);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::ClearRenderTarget(RenderTexture* renderTexture, Rgba8 clearColor)
{
	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = clearColor.r;
	color[1] = clearColor.g;
	color[2] = clearColor.b;
	color[3] = clearColor.a;
	m_deviceContext->ClearRenderTargetView(renderTexture->m_renderTargetView, color);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Texture* Renderer::CreateOrGetTextureFromNothing(std::string name, IntVec2 dimensions, ID3D11Texture2D* textureR, ID3D11ShaderResourceView* shaderResourceView)
{
	Texture* texture = new Texture();
	texture->m_dimensions = dimensions;
	texture->m_name = name;
	texture->m_texture = textureR;
	texture->m_shaderResourceView = shaderResourceView;
	return texture;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::SetBackBufferRenderTarget()
{
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::BindRenderTexture(RenderTexture* texture, unsigned int slot /*= 0*/)
{
	if (texture)
	{
		m_deviceContext->PSSetShaderResources(slot, 1, &texture->m_shaderResourceView);
	}
	else
	{
		m_deviceContext->PSSetShaderResources(slot, 1, &m_defaultTexture->m_shaderResourceView);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Texture* Renderer::CreateCubeMapFomFile(char const* imagePath)
{
	std::string imagePathStr = imagePath;
	size_t lastDotPos = imagePathStr.find_last_of(".");
	if (lastDotPos == std::string::npos)
	{
		ERROR_AND_DIE("Invalid file path for cube map creation.");
	}

	std::string baseFileName = imagePathStr.substr(0, lastDotPos);
	std::string extension = imagePathStr.substr(lastDotPos);

	std::vector<Image> cubeFaces;
	for (int cubeFaceInd = 0; cubeFaceInd < 6; cubeFaceInd++)
	{
		std::string facePath = baseFileName + std::to_string(cubeFaceInd) + extension;

		if (DoesFileExist(facePath))
		{
			cubeFaces.emplace_back(facePath.c_str());
		}
		else
		{
			ERROR_AND_DIE(Stringf("Cube map face file missing: %s", facePath.c_str()).c_str());
		}
	}

	Texture* newCubeMap = CreateCubemapFromImages(cubeFaces);
	return newCubeMap;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::SetStatesIfChanged()
{
	if (m_blendState != m_blendStates[static_cast<int>(m_desiredBlendMode)])
	{
		m_blendState = m_blendStates[static_cast<int>(m_desiredBlendMode)];

		float floatArray[4] = { 0.f, 0.f, 0.f, 0.f };
		m_deviceContext->OMSetBlendState(m_blendState, floatArray, 0xffffffff);
	}

	if (m_samplerState != m_samplerStates[static_cast<int>(m_desiredSamplerMode)])
	{
		m_samplerState = m_samplerStates[static_cast<int>(m_desiredSamplerMode)];
		m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);
	}

	if(m_rasterizerState != m_rasterizerStates[static_cast<int>(m_desiredRasterizerMode)])
	{
		m_rasterizerState = m_rasterizerStates[static_cast<int>(m_desiredRasterizerMode)];
		m_deviceContext->RSSetState(m_rasterizerState);
	}

	if (m_depthStencilState != m_depthStencilStates[static_cast<int>(m_desiredDepthMode)])
	{
		m_depthStencilState = m_depthStencilStates[static_cast<int>(m_desiredDepthMode)];
		m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
	}
}

//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName(imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	return newTexture;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Texture* Renderer::GetTextureForFileName(char const* imageFilePath)
{
	for (Texture* t : m_loadedTextures)
	{
		if (t->GetImageFilePath() == imageFilePath)
		{
			return t;
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromFile(char const* imageFilePath)
{
	Image newImage(imageFilePath);
	Texture* newTexture = CreateTextureFromImage(newImage);
	return newTexture;
}

//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData)
{
	GUARANTEE_OR_DIE(texelData, Stringf("CreateTextureFromData failed for \"%s\" - texelData was null!", name));
	GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel));
	GUARANTEE_OR_DIE(dimensions.x > 0 && dimensions.y > 0, Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y));

	Texture* newTexture = new Texture();
	newTexture->m_name = name; 
	newTexture->m_dimensions = dimensions;

	m_loadedTextures.push_back(newTexture);
	return newTexture;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromImage(const Image& image)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = image.m_dimensions.x;
	textureDesc.Height = image.m_dimensions.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;

	D3D11_SUBRESOURCE_DATA textureData;
	textureData.pSysMem = image.GetRawData();
	textureData.SysMemPitch = image.m_dimensions.x * sizeof(Rgba8);

	Texture* newTexture = new Texture();
	newTexture->m_dimensions = IntVec2(image.m_dimensions.x, image.m_dimensions.y);
	HRESULT hr = m_device->CreateTexture2D(&textureDesc, &textureData, &newTexture->m_texture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create texture2D.");
	}

	hr = m_device->CreateShaderResourceView(newTexture->m_texture, NULL, &newTexture->m_shaderResourceView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create shader resource view");
	}
	newTexture->m_name = image.m_imageFilePath;
	m_loadedTextures.push_back(newTexture);
	return newTexture;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
BitmapFont* Renderer::GetBitmapForFileName(char const* bitmapFontFilePath)
{
	for (int i = 0; i < m_loadedFonts.size(); i++)
	{
		if (m_loadedFonts[i]->m_fontFilePathNameWithNoExtension == bitmapFontFilePath)
		{
			return m_loadedFonts[i];
		}
	}
	return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
BitmapFont* Renderer::CreateOrGetBitmapFontFromFile(char const* bitmapFontFilePathWithoutExtension)
{
	std::string fontFilePath = Stringf("%s.png", bitmapFontFilePathWithoutExtension);
	BitmapFont* existingBitmapFont = GetBitmapForFileName(fontFilePath.c_str());
	if (existingBitmapFont)
	{
		return existingBitmapFont;
	}
	BitmapFont* newBitmapFont = CreateBitmapFontFromFile(fontFilePath.c_str());
	return newBitmapFont;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
BitmapFont* Renderer::CreateBitmapFontFromFile(char const* bitmapFontFilePath)
{
	Texture* fontTexture = CreateOrGetTextureFromFile(bitmapFontFilePath);
	BitmapFont* newFont = new BitmapFont(bitmapFontFilePath, *fontTexture);
	m_loadedFonts.push_back(newFont);
	return newFont;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::BindShader(Shader* shader)
{
	if (shader == nullptr)
	{
		shader = m_defaultShader;
	}

	m_deviceContext->IASetInputLayout(shader->m_inputLayout);
	m_deviceContext->VSSetShader(shader->m_vertexShader, nullptr, 0);
	m_deviceContext->PSSetShader(shader->m_pixelShader, nullptr, 0);

	if (shader->m_geometryShader)
	{
		m_deviceContext->GSSetShader(shader->m_geometryShader, nullptr, 0);
	}
	else
	{
		m_deviceContext->GSSetShader(nullptr, nullptr, 0);  
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Shader* Renderer::GetShaderByName(const char* shaderName)
{
	for (Shader* s : m_loadedShaders)
	{
		if (s->GetName() == shaderName)
		{
 			return s;
		}
	}
	return nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 Shader* Renderer::CreateShader(char const* shaderName, char const* source, VertexType  vertexType/*= VertexType::PCU */)
 {
	 ShaderConfig newConfig;
	 newConfig.m_name = shaderName;
	 Shader* newShader = new Shader(newConfig);

	 std::vector<UCHAR> vertexByteCode;
	 std::vector<UCHAR> pixelByteCode;
	 std::vector<UCHAR> geometryByteCode;

	 D3D11_INPUT_ELEMENT_DESC inputElementDesc_PCU[] =
	 {
		 {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	 };

	 D3D11_INPUT_ELEMENT_DESC inputElementDesc_PNCU[] =
	 {
		 {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	 };
	
	 D3D11_INPUT_ELEMENT_DESC inputElementDesc_PCUTBN[] =
	 {
		 {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	 };
	
	 D3D11_INPUT_ELEMENT_DESC inputElementDesc_DENSITY[] =
	 {
		 {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		 {"DENSITY", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	 };

	 CompileShaderToByteCode(vertexByteCode, shaderName, source, "VertexMain", "vs_5_0");
	 HRESULT hr = 0;
	 if (vertexType == VertexType::PCU)
	 {
		 hr = m_device->CreateInputLayout(inputElementDesc_PCU, 3, vertexByteCode.data(), vertexByteCode.size(), &newShader->m_inputLayout);
	 }
	 else if (vertexType == VertexType::PNCU)
	 {
		 hr = m_device->CreateInputLayout(inputElementDesc_PNCU, 4, vertexByteCode.data(), vertexByteCode.size(), &newShader->m_inputLayout);
	 }
	 else if (vertexType == VertexType::PCUTBN)
	 {
		 hr = m_device->CreateInputLayout(inputElementDesc_PCUTBN, 6, vertexByteCode.data(), vertexByteCode.size(), &newShader->m_inputLayout);
	 }

	 if (!SUCCEEDED(hr))
	 {
		 ERROR_AND_DIE("Could not create input layout.");
	 }

	 CompileShaderToByteCode(pixelByteCode, shaderName, source, "PixelMain", "ps_5_0");
	 hr = m_device->CreateVertexShader(vertexByteCode.data(), vertexByteCode.size(), NULL, &newShader->m_vertexShader);
	 if (!SUCCEEDED(hr))
	 {
		 ERROR_AND_DIE("Could not create vertex shader.");
	 }
	 hr = m_device->CreatePixelShader(pixelByteCode.data(), pixelByteCode.size(), NULL, &newShader->m_pixelShader);

	bool hasGeomtryShader = (std::string(source).find("GeometryMain") != std::string::npos);

	if (hasGeomtryShader)
	{
		CompileShaderToByteCode(geometryByteCode, shaderName, source, "GeometryMain", "gs_5_0");
		hr = m_device->CreateGeometryShader(geometryByteCode.data(), geometryByteCode.size(), NULL, &newShader->m_geometryShader);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Could not create geometry shader.");
		}
	}
	

	 m_loadedShaders.push_back(newShader);
	 return newShader;
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
  Shader* Renderer::CreateShaderOrGetFromFile(char const* shaderName, VertexType  vertexType/*= VertexType::PCU */)
  {
	  Shader* existingShader = GetShaderByName(shaderName);
	  if (existingShader)
	  {
		  return existingShader;
	  }

	  std::string filename = shaderName;
	  filename.append(".hlsl");

	  std::string source;
	  int result = FileReadToString(source, filename);
	  if (result != 0)
	  {
		  return nullptr;
	  }
	  Shader* newShader = CreateShader(shaderName, source.c_str(), vertexType);
	  return newShader;
  }
  //--------------------------------------------------------------------------------------------------------------------------------------------------------
  Shader* Renderer::CreateComputeShaderOnly(char const* shaderName, char const* entryPoint)
  {
	  Shader* existingShader = GetShaderByName(shaderName);
	  if (existingShader)
	  {
		  return existingShader;
	  }

	  std::string filename = shaderName;
	  filename.append(".hlsl");

	  std::string source;
	  int result = FileReadToString(source, filename);
	  if (result != 0)
	  {
		  return nullptr;
	  }

	  ShaderConfig newConfig;
	  newConfig.m_name = shaderName;
	  Shader* newShader = new Shader(newConfig);

	  std::vector<UCHAR> computeByteCode;
	  CompileShaderToByteCode(computeByteCode, shaderName, source.c_str(), entryPoint, "cs_5_0");
	  HRESULT hr;
	  hr = m_device->CreateComputeShader(computeByteCode.data(), computeByteCode.size(), NULL, &newShader->m_computeShader);
	  if (!SUCCEEDED(hr))
	  {
		  ERROR_AND_DIE("Could not create compute shader.");
	  }

	//  m_loadedShaders.push_back(newShader);
	  return newShader;
  }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target)
{
	DWORD deviceFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDER)
	deviceFlags = D3DCOMPILE_DEBUG;
	deviceFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	//deviceFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompile(source, strlen(source), name, nullptr, nullptr, entryPoint, target, deviceFlags, 0, &shaderBlob, &errorBlob);
	if (SUCCEEDED(hr))
	{
		outByteCode.resize(shaderBlob->GetBufferSize());
		memcpy(outByteCode.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
		shaderBlob->Release();
		return true;
	}
	else
	{
		DebuggerPrintf((const char*)errorBlob->GetBufferPointer());
		if (shaderBlob)
		{
			shaderBlob->Release();
		}
		errorBlob->Release();
		ERROR_AND_DIE("Could not compile shader");
	}		
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
VertexBuffer* Renderer::CreateVertexBuffer(const size_t numVerts, const unsigned int stride)
{
	// Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = static_cast<UINT>(numVerts);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.StructureByteStride = stride;

	VertexBuffer* vertexBuffer = new VertexBuffer( numVerts, stride);
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &vertexBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex buffer.");
	}
	else
	{
		return vertexBuffer;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::CopyCPUToGPU(const void* data, size_t size, VertexBuffer*& vbo)
{
	//--------------------------------------------------------------------------------------------------------------------------------------------------------
	//Checking size of buffer
	if (vbo->m_size < size)
	{
		unsigned int stride = 0;
		if (vbo->m_buffer) 
		{
			stride = vbo->GetStride();
			vbo->m_buffer->Release();
		}
		
		// create new buffer with sufficient size
		vbo = CreateVertexBuffer(size, stride);
		if (!vbo)
		{
			ERROR_AND_DIE( "Memory allocation for vertex buffer failed.");
			return;
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------------------------------------

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	if (!vbo || !vbo->m_buffer) 
	{
		ERROR_AND_DIE( "Invalid vertex buffer pointer.");
		return;
	}

	if (!m_deviceContext) 
	{
		ERROR_AND_DIE("Invalid device context pointer.");
		return;
	}

	HRESULT result = m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (SUCCEEDED(result))
	{
		memcpy(mappedResource.pData, data, size);
		m_deviceContext->Unmap(vbo->m_buffer, 0);
	}
	if (!SUCCEEDED(result))
	{
		ERROR_AND_DIE("Could not map.");
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IndexBuffer* Renderer::CreateIndexBuffer(const size_t numIndexes, const unsigned int stride)
{
	//Create vertex  buffer
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = static_cast<UINT>(numIndexes);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.StructureByteStride = stride;

	IndexBuffer* indexBuffer = new IndexBuffer(numIndexes, stride);
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &indexBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create index buffer.");
	}
	else
	{
		return indexBuffer;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::CopyCPUToGPU(const void* data, size_t size, IndexBuffer*& ibo)
{
	if (ibo->m_size < size)
	{
		unsigned int stride = 0;
		if (ibo->m_buffer)
		{
			stride = ibo->GetStride();
			ibo->m_buffer->Release();
		}

		ibo = CreateIndexBuffer(size, stride);
		if (!ibo)
		{
			ERROR_AND_DIE("Memory allocation for index buffer failed.");
			return;
		}
	}

	if (!ibo || !ibo->m_buffer)
	{
		ERROR_AND_DIE("Invalid index buffer pointer.");
		return;
	}

	if (!m_deviceContext)
	{
		ERROR_AND_DIE("Invalid device context pointer.");
		return;
	}
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (SUCCEEDED(result))
	{
		memcpy(mappedResource.pData, data, size);
		m_deviceContext->Unmap(ibo->m_buffer, 0);
	}
	if (!SUCCEEDED(result))
	{
		ERROR_AND_DIE("Could not map.");
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::BindIndexBuffer(IndexBuffer* ibo)
{
	m_deviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::DrawIndexBuffer(IndexBuffer* ibo, int indexCount, int indexOffset /*= 0*/)
{
	(void)indexOffset;
	SetStatesIfChanged();
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::BindVertexBuffer(VertexBuffer* vbo)
{
	UINT stride = vbo->GetStride();
	UINT offset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &stride, &offset);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset /*= 0*/)
{
	SetStatesIfChanged();
	BindVertexBuffer(vbo);
	m_deviceContext->Draw(vertexCount, vertexOffset);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, IndexBuffer* ibo, int indexCount)
{
	(void)vertexCount;
	SetStatesIfChanged();
	BindVertexBuffer(vbo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
ConstantBuffer* Renderer::CreateConstantBuffer(const size_t size)
{
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = static_cast<UINT>(size);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ConstantBuffer* constantBuffer = new ConstantBuffer(size);
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &constantBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create constant buffer.");
	}
	else
	{
		return constantBuffer;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::CopyCPUtoGPU(const void* data, size_t size, ConstantBuffer* cbo)
 {
 
	 D3D11_MAPPED_SUBRESOURCE mappedResource = {};
 
 	if (!cbo || !cbo->m_buffer)
 	{
 		ERROR_AND_DIE("Invalid constant buffer pointer.");
 		return;
 	}
 
 	if (!m_deviceContext)
 	{
 		ERROR_AND_DIE("Invalid device context pointer.");
 		return;
 	}
 
 	HRESULT result = m_deviceContext->Map(cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
 	if (SUCCEEDED(result))
 	{
 		memcpy(mappedResource.pData, data, size);
  		m_deviceContext->Unmap(cbo->m_buffer, 0);
 	}
 	if (!SUCCEEDED(result))
 	{
 		ERROR_AND_DIE("Could not map.");
 	}
 
 }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::BindConstantBuffer(int slot, ConstantBuffer* cbo)
 {
 	m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
 	m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->GSSetConstantBuffers(slot, 1, &cbo->m_buffer);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::BindConstantBufferWithGS(int slot, ConstantBuffer* cbo)
 {
	 m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	 m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	 m_deviceContext->GSSetConstantBuffers(slot, 1, &cbo->m_buffer);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 UnorderedAccessBuffer* Renderer::CreateUnorderedAccessBuffer(const size_t size, const unsigned int stride, void* data)
 {
	 D3D11_BUFFER_DESC bufferDesc = { 0 };
	 bufferDesc.ByteWidth = static_cast<UINT>(size * stride);
	 bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	 bufferDesc.StructureByteStride = stride;
	 bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;


	 D3D11_SUBRESOURCE_DATA* initialData = nullptr;
	 if (data)
	 {
		 initialData = new D3D11_SUBRESOURCE_DATA();
		initialData->pSysMem = data;
	 }

	 UnorderedAccessBuffer* uavBuffer = new UnorderedAccessBuffer(size, stride, data);
	 HRESULT hr = m_device->CreateBuffer(&bufferDesc, initialData, &uavBuffer->m_buffer);
	 if (!SUCCEEDED(hr))
	 {
		 ERROR_AND_DIE("Could not create structured buffer.");
	 }

	 D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	 uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	 uavDesc.Buffer.FirstElement = 0;
	 uavDesc.Buffer.NumElements = static_cast<UINT>(size);
	 hr = m_device->CreateUnorderedAccessView(uavBuffer->m_buffer, &uavDesc, &uavBuffer->m_UAV);
	 if (!SUCCEEDED(hr))
	 {
		 ERROR_AND_DIE("Could not create unordered access view.");
	 }

	 D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	 srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	 srvDesc.Buffer.FirstElement = 0;
	 srvDesc.Buffer.NumElements = static_cast<UINT>(size);
	 hr = m_device->CreateShaderResourceView(uavBuffer->m_buffer, &srvDesc, &uavBuffer->m_SRV);
	 if (!SUCCEEDED(hr))
	 {
		 ERROR_AND_DIE("Could not create shader resource view.");
	 }

	 return uavBuffer;
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::VSSetShaderResource(int slot, int numViews, UnorderedAccessBuffer* uav)
 {
	 ID3D11ShaderResourceView* srv = nullptr;
	 if (uav) 
	 {
		 srv = uav->m_SRV;
	 }
	 m_deviceContext->VSSetShaderResources(slot, numViews, &srv);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::IASetVertexBuffers(VertexBuffer* vbo)
 {
	 if (vbo == nullptr)
	 {
		 ID3D11Buffer* pNullBuffer = nullptr;
		 UINT iNullUINT = 0;
		m_deviceContext->IASetVertexBuffers(0, 1 , &pNullBuffer, &iNullUINT, &iNullUINT);
		return;
	 }

	 UINT stride = vbo->GetStride();
	 UINT offset = 0;
	 m_deviceContext->IASetVertexBuffers(NULL, 1, &vbo->m_buffer, &stride, &offset);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::Draw(int numVertexes)
 {
	 SetStatesIfChanged();
	 m_deviceContext->Draw(numVertexes, 0);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::IASetPrimitiveTopology()
 {
	 m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::CSSetUnorderedAccessViews(int numSlot, UnorderedAccessBuffer* uav)
 {
	 UINT UAVInitialCounts = 0;
	 ID3D11UnorderedAccessView* UAV = nullptr;
	 if (uav)
	 {
		 UAV = uav->m_UAV;
	 }
	 m_deviceContext->CSSetUnorderedAccessViews(numSlot, 1, &UAV, &UAVInitialCounts);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::CSSetShaderResources(int slot, int numViews, UnorderedAccessBuffer* uav)
 {
	 ID3D11ShaderResourceView* srv = nullptr;
	 if (uav)
	 {
		 srv = uav->m_SRV;
	 }
	 m_deviceContext->CSSetShaderResources(slot, numViews, &srv);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::CSSetShader(Shader* cs)
 {
	 ID3D11ComputeShader* shader = nullptr;
	 if (cs)
	 {
		 shader = cs->m_computeShader;
	 }
	 m_deviceContext->CSSetShader(shader, nullptr, 0);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::CSSetConstantBuffers(int startSlot, int numBuffers, ConstantBuffer* cb)
 {
	 ID3D11Buffer* buffer = nullptr;
	 if (cb)
	 {
		 buffer = cb->m_buffer;
	 }
	 m_deviceContext->CSSetConstantBuffers(startSlot, numBuffers, &buffer);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::DispatchCS(int threadX, int threadY, int threadZ)
 {
	 m_deviceContext->Dispatch(threadX, threadY, threadZ);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::CopyResource(UnorderedAccessBuffer* toCopyTo, UnorderedAccessBuffer* toCopyFrom)
 {
	 m_deviceContext->CopyResource( toCopyTo->m_buffer, toCopyFrom->m_buffer);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::SetModelConstants(const Mat44& modelMatrix /*= Mat44()*/, const Rgba8& modelColor /*= Rgba8(255, 255, 255, 255)*/)
 {
	 ModelConstants newModelConstants;
	 newModelConstants.ModelMatrix = modelMatrix;
	 modelColor.GetAsFloats(newModelConstants.ModelColor);

	 CopyCPUtoGPU(&newModelConstants, sizeof(ModelConstants), m_modelCBO);

	 BindConstantBuffer(k_modelConstantsSlot, m_modelCBO);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::SetLightConstants(Vec3 sunDirection, float sunIntensity, float ambientIntensity, Vec3 worldEyePos, int normalMode, int specularMode, float specularIntensity, float specularPower)
 {
	 LightConstants lightConstants;
	 lightConstants.sunDirection = sunDirection.GetNormalized();
	 lightConstants.sunIntensity = sunIntensity;
	 lightConstants.ambientIntensity = ambientIntensity;
	 lightConstants.worldEyePosition = worldEyePos;
	 lightConstants.normalMode = normalMode;
	 lightConstants.SpecularMode = specularMode;
	 lightConstants.specularIntensity = specularIntensity;
	 lightConstants.specularPower = specularPower;

	 CopyCPUtoGPU(&lightConstants, sizeof(lightConstants), m_lightCBO);
	 BindConstantBuffer(k_lightConstantsSlot, m_lightCBO);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::SetPointLightConstants(Vec3 Position, float intensity, Rgba8 color, float radius)
 {
	 PointLights pointLights;
	 pointLights.PointLightPosition = Position;
	 pointLights.PointLightIntensity = intensity;
	 color.GetAsFloats(pointLights.PointLightColor);
	 pointLights.PointLightRadius = radius;
	 
	 CopyCPUtoGPU(&pointLights, sizeof(pointLights), m_pointLightCBO);
	 BindConstantBuffer(k_pointLightConstantsSlot, m_pointLightCBO);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Renderer::CreateBlendStates()
 {
	 D3D11_BLEND_DESC blend_desc = { 0 };
	 blend_desc.RenderTarget[0].BlendEnable = true;
	 blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	 blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	 blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	 blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	 blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	 //OPAQUE
	 blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	 blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	 HRESULT hr = m_device->CreateBlendState(&blend_desc, &m_blendStates[static_cast<int>(BlendMode::OPAQUE)]);
	 if (!SUCCEEDED(hr))
	 {
		 ERROR_AND_DIE("Failed to create OPAQUE blend state");
	 }
	 
	 //ALPHA
	 blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	 blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	 hr = m_device->CreateBlendState(&blend_desc, &m_blendStates[static_cast<int>(BlendMode::ALPHA)]);
	 if (!SUCCEEDED(hr))
	 {
		 ERROR_AND_DIE("Failed to create ALPHA blend state");
	 }

	 //ADDITIVE
	 blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	 blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	 hr = m_device->CreateBlendState(&blend_desc, &m_blendStates[static_cast<int>(BlendMode::ADDITIVE)]);
	 if (!SUCCEEDED(hr))
	 {
		 ERROR_AND_DIE("Failed to create ADDITIVEXX blend state");
	 }
 }

//-----------------------------------------------------------------------------------------------
void Renderer::BindTexture(const Texture* texture, unsigned int slot /*= 0*/)
{
	if (texture)
	{
		m_deviceContext->PSSetShaderResources(slot, 1, &texture->m_shaderResourceView);
	}
	else
	{
		m_deviceContext->PSSetShaderResources(slot, 1, &m_defaultTexture->m_shaderResourceView);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::BindTextures(const Texture* texture0, const Texture* texture1, const Texture* texture2)
{
	if (texture0)
	{
		m_deviceContext->PSSetShaderResources(0, 1, &texture0->m_shaderResourceView);
	}
	else
	{
		m_deviceContext->PSSetShaderResources(0, 1, &m_defaultTexture->m_shaderResourceView);
	}

	if (texture1)
	{
		m_deviceContext->PSSetShaderResources(1, 1, &texture1->m_shaderResourceView);
	}
	else
	{
		m_deviceContext->PSSetShaderResources(0, 1, &m_defaultTexture->m_shaderResourceView);
	}

	if (texture2)
	{
		m_deviceContext->PSSetShaderResources(2, 1, &texture2->m_shaderResourceView);
	}
	else
	{
		m_deviceContext->PSSetShaderResources(0, 1, &m_defaultTexture->m_shaderResourceView);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Texture* Renderer::CreateCubemapFromImages(std::vector<Image> const& images)
{
	Texture* newTexture = CreateCubemap(images);
	m_loadedTextures.push_back(newTexture);
	return newTexture;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Texture* Renderer::CreateCubemap(std::vector<Image> const& images)
{
	if (images.size() != 6)
	{
		ERROR_AND_DIE("Six images are required to create a cubemap");
	}

	Image const& firstImage = images[0];

	D3D11_TEXTURE2D_DESC pTextureDesc = { 0 };
	pTextureDesc.Width = firstImage.GetDimensions().x;
	pTextureDesc.Height = firstImage.GetDimensions().y;
	pTextureDesc.MipLevels = 1;
	pTextureDesc.ArraySize = 6;
	pTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	pTextureDesc.SampleDesc.Count = 1;
	pTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	pTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	pTextureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SUBRESOURCE_DATA pIntialData[6];
	for (int cubeFaceInd = 0; cubeFaceInd < 6; cubeFaceInd++)
	{
		pIntialData[cubeFaceInd].pSysMem = images[cubeFaceInd].GetRawData();
		pIntialData[cubeFaceInd].SysMemPitch = ((UINT)images[cubeFaceInd].GetSizeBytes() / (UINT)images[cubeFaceInd].GetDimensions().y);
		pIntialData[cubeFaceInd].SysMemSlicePitch = 0;
	}

	ID3D11Texture2D* cubeTexture = nullptr;
	HRESULT textureResult = m_device->CreateTexture2D(&pTextureDesc, pIntialData, &cubeTexture);
	if (!SUCCEEDED(textureResult))
	{
		std::string errorString = "Error while creating Cubemap texture: ";
		errorString.append(firstImage.GetImageFilePath());
		ERROR_AND_DIE(errorString);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = pTextureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1; 

	ID3D11ShaderResourceView* srv = nullptr;
	HRESULT srvResult = m_device->CreateShaderResourceView(cubeTexture, &srvDesc, &srv);
	if (!SUCCEEDED(srvResult))
	{
		cubeTexture->Release();
		ERROR_AND_DIE("Failed to create shader resource view for the cubemap");
	}

	Texture* tex = new Texture();
	tex->m_name = firstImage.GetImageFilePath();
	tex->m_dimensions = firstImage.GetDimensions();
	tex->m_texture = cubeTexture;
	tex->m_shaderResourceView = srv;


	return tex;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Renderer::DestroyTexture(Texture* texture)
{
	if (texture != nullptr)
	{
		delete texture;
		texture = nullptr;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------