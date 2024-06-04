
#define WIN32_LEAN_AND_MEAN	
#include <windows.h>
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Game/EngineBuildPreferences.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "ThirdParty/D3D12DXR/d3dx12.h"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RaytracingHelpers.hpp"
#include "Engine/Renderer/Denoiser.hpp"
#include "Engine/Renderer/GlobalIllumination.hpp"
#include "Engine/Renderer/Composition.hpp"
#include "Engine/Renderer/PostProcess.hpp"
#include "Engine/Renderer/ResourceManager.hpp"
#include "Engine/Renderer/ShaderD12.hpp"
#include <dxgidebug.h>
#include <combaseapi.h>
#include <dxgi1_6.h>
#/*include <dxcapi.h>*/
#include "Engine/Core/Time.hpp"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <minwindef.h>
#include <Engine/Core/VertexUtils.hpp>
#include "Engine/Renderer/ShadowMap.hpp"
//#include "CompiledShaders\Raytracing.hlsl.h"

#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

#ifdef ENGINE_DEBUG_RENDER
#pragma comment( lib, "ThirdParty/D3D12DXR/DirectXTK12_debug.lib" )
#else
#pragma comment( lib, "ThirdParty/D3D12DXR/DirectXTK12_release.lib" )
#endif // ENGINE_DEBUG_RENDER

#pragma comment( lib, "dxguid.lib" )
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxcompiler.lib")
//
//const unsigned int  RendererD12::MINECRAFTCHUNKS = 10;


namespace
{
	inline DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8X8_UNORM;
		default:                                return fmt;

		}
	}
};

const wchar_t* RendererD12::c_hitGroupName = L"MyHitGroup";
const wchar_t* RendererD12::c_raygenShaderName = L"MyRaygenShader";
const wchar_t* RendererD12::c_closestHitShaderName = L"MyClosestHitShader";
const wchar_t* RendererD12::c_missShaderName = L"MyMissShader";


//-----------------------------CONSTRUCTORS--------------------------
RendererD12::RendererD12()
{
}
RendererD12::RendererD12(RendererD12Config const& rendererConfig)
{
	m_renderConfig = rendererConfig;
}

//-------------------------------MAIN FUNCTIONS-------------------------
void RendererD12::Startup()
{
#ifdef ENGINE_DEBUG_RENDER
	m_dxgiDebugModule = (void*) ::LoadLibraryA("dxgidebug.dll");
	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB) ::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))(__uuidof(IDXGIDebug), (void**)&m_dxgiDebug);
#endif // ENGINE_DEBUG_RENDER

	D3D12InterfaceInitialization();
	if (m_renderingPipeline == RenderingPipeline::Rasterization)
	{
		InitializeRasterization();
	}
}
void RendererD12::BeginFrame()
{

}
void RendererD12::ShutDown()
{
	WaitForGpu();

	for (int i = 0; i < m_loadedShaders.size(); i++)
	{
		delete m_loadedShaders[i];
		m_loadedShaders[i] = nullptr;
	}
	m_loadedShaders.clear();

	for (int i = 0; i < m_loadedTextures.size(); i++)
	{
		delete m_loadedTextures[i];
		m_loadedTextures[i] = nullptr;
	}
	m_loadedTextures.clear();
	m_hemisphereSamplesGPUBuffer.ResetResources();
	m_raytracingGlobalRootSignature.Reset();
	m_raytracingLocalRootSignature.Reset();
	m_bottomLevelASInstanceDescs.ResetResources();
	m_fence.Reset();
	for (int i = 0; i < m_backBufferCount; i++)
	{
		m_RcommandAllocator[i]->Reset();
		m_backBufferRenderTarget[i].Reset();
	}
	delete m_shadowMap;

	delete m_resourceManager;
	m_resourceManager = nullptr;

	delete m_denoiser;
	m_denoiser = nullptr;

	delete m_globalIllumination;
	m_globalIllumination = nullptr;

	delete m_postProcess;
	m_postProcess = nullptr;

	delete m_compositor;
	m_compositor = nullptr;

	m_bunnyTlas.Reset();
	m_bunnyBlas.Reset();
	m_sceneCB.ResetResources();
	m_lightCB.ResetResources();
	m_irradianceCacheGPUBuffer.ResetResources();
	m_rtvDescriptorHeap.Reset();
	m_dsvDescriptorHeap.Reset();
	m_imguiDescriptorHeap.Reset();
	m_dxrDevice.Reset();
	m_dxrCommandList.Reset();
	m_dxrStateObject.Reset();
	m_cameraCB.ResetResources();
	m_gameDataCB.ResetResources();
	m_sceneCB.ResetResources();
	m_lightCB.ResetResources();
	m_descriptorHeap.Reset();
	m_uselessRenderTarget.ResetResource();
	m_descriptorsAllocated = 0;
	m_imguiDescriptorsAllocated = 0;
	m_adapter.Reset();
	m_indexBuffer.ResetResource();
	m_vertexBuffer.ResetResource();
	m_bottomLevelAccelerationStructure.Reset();
	m_topLevelAccelerationStructure.Reset();
	m_dxgiFactory.Reset();

	RaygenShaderTable.ResetResources();
	MissShaderTable.ResetResources();
	HitShaderTable.ResetResources();

	m_depthStencilBuffer.ResetResource();
	m_RcommandList.Reset();
	m_RswapChain.Reset();
	m_Rdevice.Reset();
	m_RcommandQueue.Reset();

#ifdef ENGINE_DEBUG_RENDER
	((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;
	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif 
}
void RendererD12::AddPointLights(Vec4 position)
{
	int currentCounter = m_lightCB->Counter;
	if (currentCounter < m_lightCB->MaxLights - 1)
	{
		m_lightCB->Counter = currentCounter + 1;
		m_lightCB->PointLightPosition[currentCounter + 1] = position;
	}
}
void RendererD12::EndFrame()
{
	//return;
	//HRESULT result;
	//unsigned long long fenceToWaitFor;
	//ID3D12CommandList* ppCommandLists[1];
	//// Indicate that the back buffer will now be used to present.
	//m_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//m_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//m_commandList->ResourceBarrier(1, &m_barrier);
	////Once we are done our rendering list we close the command listand then submit it to the command queue to execute that list for us.

	//// Close the list of commands.
	//result = m_commandList->Close();
	////if (FAILED(result))
	////{
	////	ERROR_AND_DIE("Failed to close command list");
	////}

	//// Load the command list array (only one command list for now).
	//ppCommandLists[0] = m_commandList;

	//// Execute the list of commands.
	//m_commandQueue->ExecuteCommandLists(1, ppCommandLists);
	//m_swapChain->Present(1, 0);

	//fenceToWaitFor = m_fenceValue;
	//result = m_commandQueue->Signal(m_fencePointer, fenceToWaitFor);
	//if (FAILED(result))
	//{
	//	ERROR_AND_DIE("Failed while fetching fence")
	//}
	//m_fenceValue++;

	//// Wait until the GPU is done rendering.
	//if (m_fence->GetCompletedValue() < fenceToWaitFor)
	//{
	//	result = m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEventHandle);
	//	if (FAILED(result))
	//	{
	//		ERROR_AND_DIE("Failed while event completion in end frame")
	//	}
	//	WaitForSingleObject(m_fenceEventHandle, INFINITE);
	//}
	////For the next frame swap to the other back buffer using the alternating index.

	//// Alternate the back buffer index back and forth between 0 and 1 each frame.
	//m_frameIndex == 0 ? m_frameIndex = 1 : m_frameIndex = 0;
}


//----------------------------CAMERA FUNCTIONS-----------------------------
void RendererD12::BeginCamera(const Camera& camera)
{
	m_currentCamera = camera;

	Mat44 game = camera.GetViewToRenderMatrix();
	Mat44 model = Mat44();
	Mat44 proj = camera.GetProjectionMatrix();
	Mat44 view = camera.GetViewMatrix();
	Mat44 viewOrigin = camera.GetViewMatrixAtOrigin();
	Mat44 invProj, invView, invViewOrigin;
	if (!m_isFirstFrame)
	{
		m_sceneCB->_cameraPosition = m_sceneCB->cameraPosition;
		m_sceneCB->_viewMatrix = m_sceneCB->viewMatrix;
	}
	//------------GETTING INVERSE TO CONVERT FROM NDC TO WORLD--------------------
	invProj = proj.GetNonOrthonormalInverse();
	invView = view.GetNonOrthonormalInverse();
	invViewOrigin = viewOrigin.GetNonOrthonormalInverse();
	Vec3 ibasis, jbasis, kbasis;
	m_sceneCB->inversedProjectionMatrix = invProj;
	m_sceneCB->projectionMatrix = proj;
	m_sceneCB->viewMatrix = view;
	m_sceneCB->inversedViewMatrix = invView;
	m_sceneCB->inversedViewMatrixOrigin = invViewOrigin;
	m_sceneCB->cameraPosition = Vec4(camera.m_position, 1.0f);
	m_sceneCB->lightPosition = m_lightPosition;
	m_sceneCB->samplingData = m_gameValues.samplingData;
	m_sceneCB->GIColor = m_gameValues.GIColor;
	m_sceneCB->lightBools = m_gameValues.lightBools;
	m_sceneCB->lightfallOff_AmbientIntensity_CosineSampling = m_gameValues.lightfallOff_AmbientIntensity_CosineSampling;
	m_sceneCB->textureMappings = m_gameValues.textureMappings;
	m_isFirstFrame = false;

}
void RendererD12::BeginRasterizerCamera(const Camera& camera)
{
	m_currentCamera = camera;
	auto renderTarget = GetBackBufferCPUHandle();
	auto depthTarget = &m_depthStencilBuffer.cpuDescriptorHandle;
	m_RcommandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
	m_RcommandList->OMSetRenderTargets(1, renderTarget, FALSE, depthTarget);
	m_RcommandList->RSSetViewports(1, &m_screenViewport);
	m_RcommandList->RSSetScissorRects(1, &m_scissorRect);

	m_cameraCB->projectionMatrix = m_currentCamera.GetProjectionMatrix();
	m_cameraCB->viewMatrix = m_currentCamera.GetViewMatrix();

	Mat44 lightViewMatrix = m_shadowMap->m_shadowCamera.GetViewMatrix();
	Mat44 lightProjectionMatrix = m_shadowMap->m_shadowCamera.GetProjectionMatrix();
	m_cameraCB->lightViewMatrix = lightViewMatrix;
	m_cameraCB->lightProjMatrix = lightProjectionMatrix;

	m_cameraCB->cameraPosition = Vec4(m_currentCamera.m_position, 0.0f);
	m_gameDataCB->ViewX_GIOnY_ShadowPassZ.z = 0;
	m_cameraCB.CopyStagingToGpu(m_frameIndex);
	m_gameDataCB.CopyStagingToGpu(m_frameIndex);
}
void RendererD12::SetRaytraceQuadCamera(Vec3 topLeft, Vec3 bottomLeft, Vec3 topRight, Vec3 bottomRight)
{
	UNUSED((void)topLeft);
	UNUSED((void)bottomLeft);
	UNUSED((void)topRight);
	UNUSED((void)bottomRight);
	//	m_sceneCB[m_frameIndex].quadCornerPositions[0] = Vec4(topLeft);
	//	m_sceneCB[m_frameIndex].quadCornerPositions[1] = Vec4(bottomLeft);
	//	m_sceneCB[m_frameIndex].quadCornerPositions[2] = Vec4(topRight);
	//	m_sceneCB[m_frameIndex].quadCornerPositions[3] = Vec4(bottomRight);
}
void RendererD12::EndCamera(const Camera& camera)
{
	UNUSED((void)camera);
}

void RendererD12::D3D12InterfaceInitialization()
{
	m_windowDimensions = m_renderConfig.m_window->GetClientDimensions();
	m_dimensions = m_renderConfig.m_window->GetClientDimensions();
	//m_dimensions = IntVec2(1280, 720);
	//m_renderConfig.m_window->SetRenderTextureDimensions(m_dimensions);
	//m_dimensions = IntVec2(1920, 1080);
	InitializeAdapterAndCheckRaytracingSupport();

	//------------------CREATING THE DEVICE----------------------
	ThrowIfFailed(D3D12CreateDevice(m_adapter.Get(), m_d3dMinFeatureLevel, IID_PPV_ARGS(&m_Rdevice)), "Failed while Creating Device");

	//---------------COMMAND QUEUE-----------------
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(m_Rdevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_RcommandQueue)), "Failed While Creating Command Queue");


	//--------------RTV AND DSV DESCRIPTOR HEAPS----------------------------
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.NumDescriptors = m_backBufferCount;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ThrowIfFailed(m_Rdevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap)), "Failed while creating descriptor heap");
	m_RrtvDescriptorSize = m_Rdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
	dsvDescriptorHeapDesc.NumDescriptors = 10;
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	ThrowIfFailed(m_Rdevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&m_dsvDescriptorHeap)), "Failed while creating DSV HEAP");


	//----------------CREATING COMMAND ALLOCATOR--------------
	for (UINT n = 0; n < m_backBufferCount; n++)
	{
		ThrowIfFailed(m_Rdevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_RcommandAllocator[n])), "Failed while creating command allocator");
	}


	//----------------CREATING COMMAND LIST----------------------
	ThrowIfFailed(m_Rdevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_RcommandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_RcommandList)), "Failed while creating command list");
	ThrowIfFailed(m_Rdevice->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Failed while creating fence");
	m_fenceValues[m_frameIndex]++;

	m_fenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
	if (!m_fenceEvent.IsValid())
	{
		ThrowIfFailed(E_FAIL, "Failed while creating fence");
	}

	//// Wait until all previous GPU work is complete.
	/*WaitForGpu();*/

	for (UINT n = 0; n < m_backBufferCount; n++)
	{
		m_backBufferRenderTarget[n].Reset();
		m_fenceValues[n] = m_fenceValues[m_frameIndex];
	}
	DXGI_FORMAT backBufferFormat = NoSRGB(m_backBufferFormat);

	//----------------CREATING SWAP CHAIN-----------------------
	ComPtr<IDXGISwapChain1> swapChain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = m_dimensions.x;
	swapChainDesc.Height = m_dimensions.y;
	swapChainDesc.Format = backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = m_backBufferCount;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
	fsSwapChainDesc.Windowed = TRUE;
	ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
		m_RcommandQueue.Get(), (HWND)GetRenderConfig().m_window->GetHwnd(), &swapChainDesc, &fsSwapChainDesc,
		nullptr, &swapChain), "Failed While creating swap chain");

	ThrowIfFailed(swapChain.As(&m_RswapChain), "Failed While Creating Swap Chain");

	m_RswapChain->ResizeBuffers(
		m_backBufferCount,
		m_dimensions.x,
		m_dimensions.y,
		backBufferFormat,
		0
	);
	// This sample does not support fullscreen transitions.
	ThrowIfFailed(m_dxgiFactory->MakeWindowAssociation(HWND(GetRenderConfig().m_window->GetHwnd()), DXGI_MWA_NO_ALT_ENTER), "Failed While Making Full Screen Association");


	//-------------CREATING RENDER TARGETS----------------------
	for (UINT n = 0; n < m_backBufferCount; n++)
	{
		ThrowIfFailed(m_RswapChain->GetBuffer(n, IID_PPV_ARGS(&m_backBufferRenderTarget[n])), "Failed while getting render targets");

		wchar_t name[25] = {};
		swprintf_s(name, L"Render target %u", n);
		m_backBufferRenderTarget[n]->SetName(name);

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = m_backBufferFormat;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;


		m_backBufferHeapCPUHandle[n] = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), n, m_RrtvDescriptorSize);
		m_rtvDescriptorsAllocated++;
		m_Rdevice->CreateRenderTargetView(m_backBufferRenderTarget[n].Get(), &rtvDesc, m_backBufferHeapCPUHandle[n]);
		float colorfloats[4];
		Rgba8::BLACK.GetAsFloats(colorfloats);
		//m_RcommandList->ClearRenderTargetView(m_backBufferHeapCPUHandle[n], colorfloats, 0, NULL);
	}
	//m_rtvDescriptorsAllocated++;  // For Imgui
	//D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	//rtvDesc.Format = m_backBufferFormat;
	//rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//rtvDesc.Texture2D.MipSlice = 0;

	//// Create the dummy render target.
	//D3D12_CLEAR_VALUE clearValue = {};
	//clearValue.Format = m_backBufferFormat;
	//clearValue.DepthStencil.Depth = 1.0f;
	//clearValue.DepthStencil.Stencil = 0;

	//CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	//CD3DX12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(rtvDesc.Format, 1024, 1024, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	//HRESULT hr = m_Rdevice->CreateCommittedResource(&heapProps,
	//	D3D12_HEAP_FLAG_NONE,
	//	&texDesc,
	//	D3D12_RESOURCE_STATE_RENDER_TARGET,
	//	&clearValue,
	//	IID_PPV_ARGS(&m_uselessRenderTarget.resource));
	//
	/*if (m_renderingPipeline == RenderingPipeline::Rasterization)
	{
		UINT heapIndexRtv = UINT_MAX;
		AllocateRenderTargetDescriptor(&m_uselessRenderTarget.cpuDescriptorHandle, heapIndexRtv);
		m_Rdevice->CreateRenderTargetView(m_uselessRenderTarget.resource.Get(), &rtvDesc, m_uselessRenderTarget.cpuDescriptorHandle);
	}
	*/
	//---------------CREATING DEPTH STENCIL BUFFER AND VIEW-----------------------
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_dimensions.x;
	depthStencilDesc.Height = m_dimensions.y;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_CLEAR_VALUE depthClear;
	depthClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthClear.DepthStencil.Depth = 1;
	depthClear.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES resource = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(m_Rdevice->CreateCommittedResource(&resource, D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClear,
		IID_PPV_ARGS(&m_depthStencilBuffer.resource)
	), "Failed while getting render targets");


	D3D12_DEPTH_STENCIL_VIEW_DESC depthDesc = {};
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthDesc.Flags = D3D12_DSV_FLAG_NONE;
	m_depthStencilBuffer.m_UsageState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	UINT heapIndex = UINT_MAX;
	AllocateDepthDescriptor(&m_depthStencilBuffer.cpuDescriptorHandle, heapIndex);
	m_Rdevice->CreateDepthStencilView(m_depthStencilBuffer.GetResource(), &depthDesc, m_depthStencilBuffer.cpuDescriptorHandle);


	//------------------TRANSITION RESOURCE TO BE USED AS DEPTH BUFFER---------------
	CD3DX12_RESOURCE_BARRIER depthTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer.GetResource(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_RcommandList->ResourceBarrier(1, &depthTransitionBarrier);

	//----------------SETTING THE VIEWPORT-----------------------

	m_screenViewport.TopLeftX = 0.0f;
	m_screenViewport.TopLeftY = 0.0f;
	m_screenViewport.Width = static_cast<float>(m_dimensions.x);
	m_screenViewport.Height = static_cast<float>(m_dimensions.y);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;
	m_RcommandList->RSSetViewports(1, &m_screenViewport);
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_dimensions.x), static_cast<LONG>(m_dimensions.y));

	//Closing the command list here after some initial commands
	ThrowIfFailed(m_RcommandList->Close(), "Failed while closing command list");

	m_frameIndex = m_RswapChain->GetCurrentBackBufferIndex();
	m_shaderCompiler = new ShaderCompiler();

	//----------------CREATING RAYTRACING DESCRIPTOR HEAP---------------
	D3D12_DESCRIPTOR_HEAP_DESC heapdesc = {};
	heapdesc.NumDescriptors = 1000;
	heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapdesc.NodeMask = 0;
	ThrowIfFailed(m_Rdevice->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&m_imguiDescriptorHeap)), "Failed While Creating Descriptor Heap");
	heapdesc.NumDescriptors = 100;
	ThrowIfFailed(m_Rdevice->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&m_descriptorHeap)), "Failed While Creating Descriptor Heap");

	m_descriptorSize = m_Rdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_depthDescriptorSize = m_Rdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	//----------------CREATING RESOURCE MANAGER---------------------
	m_resourceManager = new ResourceManager();
	m_resourceManager->Bind(m_RcommandList.Get());

}
void RendererD12::InitializeRasterization()
{
	//----------Creating camera Constant buffer---------------
	m_cameraCB.Create(m_Rdevice.Get(), m_backBufferCount, L"Camera Constant Buffer");
	m_gameDataCB.Create(m_Rdevice.Get(), m_backBufferCount, L"Game Constant Buffer");

	m_shadowMap = new ShadowMap(this, m_Rdevice.Get(), m_dimensions.x, m_dimensions.y);
}

void RendererD12::BeginShadowMapRender()
{
	//D3D12_CPU_DESCRIPTOR_HANDLE* depthDescriptorHandle = &m_shadowMap->m_shadowMapDSVBuffer.cpuDescriptorHandle;
	auto depthTarget = &m_depthStencilBuffer.cpuDescriptorHandle;
	m_RcommandList->OMSetRenderTargets(0, nullptr, FALSE, depthTarget);

	m_currentCamera = m_shadowMap->m_shadowCamera;
	m_currentCamera.SetTransform(m_shadowMap->m_shadowCamera.m_position, m_shadowMap->m_shadowCamera.m_orientation);
	m_RcommandList->RSSetViewports(1, &m_screenViewport);
	m_RcommandList->RSSetScissorRects(1, &m_scissorRect);

	m_cameraCB->projectionMatrix = m_currentCamera.GetProjectionMatrix();
	m_cameraCB->viewMatrix = m_currentCamera.GetViewMatrix();
	m_cameraCB->cameraPosition = Vec4(m_currentCamera.m_position, 0.0f);
	m_gameDataCB->ViewX_GIOnY_ShadowPassZ.z = 1;

	//m_gameDataCB->globalLightPosition = 
	m_cameraCB.CopyStagingToGpu(m_frameIndex);
	m_gameDataCB.CopyStagingToGpu(m_frameIndex);

	m_fenceValues[m_frameIndex] = m_fenceValues[m_frameIndex] + 1;
}
void RendererD12::EndShadowMapRender()
{
	FinishUpGPUWork();
	CopyTextureResourceFromBuffer(&m_depthStencilBuffer, &m_shadowMap->m_shadowBuffer, m_shadowMap->GetDimensions());

	D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
	m_RcommandList.Get()->ClearDepthStencilView(m_depthStencilBuffer.cpuDescriptorHandle, flags, 1, 0, 0, nullptr);
}
void RendererD12::FinishUpGPUWork()
{
	ExecuteCommandList(m_RcommandList);
	WaitForGpu();
	m_RcommandList->Reset(m_RcommandAllocator[m_frameIndex].Get(), nullptr);
}
//void RendererD12::SetRasterizationState(CullModeD12 cullMode, FillModeD12 fillMode,WindingOrderD12 windingOrder )
//{
//	switch (fillMode)
//	{
//	case FillModeD12::SOLID:
//		m_rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
//		break;
//	case FillModeD12::WIREFRAME:
//		m_rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
//		break;
//	default:
//		break;
//	}
//	switch (cullMode)
//	{
//	case CullModeD12::NONE:
//		m_rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
//		break;
//	case CullModeD12::FRONT:
//		m_rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
//		break;
//	case CullModeD12::BACK:
//		m_rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
//		break;
//	default:
//		break;
//	}
//	switch (windingOrder)
//	{
//	case WindingOrderD12::CLOCKWISE:
//		m_rasterizerDesc.FrontCounterClockwise= false;
//		break;
//	case WindingOrderD12::COUNTERCLOCKWISE:
//		m_rasterizerDesc.FrontCounterClockwise = true;
//		break;
//	default:
//		break;
//	}
//}
void RendererD12::CreateRaytracingInterfaces()
{
	auto device = m_Rdevice.Get();
	auto commandList = m_RcommandList.Get();

	ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice)), "Couldn't get DirectX Raytracing interface for the device.\n");
	ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), "Couldn't get DirectX Raytracing interface for the command list.\n");


	//m_indexBuffer.resize(3);
	//m_vertexBuffer.resize(3);
	//m_bottomLevelAccelerationStructure.resize(3);
	//m_topLevelAccelerationStructure.resize(3);
}
void RendererD12::CompileShaders()
{


}
void RendererD12::CreateRaytracingPipelineStateObject()
{
	ComPtr<IDxcBlob> mainShaderBlob;
	CD3DX12_SHADER_BYTECODE mainRaytracer;
	mainShaderBlob = m_shaderCompiler->Compile("Data/Shaders/Raytracing3D.hlsl");
	mainRaytracer.BytecodeLength = mainShaderBlob->GetBufferSize();
	mainRaytracer.pShaderBytecode = mainShaderBlob->GetBufferPointer();

	//CREATING PSO
	CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

	auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	lib->SetDXILLibrary(&mainRaytracer);
	lib->DefineExport(c_raygenShaderName);
	lib->DefineExport(c_closestHitShaderName);
	lib->DefineExport(c_missShaderName);

	auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
	hitGroup->SetClosestHitShaderImport(c_closestHitShaderName);
	hitGroup->SetHitGroupExport(c_hitGroupName);
	hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

	auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = sizeof(RaytracerPayload);
	UINT attributeSize = 2 * sizeof(Vec2);
	shaderConfig->Config(payloadSize, attributeSize);

	// Local root signature and shader association
	/*auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	localRootSignature->SetRootSignature(m_raytracingLocalRootSignature.Get());

	auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
	rootSignatureAssociation->AddExport(c_hitGroupName);*/
	//rootSignatureAssociation->AddExport(c_closestHitShaderName);

	auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());

	auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	UINT maxRecursionDepth = 30;
	pipelineConfig->Config(maxRecursionDepth);

#ifdef ENGINE_DEBUG_RENDER
	PrintStateObjectDesc(raytracingPipeline);
#endif
	// Create the state object.
	ThrowIfFailed(m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), "Couldn't create DirectX Raytracing state object.\n");

	auto device = m_Rdevice.Get();
	auto frameCount = m_backBufferCount;

	m_sceneCB.Create(device, frameCount, L"Scene Constant buffer");
	m_lightCB.Create(device, frameCount, L"Light Constant buffer");
}
void RendererD12::InitializeBasicBottomLevelAS()
{
	/*int maxBLAS = 1000;
	auto* device = m_dxrDevice.Get();
	m_raytracingASManager = new RaytracingASManager(device, maxBLAS, m_frameIndex);

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	for (auto& geom : m_geomtries)
	{
		m_raytracingASManager->AddBottomLevelAS(device, buildFlags, geom, false, false);
	}*/
}
//void RendererD12::InitializeSampler()
//{
//	//UINT maxPixelsInSampleSet1D = 8;
//	UINT maxSamplesPerSet = 65536;
//	UINT numSampleSets = 83;
//	auto device = m_Rdevice.Get();
//	m_sampler = new Sampler(SamplerType::Cosine, 64, numSampleSets);
//
//	m_hemisphereSamplesGPUBuffer.Create(device, maxSamplesPerSet * numSampleSets,m_backBufferCount, L"GPU buffer: Random hemisphere samples");
//	UINT numSamples = m_sampler->m_numSamples * m_sampler->m_numSampleSets;
//	for (UINT i = 0; i < numSamples; i++)
//	{
//		UINT sampleindex = m_sampler->GetSampleIndex();
//		Vec3 sampleDataPoint = m_sampler->m_hemisphereSampleData[sampleindex];
//		// Convert [-1,1] to [0,1].
//		//m_samplesGPUBuffer[i].value = XMFLOAT2(p.x * 0.5f + 0.5f, p.y * 0.5f + 0.5f);
//		m_hemisphereSamplesGPUBuffer[i].sampleValue = sampleDataPoint;
//	}
//}
void RendererD12::InitializeDenoising()
{
	m_denoiser = new Denoiser();
	m_denoiser->InitializeDenoiser(m_dxrDevice.Get(), m_backBufferCount);
}
void RendererD12::InitializeGlobalIllumination()
{
	m_globalIllumination = new GlobalIllumination();
	m_globalIllumination->Initialize(m_dxrDevice.Get(), m_backBufferCount);
}
void RendererD12::InitializeIrradianceCaching()
{
	//UINT maxPixelsInSampleSet1D = 8;
	UINT maxSamplesPerSet = 65536;
	UINT numSampleSets = 83;
	auto device = m_Rdevice.Get();

	m_irradianceCacheGPUBuffer.Create(device, maxSamplesPerSet * numSampleSets, m_backBufferCount, L"GPU buffer: Irradiance Cache");
}

//--------------------------------------ACCELERATION STRUCTURE BUILDS-----------------------------
void RendererD12::BuildGeometryAndAS(std::vector<Vertex_PCUTBN>& verts, std::vector<UINT>& indices, int index)
{
	UNUSED((void)index);
	WaitForGpu();
	//m_bottomLevelAccelerationStructure.Reset();
	//m_topLevelAccelerationStructure.Reset();

	//----------------CREATING UPLOAD BUFFER FOR VERTS AND INDICES-------------
	auto device = m_Rdevice.Get();
	AllocateUploadBuffer(device, indices.data(), indices.size() * sizeof(UINT), &m_indexBuffer.resource, nullptr);
	AllocateUploadBuffer(device, verts.data(), verts.size() * sizeof(Vertex_PCUTBN), &m_vertexBuffer.resource, nullptr);

	UINT descriptorIndexIB = CreateBufferSRV(&m_indexBuffer, (UINT)(indices.size()), 0, DXGI_FORMAT_R32_TYPELESS, D3D12_BUFFER_SRV_FLAG_RAW);
	UINT descriptorIndexVB = CreateBufferSRV(&m_vertexBuffer, ((UINT)verts.size()), (UINT)sizeof(Vertex_PCUTBN), DXGI_FORMAT_UNKNOWN, D3D12_BUFFER_SRV_FLAG_NONE);

	if (descriptorIndexVB != descriptorIndexIB + 1) { ERROR_AND_DIE("Vertex Buffer descriptor index must follow that of Index Buffer descriptor index!"); }

	//-----------------BUILDING THE ACTUAL STRUCTURE---------------------
	auto commandList = m_RcommandList.Get();
	//auto commandQueue = m_RcommandQueue.Get();
	auto commandAllocator = m_RcommandAllocator[m_frameIndex].Get();

	// Reset the command list for the acceleration structure construction.
	commandList->Reset(commandAllocator, nullptr);


	//---------------------------------------GEOMETRY DESCRIPTION BUILD-----------------------------
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = m_indexBuffer.resource->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = static_cast<unsigned int>(m_indexBuffer.resource->GetDesc().Width) / sizeof(unsigned int);
	geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexCount = static_cast<unsigned int>(m_vertexBuffer.resource->GetDesc().Width) / sizeof(Vertex_PCUTBN);
	geometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer.resource->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex_PCUTBN);
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	//------------------------------------BOTTOM LEVEL BUILD-----------------------
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS bottomLevelbuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

	//topLevelbuildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	bottomLevelInputs.Flags = bottomLevelbuildFlags;
	bottomLevelInputs.NumDescs = 1;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.pGeometryDescs = &geometryDesc;


	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	ThrowIfFalse(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0, "");

	ComPtr<ID3D12Resource> scratchResource;
	AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes, &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");
	{
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructure, initialResourceState, L"BottomLevelAccelerationStructure");
		// Bottom Level Acceleration Structure desc

		bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
		bottomLevelBuildDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
	}

	auto* raytracingCommandList = m_dxrCommandList.Get();

	CD3DX12_RESOURCE_BARRIER bar = CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructure.Get());
	raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
	commandList->ResourceBarrier(1, &bar);


	//-------------------------TOP LEVEL----------------------------------
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS topLevelbuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topLevelInputs = topLevelBuildDesc.Inputs;
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = topLevelbuildFlags;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.pGeometryDescs = nullptr;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0, "");


	{
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure, initialResourceState, L"TopLevelAccelerationStructure");
	}

	// Create an instance desc for the bottom-level acceleration structure.
	ComPtr<ID3D12Resource> instanceDescs;
	D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
	instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
	instanceDesc.InstanceMask = 1;
	instanceDesc.AccelerationStructure = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
	AllocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");


	// Top Level Acceleration Structure desc
	{
		topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
		topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
		topLevelBuildDesc.Inputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
	}
	raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	// Kick off acceleration structure construction.
	ExecuteCommandList(m_RcommandList);

	// Wait for GPU to finish 
	WaitForGpu();
}

void RendererD12::BuildModelGeometryAndAS(std::vector<Vertex_PCUTBN>& verts, std::vector<UINT>& indices, int index)
{
	UNUSED((void)index);
	WaitForGpu();
	//m_bottomLevelAccelerationStructure.Reset();
	//m_topLevelAccelerationStructure.Reset();

	//----------------CREATING UPLOAD BUFFER FOR VERTS AND INDICES-------------
	auto device = m_Rdevice.Get();
	AllocateUploadBuffer(device, indices.data(), indices.size() * sizeof(UINT), &m_bunnyIndexBuffer.resource, nullptr);
	AllocateUploadBuffer(device, verts.data(), verts.size() * sizeof(Vertex_PCUTBN), &m_bunnyVertexBuffer.resource, nullptr);

	UINT descriptorIndexIB = CreateBufferSRV(&m_bunnyIndexBuffer, (UINT)(indices.size()), 0, DXGI_FORMAT_R32_TYPELESS, D3D12_BUFFER_SRV_FLAG_RAW);
	UINT descriptorIndexVB = CreateBufferSRV(&m_bunnyVertexBuffer, ((UINT)verts.size()), (UINT)sizeof(Vertex_PCUTBN), DXGI_FORMAT_UNKNOWN, D3D12_BUFFER_SRV_FLAG_NONE);

	if (descriptorIndexVB != descriptorIndexIB + 1) { ERROR_AND_DIE("Vertex Buffer descriptor index must follow that of Index Buffer descriptor index!"); }

	//-----------------BUILDING THE ACTUAL STRUCTURE---------------------
	auto commandList = m_RcommandList.Get();
	//auto commandQueue = m_RcommandQueue.Get();
	auto commandAllocator = m_RcommandAllocator[m_frameIndex].Get();

	// Reset the command list for the acceleration structure construction.
	commandList->Reset(commandAllocator, nullptr);


	//---------------------------------------GEOMETRY DESCRIPTION BUILD-----------------------------
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = m_bunnyIndexBuffer.resource->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = static_cast<UINT>(m_bunnyIndexBuffer.resource->GetDesc().Width) / sizeof(UINT);
	geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexCount = static_cast<UINT>(m_bunnyVertexBuffer.resource->GetDesc().Width) / sizeof(Vertex_PCUTBN);
	geometryDesc.Triangles.VertexBuffer.StartAddress = m_bunnyVertexBuffer.resource->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex_PCUTBN);
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	//------------------------------------BOTTOM LEVEL BUILD-----------------------
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS bottomLevelbuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

	//topLevelbuildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	bottomLevelInputs.Flags = bottomLevelbuildFlags;
	bottomLevelInputs.NumDescs = 1;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.pGeometryDescs = &geometryDesc;


	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	ThrowIfFalse(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0, "");

	ComPtr<ID3D12Resource> scratchResource;
	AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes, &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");
	{
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_bunnyBlas, initialResourceState, L"Bunny BottomLevelAccelerationStructure");
		// Bottom Level Acceleration Structure desc

		bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
		bottomLevelBuildDesc.DestAccelerationStructureData = m_bunnyBlas->GetGPUVirtualAddress();
	}

	auto* raytracingCommandList = m_dxrCommandList.Get();

	CD3DX12_RESOURCE_BARRIER bar = CD3DX12_RESOURCE_BARRIER::UAV(m_bunnyBlas.Get());
	raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
	commandList->ResourceBarrier(1, &bar);


	//-------------------------TOP LEVEL----------------------------------
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS topLevelbuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topLevelInputs = topLevelBuildDesc.Inputs;
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = topLevelbuildFlags;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.pGeometryDescs = nullptr;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0, "");
	{
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_bunnyTlas, initialResourceState, L"Bunny TopLevelAccelerationStructure");
	}

	// Create an instance desc for the bottom-level acceleration structure.
	ComPtr<ID3D12Resource> instanceDescs;
	D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
	instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
	instanceDesc.InstanceMask = 1;
	instanceDesc.AccelerationStructure = m_bunnyBlas->GetGPUVirtualAddress();
	AllocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");


	// Top Level Acceleration Structure desc
	{
		topLevelBuildDesc.DestAccelerationStructureData = m_bunnyTlas->GetGPUVirtualAddress();
		topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
		topLevelBuildDesc.Inputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
	}
	raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	// Kick off acceleration structure construction.
	ExecuteCommandList(m_RcommandList);

	// Wait for GPU to finish 
	WaitForGpu();
}
void RendererD12::BuildBLAS(std::vector<Vertex_PCUTBN>& verts, std::vector<UINT>& indices, int index)
{

	//m_bottomLevelAccelerationStructure.Reset();
	//m_topLevelAccelerationStructure.Reset();
	//----------------CREATING UPLOAD BUFFER FOR VERTS AND INDICES-------------
	GpuBuffer indexDataBuffer;
	GpuBuffer vertexDataBuffer;
	auto device = m_Rdevice.Get();
	AllocateUploadBuffer(device, indices.data(), indices.size() * sizeof(UINT), &indexDataBuffer.resource, nullptr);
	AllocateUploadBuffer(device, verts.data(), verts.size() * sizeof(Vertex_PCUTBN), &vertexDataBuffer.resource, nullptr);
	UINT descriptorIndexIB = CreateBufferSRV(&indexDataBuffer, (UINT)(indices.size()), 0, DXGI_FORMAT_R32_TYPELESS, D3D12_BUFFER_SRV_FLAG_RAW);
	UINT descriptorIndexVB = CreateBufferSRV(&vertexDataBuffer, ((UINT)verts.size()), (UINT)sizeof(Vertex_PCUTBN), DXGI_FORMAT_UNKNOWN, D3D12_BUFFER_SRV_FLAG_NONE);
	if (descriptorIndexVB != descriptorIndexIB + 1) { ERROR_AND_DIE("Vertex Buffer descriptor index must follow that of Index Buffer descriptor index!"); }
	//-----------------BUILDING THE ACTUAL STRUCTURE---------------------
	auto commandList = m_RcommandList.Get();
	//auto commandQueue = m_RcommandQueue.Get();
	auto commandAllocator = m_RcommandAllocator[m_frameIndex].Get();
	// Reset the command list for the acceleration structure construction.
	commandList->Reset(commandAllocator, nullptr);

	//---------------------------------------GEOMETRY DESCRIPTION BUILD-----------------------------
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = indexDataBuffer.resource->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = static_cast<UINT>(indexDataBuffer.resource->GetDesc().Width) / sizeof(UINT);
	geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexCount = static_cast<UINT>(vertexDataBuffer.resource->GetDesc().Width) / sizeof(Vertex_PCUTBN);
	geometryDesc.Triangles.VertexBuffer.StartAddress = vertexDataBuffer.resource->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex_PCUTBN);
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	LocalRootArgumentsGeometry rs = {};
	UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	memcpy(&rs.m_indexBufferGPUHandle, &indexDataBuffer.gpuReadDescriptorHandle, sizeof(indexDataBuffer.gpuReadDescriptorHandle));
	memcpy(&rs.m_vertexBufferGPUHandle, &vertexDataBuffer.gpuReadDescriptorHandle, sizeof(vertexDataBuffer.gpuReadDescriptorHandle));
	//UINT shaderRecordSize = shaderIdentifierSize + sizeof(rs);
	ShaderRecord chunkRecord = ShaderRecord(hitGroupShaderID, shaderIdentifierSize, &rs, sizeof(rs));
	HitShaderTable.AddShaderRecordAtIndex(chunkRecord, index);

	//-------------------------------------BOTTOM LEVEL BUILD----------------------------
	AccelerationStructureBuffers& bottomLevelASBuffers = m_bottomLevelAccelerationStructureBuffers[index];
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS bottomLevelbuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	//topLevelbuildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	bottomLevelInputs.Flags = bottomLevelbuildFlags;
	bottomLevelInputs.NumDescs = 1;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.pGeometryDescs = &geometryDesc;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	ThrowIfFalse(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0, "");
	ComPtr<ID3D12Resource> scratchResource;
	//ComPtr<ID3D12Resource> blasResource;
	AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes, &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");
	{
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &bottomLevelASBuffers.accelerationStructure, initialResourceState, L"BottomLevelAccelerationStructure");
		// Bottom Level Acceleration Structure desc
		bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
		bottomLevelBuildDesc.DestAccelerationStructureData = bottomLevelASBuffers.accelerationStructure->GetGPUVirtualAddress();
	}
	auto* raytracingCommandList = m_dxrCommandList.Get();
	CD3DX12_RESOURCE_BARRIER bar = CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelASBuffers.accelerationStructure.Get());
	raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
	commandList->ResourceBarrier(1, &bar);

	ExecuteCommandList(m_RcommandList);
	WaitForGpu();

	//commandList->CopyResource(bottomLevelASBuffers.accelerationStructure.Get(), blasResource.Get());
	//commandList->CopyResource(bottomLevelASBuffers.scratch.Get(), scratchResource.Get());
	//bottomLevelASBuffers.accelerationStructure = blasResource;
	bottomLevelASBuffers.scratch = scratchResource;
	bottomLevelASBuffers.ResultDataMaxSizeInBytes = bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes;
	bottomLevelASBuffers.isValid = true;
	m_isTopLevelASRebuildRequired = true;


}
void RendererD12::BuildTLAS()
{
	auto device = m_Rdevice.Get();
	auto commandList = m_RcommandList.Get();
	auto commandAllocator = m_RcommandAllocator[m_frameIndex].Get();
	commandList->Reset(commandAllocator, nullptr);
	ComPtr<ID3D12Resource> scratch;

	// Get required sizes for an acceleration structure.
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topLevelInputs = topLevelBuildDesc.Inputs;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	topLevelInputs.NumDescs = (UINT)HitShaderTable.m_shaderRecords.size();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0, "");

	AllocateUAVBuffer(device, topLevelPrebuildInfo.ScratchDataSizeInBytes, &scratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

	// Allocate resources for acceleration structures.
	// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
	// Default heap is OK since the application doesn’t need CPU read/write access to them. 
	// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
	// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
	//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
	//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
	{
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure, initialResourceState, L"TopLevelAccelerationStructure");
	}


	BuildASInstances();
	m_bottomLevelASInstanceDescs.CopyFromCPUToGPU(m_frameIndex);
	topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
	topLevelInputs.InstanceDescs = m_bottomLevelASInstanceDescs.GpuVirtualAddress(m_frameIndex);
	topLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
	//D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT
	m_dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	ExecuteCommandList(m_RcommandList);
	WaitForGpu();
	m_isTopLevelASRebuildRequired = false;

}
void RendererD12::BuildASInstances()
{
	//	ThrowIfFalse(m_numBottomLevelASInstances < m_bottomLevelASInstanceDescs.NumElements(), L"Not enough instance desc buffer size.");
		//UINT64 bufferSize = 0;
	for (int i = 0; i < MINECRAFTCHUNKS; i++)
	{
		if (m_bottomLevelAccelerationStructureBuffers[i].isValid)
		{
			D3D12_RAYTRACING_INSTANCE_DESC& instanceDesc = m_bottomLevelASInstanceDescs[i];
			instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
			instanceDesc.InstanceMask = 1;
			instanceDesc.AccelerationStructure = m_bottomLevelAccelerationStructureBuffers[i].accelerationStructure->GetGPUVirtualAddress();
			instanceDesc.InstanceContributionToHitGroupIndex = i;
			//bufferSize += static_cast <UINT64>(sizeof(instanceDesc));
		}
	}

	//bufferSize = static_cast<UINT64>(instanceDescriptions.size() * sizeof(instanceDescriptions[0]));
	//AllocateUploadBuffer(device, m_bottomLevelASInstanceDescs.data(), bufferSize, &(*instanceDescsResource), L"InstanceDescs");
}

void RendererD12::BuildAS(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomASDesc, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topASDesc, ID3D12Resource* bottomLevelAS)
{
	auto commandList = m_RcommandList.Get();
	auto* raytracingCommandList = m_dxrCommandList.Get();
	CD3DX12_RESOURCE_BARRIER bar = CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAS);
	raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomASDesc, 0, nullptr);
	commandList->ResourceBarrier(1, &bar);
	raytracingCommandList->BuildRaytracingAccelerationStructure(&topASDesc, 0, nullptr);
}
void RendererD12::BuildGeometryAndASForChunk(std::vector<Vertex_PCUTBN>& verts, std::vector<UINT>& indices,
	ComPtr<ID3D12Resource>& topLevelAS, ComPtr<ID3D12Resource>& bottomLevelAS,
	GpuBuffer& vertexBuffer, GpuBuffer& indexBuffer, int instanceIndex)
{
	UNUSED((void)instanceIndex);
	//----------------CREATING UPLOAD BUFFER FOR VERTS AND INDICES-------------
	auto device = m_Rdevice.Get();
	AllocateUploadBuffer(device, indices.data(), indices.size() * sizeof(UINT), &indexBuffer.resource, nullptr);
	AllocateUploadBuffer(device, verts.data(), verts.size() * sizeof(Vertex_PCUTBN), &vertexBuffer.resource, nullptr);

	UINT descriptorIndexIB = CreateBufferSRV(&indexBuffer, (UINT)(indices.size()), 0, DXGI_FORMAT_R32_TYPELESS, D3D12_BUFFER_SRV_FLAG_RAW);
	UINT descriptorIndexVB = CreateBufferSRV(&vertexBuffer, ((UINT)verts.size()), (UINT)sizeof(Vertex_PCUTBN), DXGI_FORMAT_UNKNOWN, D3D12_BUFFER_SRV_FLAG_NONE);

	if (descriptorIndexVB != descriptorIndexIB + 1) { ERROR_AND_DIE("Vertex Buffer descriptor index must follow that of Index Buffer descriptor index!"); }

	//-----------------BUILDING THE ACTUAL STRUCTURE---------------------
	auto commandList = m_RcommandList.Get();
	//auto commandQueue = m_RcommandQueue.Get();
	auto commandAllocator = m_RcommandAllocator[m_frameIndex].Get();

	// Reset the command list for the acceleration structure construction.
	commandList->Reset(commandAllocator, nullptr);

	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = indexBuffer.resource->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = static_cast<UINT>(indexBuffer.resource->GetDesc().Width) / sizeof(UINT);
	geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexCount = static_cast<UINT>(vertexBuffer.resource->GetDesc().Width) / sizeof(Vertex_PCUTBN);
	geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer.resource->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex_PCUTBN);
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;


	//--------------------BUILD TOP LEVEL INFO--------------------------------------
	// Get required sizes for an acceleration structure.
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	bottomLevelInputs.Flags = buildFlags;
	bottomLevelInputs.NumDescs = 1;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.pGeometryDescs = &geometryDesc;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topLevelInputs = topLevelBuildDesc.Inputs;
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = buildFlags;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.pGeometryDescs = nullptr;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0, "");

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	ThrowIfFalse(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0, "");

	ComPtr<ID3D12Resource> scratchResource;
	AllocateUAVBuffer(device, max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

	{
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

		AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &bottomLevelAS, initialResourceState, L"BottomLevelAccelerationStructure");
		AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &topLevelAS, initialResourceState, L"TopLevelAccelerationStructure");
	}

	// Create an instance desc for the bottom-level acceleration structure.
	ComPtr<ID3D12Resource> instanceDescs;
	D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
	instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
	instanceDesc.InstanceMask = 1;
	instanceDesc.AccelerationStructure = bottomLevelAS->GetGPUVirtualAddress();
	AllocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");

	// Bottom Level Acceleration Structure desc
	{
		bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
		bottomLevelBuildDesc.DestAccelerationStructureData = bottomLevelAS->GetGPUVirtualAddress();
	}

	// Top Level Acceleration Structure desc
	{
		topLevelBuildDesc.DestAccelerationStructureData = topLevelAS->GetGPUVirtualAddress();
		topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
		topLevelBuildDesc.Inputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
	}

	BuildAS(bottomLevelBuildDesc, topLevelBuildDesc, bottomLevelAS.Get());

	// Kick off acceleration structure construction.
	ExecuteCommandList(m_RcommandList);

	// Wait for GPU to finish 
	WaitForGpu();

	//After building add shader record to Raygen and HitGroup Shader Tables
	//LocalRootArgumentsGeometry rs = {};
	//UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	//rs.indexBufferGPUHandle = indexBuffer.gpuDescriptorHandle;
	//rs.vertexBufferGPUHandle = vertexBuffer.gpuDescriptorHandle;
	//UINT shaderRecordSize = shaderIdentifierSize + sizeof(rs);
	//ShaderRecord chunkRecord = ShaderRecord(hitGroupShaderID, shaderIdentifierSize, &rs, sizeof(rs));
	//HitShaderTable.AddShaderRecordAtIndex(chunkRecord, instanceIndex);


}
//------------------------------------SHADER TABLES AND OUTPUT RESOURCES----------------------------
void RendererD12::BuildShaderTables()
{
	auto device = m_Rdevice.Get();
	ComPtr<ID3D12StateObjectProperties> stateObjectProps;
	ThrowIfFailed(m_dxrStateObject.As(&stateObjectProps), "Failed to create properties for shader table");
	rayGenShaderID = stateObjectProps->GetShaderIdentifier(c_raygenShaderName);
	missShaderID = stateObjectProps->GetShaderIdentifier(c_missShaderName);
	hitGroupShaderID = stateObjectProps->GetShaderIdentifier(c_hitGroupName);

	UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	UINT numShaderRecords = 1;
	UINT shaderRecordSize = shaderIdentifierSize;

	// Ray gen shader table
	{
		shaderRecordSize = shaderIdentifierSize;
		RaygenShaderTable = ShaderTable(device, numShaderRecords, shaderRecordSize, L"Raytracer RayGenShaderTable");
		RaygenShaderTable.push_back(ShaderRecord(rayGenShaderID, shaderIdentifierSize, nullptr, 0));
		m_raygenGroupShaderTableStrideInBytes = RaygenShaderTable.GetShaderRecordSize();
	}

	// Miss shader table
	{
		MissShaderTable = ShaderTable(device, numShaderRecords, shaderRecordSize, L"Raytracer MissShaderTable");
		MissShaderTable.push_back(ShaderRecord(missShaderID, shaderIdentifierSize, nullptr, 0));
	}

	// Hit group shader table
	{
		LocalRootArgumentsGeometry rs = {};
		//numShaderRecords = MINECRAFTCHUNKS;
		//shaderRecordSize = shaderIdentifierSize + sizeof(LocalRootArgumentsGeometry);
		HitShaderTable = ShaderTable(device, numShaderRecords, shaderRecordSize, L"Raytracer HitGroupShaderTable");
		HitShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIdentifierSize));
		//for (int i = 0; i < MINECRAFTCHUNKS; i++)
		//{
		//	HitShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIdentifierSize, &rs, sizeof(rs)));
		//}

		m_hitGroupShaderTableStrideInBytes = HitShaderTable.GetShaderRecordSize();
	}

	//m_bottomLevelASInstanceDescs.Create(device, MINECRAFTCHUNKS, m_backBufferCount, L"Bottom-Level Acceleration Structure Instance descs.");
}
void RendererD12::CreateRaytracingOutputResources()
{
	// Create the output resource. The dimensions and format should match the swap-chain.
	IntVec2 gbufferDimensions = m_dimensions;
	D3D12_RESOURCE_STATES startingState = D3D12_RESOURCE_STATE_COMMON;
	m_resourceManager->CreateGBufferResource(m_backBufferFormat, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::OutputResource, L"Output Resource");
	m_resourceManager->CreateGBufferResource(m_backBufferFormat, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::CompositorOutput, L"Compistor Output Resource");
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R32G32B32A32_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::VertexPosition, L"VertexPosition Resource");
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R32G32B32A32_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::VertexNormal, L"Vertex normal Resource");
	m_resourceManager->CreateGBufferResource(m_backBufferFormat, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::VertexAlbedo, L"Vertex Albedo Resource");
	m_resourceManager->CreateGBufferResource(m_backBufferFormat, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::VertexIndirectAlbedo, L"Vertex Indirect Albedo Resource");
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R32G32_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::MotionVector, L"Motion Vector Resource");
	m_resourceManager->CreateGBufferResource(m_backBufferFormat, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::GI, L"Global Illumination  Resource");
	m_resourceManager->CreateGBufferResource(m_backBufferFormat, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::DirectLight, L"Direct Light Resource");
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R32_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::Depth, L"Depth Resource");
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R16_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::HistoryLength, L"History Length Resource");
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R16G16_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::Moments, L"Moments Resource");
	m_resourceManager->CreateGBufferResource(m_backBufferFormat, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::DenoiserInput, L"Denoiser output Resource");
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R16G16_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::PartialDerivates, L" Partial Derivatives Resource");
	m_resourceManager->CreateGBufferResource(m_backBufferFormat, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::OcclusionTexture, L" Occlusion Texture Resource");
	//m_resourceManager->CreateGBufferResource(m_backBufferFormat, m_windowDimensions, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::Count);

	//PREVIOUS FRAME RESOURCES
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R32G32B32A32_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResourcesPreviousFrame::VertexNormal, L"PreviousFrame Normal resource", true);
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R32_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResourcesPreviousFrame::Depth, L"PreviousFrame Depth resource", true);
	m_resourceManager->CreateGBufferResource(m_backBufferFormat, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResourcesPreviousFrame::GI, L"PreviousFrame GI resource", true);
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R16G16_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResourcesPreviousFrame::Moments, L"PreviousFrame Moments resource", true);
	m_resourceManager->CreateGBufferResource(DXGI_FORMAT_R16_FLOAT, gbufferDimensions, startingState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResourcesPreviousFrame::History, L"PreviousFrame Moments resource", true);

	//DENOISER RESOURCE
	m_resourceManager->CreateDenoiserGBufferResource(m_backBufferFormat, gbufferDimensions, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, L"Denoiser Resource");
}

//-----------------------------COMPOSITION---------------------
void RendererD12::InitializePostProcess()
{
	m_postProcess = new PostProcess();
	m_postProcess->InitializeGodRays(m_dxrDevice.Get(), m_backBufferCount);
}
void RendererD12::InitializeComposition()
{
	m_compositor = new Composition();
	m_compositor->Initialize(m_dxrDevice.Get(), m_backBufferCount);
}
void RendererD12::InitializeAdapterAndCheckRaytracingSupport()
{
	bool debugDXGI = false;

#if defined(ENGINE_DEBUG_RENDER)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
		else
		{
			OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
		}

		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
		{
			debugDXGI = true;

			ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_dxgiFactory)), "Failed while creating factory");

			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
		}
	}
#endif

	if (!debugDXGI)
	{
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)), "Failed while trying to create factory");
	}
	IDXGIAdapter1** ppAdapter = &m_adapter;
	*ppAdapter = nullptr;

	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory6> factory6;
	HRESULT hr = m_dxgiFactory.As(&factory6);

	if (FAILED(hr))
	{
		ERROR_AND_DIE("1.6 not supported");
	}
	for (UINT adapterID = 0; DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterID, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)); ++adapterID)
	{
		if (m_adapterIDoverride != UINT_MAX && adapterID != m_adapterIDoverride)
		{
			continue;
		}

		DXGI_ADAPTER_DESC1 desc;
		ThrowIfFailed(adapter->GetDesc1(&desc), "Failed while getting adapter description");

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), m_d3dMinFeatureLevel, _uuidof(ID3D12Device), nullptr)))
		{
			m_adapterID = adapterID;
			m_adapterDescription = desc.Description;
#ifdef _DEBUG
			wchar_t buff[256] = {};
			swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterID, desc.VendorId, desc.DeviceId, desc.Description);
			OutputDebugStringW(buff);
#endif
			break;
		}
	}

#if !defined(NDEBUG)
	if (!adapter && m_adapterIDoverride == UINT_MAX)
	{
		// Try WARP12 instead
		if (FAILED(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter))))
		{
			ERROR_AND_DIE("WARP12 not available. Enable the 'Graphics Tools' optional feature");
		}

		OutputDebugStringA("Direct3D Adapter - WARP12\n");
	}
#endif

	if (!adapter)
	{
		if (m_adapterIDoverride != UINT_MAX)
		{
			ERROR_AND_DIE("Unavailable adapter requested.");
		}
		else
		{
			ERROR_AND_DIE("Unavailable adapter.");
		}
	}

	*ppAdapter = adapter.Detach();
	factory6.Reset();
	adapter.Reset();
}
void RendererD12::CreateRootSignatures()
{
	//D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
	//	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
	//	D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
	//	D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
	//	D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	{
		CD3DX12_DESCRIPTOR_RANGE ranges[(int)GlobalRootSignatureParams::Count]; // Perfomance TIP: Order from most frequent to least frequent.
		ranges[(int)GlobalRootSignatureParams::GBufferVertexPositionSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 texture buffer.
		ranges[(int)GlobalRootSignatureParams::GBufferVertexNormalSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);  // 1 texture buffer.
		ranges[(int)GlobalRootSignatureParams::GBufferVertexAlbedoSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);  // 1 texture buffer.
		ranges[(int)GlobalRootSignatureParams::GBufferMotionVectorSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);  // 1 texture buffer.
		ranges[(int)GlobalRootSignatureParams::GBufferDirectLightSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4);  // 1 texture buffer.
		ranges[(int)GlobalRootSignatureParams::GBufferDepthSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5);  // 1 texture buffer.
		ranges[(int)GlobalRootSignatureParams::GBufferOcclusionSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 6);  // 1 texture buffer.
		ranges[(int)GlobalRootSignatureParams::GBufferVariance].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 7);  // 1 texture buffer.


		ranges[(int)GlobalRootSignatureParams::VertexBuffersSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);  // Index Buffer
		ranges[(int)GlobalRootSignatureParams::TextureBufferSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);  // 1 texture buffer.
		ranges[(int)GlobalRootSignatureParams::NormalMapBufferSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);  // 1 texture buffer.
		ranges[(int)GlobalRootSignatureParams::SkyboxTextureSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);  // 1 texture buffer.
		ranges[(int)GlobalRootSignatureParams::SpecularMapTextureSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);  // 1 texture buffer.


		CD3DX12_ROOT_PARAMETER rootParameters[(UINT)GlobalRootSignatureParams::Count] = {};
		rootParameters[(UINT)GlobalRootSignatureParams::GBufferVertexPositionSlot].InitAsDescriptorTable(1, &ranges[(int)GlobalRootSignatureParams::GBufferVertexPositionSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::GBufferVertexNormalSlot].InitAsDescriptorTable(1, &ranges[(int)GlobalRootSignatureParams::GBufferVertexNormalSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::GBufferVertexAlbedoSlot].InitAsDescriptorTable(1, &ranges[(int)GlobalRootSignatureParams::GBufferVertexAlbedoSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::GBufferMotionVectorSlot].InitAsDescriptorTable(1, &ranges[(int)GlobalRootSignatureParams::GBufferMotionVectorSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::GBufferDirectLightSlot].InitAsDescriptorTable(1, &ranges[(int)GlobalRootSignatureParams::GBufferDirectLightSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::GBufferDepthSlot].InitAsDescriptorTable(1, &ranges[(int)GlobalRootSignatureParams::GBufferDepthSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::GBufferOcclusionSlot].InitAsDescriptorTable(1, &ranges[(int)GlobalRootSignatureParams::GBufferOcclusionSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::GBufferVariance].InitAsDescriptorTable(1, &ranges[(int)GlobalRootSignatureParams::GBufferVariance]);

		rootParameters[(UINT)GlobalRootSignatureParams::VertexBuffersSlot].InitAsDescriptorTable(1, &ranges[(UINT)GlobalRootSignatureParams::VertexBuffersSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::TextureBufferSlot].InitAsDescriptorTable(1, &ranges[(UINT)GlobalRootSignatureParams::TextureBufferSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::NormalMapBufferSlot].InitAsDescriptorTable(1, &ranges[(UINT)GlobalRootSignatureParams::NormalMapBufferSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::SkyboxTextureSlot].InitAsDescriptorTable(1, &ranges[(UINT)GlobalRootSignatureParams::SkyboxTextureSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::SpecularMapTextureSlot].InitAsDescriptorTable(1, &ranges[(UINT)GlobalRootSignatureParams::SpecularMapTextureSlot]);
		rootParameters[(UINT)GlobalRootSignatureParams::SceneConstantSlot].InitAsConstantBufferView(0);
		rootParameters[(UINT)GlobalRootSignatureParams::AccelerationStructureSlot].InitAsShaderResourceView(0);
		CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
		{
			// LinearWrapSampler
			CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT),
		};

		CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, ARRAYSIZE(staticSamplers), staticSamplers);
		SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
	}

	// Local Root Signature
	// This is a root signature that enables a shader to have unique arguments that come from shader tables.
	// TO DO :: Fix the shader space from 5/6 to 1/2 after fixing infinite generation. Since there will be no vertex Buffer coming from Global Root signatures
	// Shader space 0 will be for Acceleration strucure
	// 3 will be for the diffuse texture
	// 4  will be sample buffers slot
	{
		//CD3DX12_DESCRIPTOR_RANGE range[(UINT)LocalRootSignatureParams::Count] = {};
		//range[(UINT)LocalRootSignatureParams::IndexBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,1, 15, 1);
		//range[(UINT)LocalRootSignatureParams::VertexBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 16, 1);

		//CD3DX12_ROOT_PARAMETER localRootParameters[(UINT)LocalRootSignatureParams::Count] = {};
		////localRootParameters[(UINT)LocalRootSignatureParams::AccelerationStructure].InitAsDescriptorTable(1, &range[(UINT)LocalRootSignatureParams::AccelerationStructure]);
		//localRootParameters[(UINT)LocalRootSignatureParams::IndexBuffer].InitAsDescriptorTable(1, &range[(UINT)LocalRootSignatureParams::IndexBuffer]);
		//localRootParameters[(UINT)LocalRootSignatureParams::VertexBuffer].InitAsDescriptorTable(1, &range[(UINT)LocalRootSignatureParams::VertexBuffer]);

		//CD3DX12_ROOT_SIGNATURE_DESC localRSdesc(ARRAYSIZE(localRootParameters), localRootParameters);
		//localRSdesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		//SerializeAndCreateRaytracingRootSignature(localRSdesc, &m_raytracingLocalRootSignature);
	}
}
void RendererD12::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig)
{
	ID3DBlob* blob;
	ID3DBlob* error;
	HRESULT result;
	result = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	m_Rdevice->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig)));
}
void RendererD12::SerializeAndCreateRaytracingRootSignature(ID3D12Device5* device, D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig)
{
	ID3DBlob* blob;
	ID3DBlob* error;
	HRESULT result;
	result = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig)));
}

//----------------------------DXR  RAYTRACING FUNCTIONS--------------------------
void RendererD12::Prepare()
{

	ThrowIfFailed(m_RcommandAllocator[m_frameIndex]->Reset(), "Failed while Resetting command allocator");
	if (m_currentShader == nullptr)
	{
		ThrowIfFailed(m_RcommandList->Reset(m_RcommandAllocator[m_frameIndex].Get(), nullptr), "Failed while Resetting command allocator");
	}
	else
	{
		ThrowIfFailed(m_RcommandList->Reset(m_RcommandAllocator[m_frameIndex].Get(), m_currentShader->m_pipelineStateObject.Get()), "Failed while Resetting command allocator");
	}
	m_beforeState = D3D12_RESOURCE_STATE_PRESENT;
	// Transition the render target into the correct state to allow for drawing into it.
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTarget[m_frameIndex].Get(), m_beforeState, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_RcommandList->ResourceBarrier(1, &barrier);
	//if (m_beforeState != D3D12_RESOURCE_STATE_RENDER_TARGET)
	//{
	//	// Transition the render target into the correct state to allow for drawing into it.
	//	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTarget[m_frameIndex].Get(), m_beforeState, D3D12_RESOURCE_STATE_RENDER_TARGET);
	//	m_RcommandList->ResourceBarrier(1, &barrier);
	//}
}
void RendererD12::RunRaytracer()
{
	auto commandList = m_RcommandList.Get();
	auto GetGBuffers = (m_resourceManager->m_GpuresourceBuffers);
	commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());
	// Copy the updated scene constant buffer to GPU.
	m_sceneCB.CopyStagingToGpu(m_frameIndex);
	m_lightCB.CopyStagingToGpu(m_frameIndex);
	//m_irradianceCacheGPUBuffer.CopyFromCPUToGPU(m_frameIndex);
	commandList->SetComputeRootConstantBufferView((UINT)GlobalRootSignatureParams::SceneConstantSlot, m_sceneCB.GpuVirtualAddress(m_frameIndex));
	//Bind the heaps, acceleration structure and dispatch rays.

	{
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::VertexPosition], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::VertexNormal], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::VertexAlbedo], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::MotionVector], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::DirectLight], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::Depth], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::OcclusionTexture], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_resourceManager->TransitionResource(&m_denoiser->m_varianceEstimator->m_varianceResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	m_dispatchRayRuntime = (float)GetCurrentTimeSeconds();
	commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::GBufferVertexPositionSlot, GetGBuffers[(UINT)GBufferResources::VertexPosition].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::GBufferVertexNormalSlot, GetGBuffers[(UINT)GBufferResources::VertexNormal].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::GBufferVertexAlbedoSlot, GetGBuffers[(UINT)GBufferResources::VertexAlbedo].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::GBufferMotionVectorSlot, GetGBuffers[(UINT)GBufferResources::MotionVector].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::GBufferDirectLightSlot, GetGBuffers[(UINT)GBufferResources::DirectLight].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::GBufferDepthSlot, GetGBuffers[(UINT)GBufferResources::Depth].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::GBufferOcclusionSlot, GetGBuffers[(UINT)GBufferResources::OcclusionTexture].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::GBufferVariance, m_denoiser->m_varianceEstimator->m_varianceResource.gpuWriteDescriptorHandle);

	if (m_currentScene == Scenes::Minecraft)
	{
		//commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::IndexBuffersSlot, m_indexBuffer.gpuDescriptorHandle);
		commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::VertexBuffersSlot, m_indexBuffer.gpuReadDescriptorHandle);
		commandList->SetComputeRootShaderResourceView((UINT)GlobalRootSignatureParams::AccelerationStructureSlot, m_topLevelAccelerationStructure->GetGPUVirtualAddress());
	}
	else if (m_currentScene == Scenes::Bunny)
	{
		//commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::IndexBuffersSlot, m_bunnyIndexBuffer.gpuDescriptorHandle);
		commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::VertexBuffersSlot, m_bunnyIndexBuffer.gpuReadDescriptorHandle);
		commandList->SetComputeRootShaderResourceView((UINT)GlobalRootSignatureParams::AccelerationStructureSlot, m_bunnyTlas->GetGPUVirtualAddress());
	}

	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::TextureBufferSlot, m_loadedTextures[0]->m_gpuDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::NormalMapBufferSlot, m_loadedTextures[1]->m_gpuDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::SkyboxTextureSlot, m_loadedTextures[2]->m_gpuDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::SpecularMapTextureSlot, m_loadedTextures[3]->m_gpuDescriptorHandle);
	//commandList->SetComputeRootShaderResourceView((UINT)GlobalRootSignatureParams::IrradianceCacheSlot, m_irradianceCacheGPUBuffer.GpuVirtualAddress(m_frameIndex));

	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	dispatchDesc.HitGroupTable.StartAddress = HitShaderTable.GetResource()->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = HitShaderTable.GetResource()->GetDesc().Width;
	dispatchDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
	dispatchDesc.MissShaderTable.StartAddress = MissShaderTable.GetResource()->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = MissShaderTable.GetResource()->GetDesc().Width;
	dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
	dispatchDesc.RayGenerationShaderRecord.StartAddress = RaygenShaderTable.GetResource()->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = RaygenShaderTable.GetResource()->GetDesc().Width;
	dispatchDesc.Width = m_dimensions.x;
	dispatchDesc.Height = m_dimensions.y;
	dispatchDesc.Depth = 1;
	m_dxrCommandList.Get()->SetPipelineState1(m_dxrStateObject.Get());
	m_dxrCommandList.Get()->DispatchRays(&dispatchDesc);

	m_dispatchRayRuntime = (float)GetCurrentTimeSeconds() - m_dispatchRayRuntime;
	{
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::VertexPosition], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::VertexNormal], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::VertexAlbedo], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::MotionVector], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::DirectLight], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::Depth], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::OcclusionTexture], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_resourceManager->TransitionResource(&m_denoiser->m_varianceEstimator->m_varianceResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

}
void RendererD12::RunGI()
{
	m_globalIllumination->Run(m_dxrCommandList.Get(), m_frameIndex);
	m_denoiser->m_varianceEstimator->RunPartialDerivatives(m_dxrCommandList.Get(), m_frameIndex);


}
void RendererD12::RunDenoiser(bool temporalSamplerOn, bool denoiserOn)
{
	m_denoiser->Run(m_dxrCommandList.Get(), 1, m_descriptorHeap.Get(), temporalSamplerOn, denoiserOn);
	GpuBuffer* denoiserOutputResource = m_resourceManager->GetDenoiserOutputResource();

	auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(denoiserOutputResource->GetResource());
	m_RcommandList->ResourceBarrier(1, &uavBarrier);
	//m_resourceManager->TransitionResource(denoiserOutputeResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
}
void RendererD12::RunCompositor(bool denoiserOn, bool godRaysOn)
{
	m_compositor->Run(m_dxrCommandList.Get(), m_descriptorHeap.Get(), denoiserOn);
	if (godRaysOn)
	{
		m_postProcess->RunGodRays(m_dxrCommandList.Get(), m_descriptorHeap.Get());
	}

}
void RendererD12::CopyRaytracingOutputToBackbuffer()
{
	auto commandList = m_RcommandList.Get();
	auto renderTarget = GetBackBuffer();

	D3D12_RESOURCE_BARRIER preCopyBarriers[2];
	preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_resourceManager->m_GpuresourceBuffers[(int)GBufferResources::OutputResource].GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);
	commandList->CopyResource(renderTarget, m_resourceManager->m_GpuresourceBuffers[(int)GBufferResources::OutputResource].GetResource());

	D3D12_RESOURCE_BARRIER imguiRenderTargetbarr;
	imguiRenderTargetbarr = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &imguiRenderTargetbarr);
}
void RendererD12::FinishRaytraceCopyToBackBuffer()
{
	auto commandList = m_RcommandList.Get();
	auto renderTarget = GetBackBuffer();

	D3D12_RESOURCE_BARRIER postCopyBarriers[2];
	postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_resourceManager->m_GpuresourceBuffers[(int)GBufferResources::OutputResource].GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}
void RendererD12::Present()
{
	auto renderTarget = GetBackBuffer();
	// Transition the render target to the state that allows it to be presented to the display.
	if (m_renderingPipeline == RenderingPipeline::Raytracing)
	{
		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PRESENT);
		m_RcommandList->ResourceBarrier(1, &barrier);
	}
	else if (m_renderingPipeline == RenderingPipeline::Rasterization)
	{
		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_RcommandList->ResourceBarrier(1, &barrier);
	}
	//------------EXECUTING COMMAND LIST------------------
	HRESULT commandListClosed = m_RcommandList->Close();
	if (FAILED(commandListClosed))
	{
		ERROR_AND_DIE("Failed While closing command list");
	}

	ID3D12CommandList* commandLists[] = { m_RcommandList.Get() };
	m_RcommandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);

	HRESULT result;
	result = m_RswapChain->Present(0, 0);
//	HRESULT result2 = m_Rdevice.Get()->GetDeviceRemovedReason();
	if (FAILED(result))
	{
		ERROR_AND_DIE("Failed while presenting");
	}

}
void RendererD12::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	m_gpuWaitTime = (float)GetCurrentTimeSeconds();
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	HRESULT result = m_RcommandQueue->Signal(m_fence.Get(), currentFenceValue);
	if (FAILED(result)) { ERROR_AND_DIE("Failed while moving to next Frame"); }



	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		result = m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent.Get());
		if (FAILED(result)) { ERROR_AND_DIE("Failed while moving to next Frame"); }
		WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
	}
	// Update the back buffer index.
	m_frameIndex = m_RswapChain->GetCurrentBackBufferIndex();
	// Set the fence value for the next frame.
	m_fenceValues[m_frameIndex] = currentFenceValue + 1;

	//--------------CLEAR DYNAMIC RENDER ITEMS----------------
	for (int i = 0; i < m_dynamicRenderItems.size(); i++)
	{
		//RenderItems& item = m_dynamicRenderItems[i];
		//item.fenceValue = m_fenceValues[m_frameIndex];
		//HRESULT hr = m_Rdevice.Get()->CreateFence(item.fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&item.fence));
		//if (FAILED(hr)) {
		//	ERROR_AND_DIE("Failed while creating the Fence for Vertex buffer");
		//}
		//m_RcommandQueue->Signal(item.fence, item.fenceValue);
		//item.fence->SetEventOnCompletion(item.fenceValue, item.fenceEvent);
		//WaitForSingleObject(item.fenceEvent, INFINITE);

		m_dynamicRenderItems[i].verticesPNCUTB.ResetResources();
		m_dynamicRenderItems[i].verticesPCU.ResetResources();
		m_dynamicRenderItems[i].indices.ResetResources();
		//m_fenceValues[m_frameIndex] = item.fenceValue + 1;
	}
	m_dynamicRenderItems.clear();
	m_gpuWaitTime = (float)GetCurrentTimeSeconds() - m_gpuWaitTime;

}

//----------------------------TEXTURES------------------------
TextureD12* RendererD12::LoadTexture(std::string fileName, std::string filePath, TextureType type)
{
	if (!DoesFileExist(filePath))
	{
		ERROR_AND_DIE("Texture file does not exist");
	}
	TextureD12* textureToLoad;
	textureToLoad = GetTextureForFileName(fileName.c_str());
	if (textureToLoad != nullptr)
	{
		return textureToLoad;
	}
	std::wstring widestr = std::wstring(filePath.begin(), filePath.end());
	const wchar_t* fileWchar = widestr.c_str();
	textureToLoad = new TextureD12();
	/*	if (type == TextureType::DDS)
		{
			std::unique_ptr<uint8_t[]> ddsData;
			std::vector<D3D12_SUBRESOURCE_DATA> subresources;

			ThrowIfFailed(DirectX::LoadDDSTextureFromFile(m_Rdevice.Get(), fileWchar, &textureToLoad->m_textureResource, ddsData, subresources), "Failed While Creating DDS Texture");

			const UINT64 uploadBufferSize = GetRequiredIntermediateSize(*&textureToLoad->m_textureResource, 0, static_cast<UINT>(subresources.size()));
			CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
			ThrowIfFailed(m_Rdevice->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&textureToLoad->m_uploadResource)
			), "Failed while Creating commited resource for DDS Texture");


			UpdateSubresources(m_RcommandList.Get(), textureToLoad->m_textureResource.Get(), textureToLoad->m_uploadResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());
			CD3DX12_RESOURCE_BARRIER barr = CD3DX12_RESOURCE_BARRIER::Transition(textureToLoad->m_textureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			m_RcommandList->ResourceBarrier(1, &barr);
			CreateTextureSRV(textureToLoad);
			textureToLoad->m_textureType = type;
		}
		else */if (type == TextureType::WICT)
		{
			DirectX::ResourceUploadBatch resourceUpload(m_Rdevice.Get());
			resourceUpload.Begin();

			ThrowIfFailed(DirectX::CreateWICTextureFromFile(m_Rdevice.Get(), resourceUpload, fileWchar, &textureToLoad->m_textureResource, true), "Failed While Creating WICT texture");
			CreateTextureSRV(textureToLoad);

			//Get if Upload is finished
			auto finish = resourceUpload.End(m_RcommandQueue.Get());

			// Wait for the upload thread to terminate
			finish.wait();
			textureToLoad->m_textureType = type;
		}
		textureToLoad->m_name = fileName;
		textureToLoad->m_path = filePath;
		m_loadedTextures.push_back(textureToLoad);
		return textureToLoad;
}
TextureD12* RendererD12::GetTextureForFileNameOrPath(char const* fileName, const char* filePath)
{
	for (int i = 0; i < m_loadedTextures.size(); i++)
	{
		if (m_loadedTextures[i]->GetImageFileName() == fileName || m_loadedTextures[i]->GetImageFilePath() == filePath)
		{
			return m_loadedTextures[i];
		}
	}
	return nullptr;
}
TextureD12* RendererD12::GetTextureForFileName(char const* name)
{
	for (int i = 0; i < m_loadedTextures.size(); i++)
	{
		if (m_loadedTextures[i]->GetImageFileName() == name)
		{
			return m_loadedTextures[i];
		}
	}
	return nullptr;
}
TextureD12* RendererD12::CreateTextureFromImage(const Image& image)
{
	TextureD12* textureToBeCreated = new TextureD12();

	CD3DX12_RESOURCE_DESC textureDesc;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width = image.GetDimensions().x;
	textureDesc.Height = image.GetDimensions().y;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Alignment = 0;


	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	HRESULT hr = m_Rdevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&textureToBeCreated->m_textureResource));

	if (FAILED(hr))
	{
		ERROR_AND_DIE("Failed to create commited resource.");
	}

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(textureToBeCreated->m_textureResource.Get(), 0, 1);

	// Create the GPU upload buffer.
	heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	hr = m_Rdevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureToBeCreated->m_uploadResource));

	if (FAILED(hr))
	{
		ERROR_AND_DIE("Failed to create commited resource.");
	}

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = image.GetRawData();
	textureData.RowPitch = image.GetDimensions().x * sizeof(Rgba8);
	textureData.SlicePitch = textureData.RowPitch * image.GetDimensions().y;

	UpdateSubresources(m_RcommandList.Get(), textureToBeCreated->m_textureResource.Get(), textureToBeCreated->m_uploadResource.Get(), 0, 0, 1, &textureData);
	auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(textureToBeCreated->m_textureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_RcommandList->ResourceBarrier(1, &resourceBarrier);

	// Get handle for srv desc heap
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	textureToBeCreated->m_heapIndex = AllocateDescriptor(&textureToBeCreated->m_cpuDescriptorHandle, textureToBeCreated->m_heapIndex);
	m_Rdevice->CreateShaderResourceView(textureToBeCreated->m_textureResource.Get(), &srvDesc, textureToBeCreated->m_cpuDescriptorHandle);
	textureToBeCreated->m_gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), textureToBeCreated->m_heapIndex, m_descriptorSize);

	// Debug Name
#ifdef _DEBUG
	textureToBeCreated->m_textureResource->SetName(L"Diffuse Texture 0");
#endif
	return textureToBeCreated;
}
TextureD12* RendererD12::CreateOrGetTextureFromFile(char const* fileName, char const* imageFilePath)
{
	//See if we already have this texture previously loaded
	TextureD12* existingTexture = GetTextureForFileNameOrPath(fileName, imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	TextureD12* newTexture = CreateTextureFromFile(imageFilePath);
	newTexture->m_name = fileName;
	return newTexture;
}
TextureD12* RendererD12::CreateTextureFromFile(char const* imageFilePath)
{
	Image* textureImage = new Image(imageFilePath);
	TextureD12* texture = CreateTextureFromImage(*textureImage);
	//SetDebugName(texture->m_texture, imageFilePath);
	m_loadedTextures.push_back(texture);
	return texture;
}
void RendererD12::BindTexture(int index, TextureD12* textureToBind)
{
	m_RcommandList->SetGraphicsRootDescriptorTable(index, textureToBind->m_gpuDescriptorHandle);
}

void RendererD12::BindHandle(int index, D3D12_GPU_DESCRIPTOR_HANDLE& handle)
{
	m_RcommandList->SetGraphicsRootDescriptorTable(index, handle);
}


//----------------------------MAIN RENDER FUNCTIONS----------------------
void RendererD12::ClearScreen(Rgba8 color)
{
	auto renderTarget = GetBackBufferCPUHandle();
	auto depthTarget = &m_depthStencilBuffer.cpuDescriptorHandle;
	m_RcommandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
	m_RcommandList->OMSetRenderTargets(1, renderTarget, FALSE, depthTarget);
	// Then set the color to clear the window to.
	float colorfloats[4];
	Rgba8(0, 0, 0).GetAsFloats(colorfloats);

	m_RcommandList->ClearRenderTargetView(*renderTarget, colorfloats, 0, NULL);
	D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
	m_RcommandList.Get()->ClearDepthStencilView(m_depthStencilBuffer.cpuDescriptorHandle, flags, 1, 0, 0, nullptr);
}
void RendererD12::DrawVertexArray(int numberOfVertices, VertexNormalArray verticesToDraw)
{
	auto device = m_Rdevice.Get();
	auto cmdList = m_RcommandList.Get();
	const UINT vByteSize = sizeof(Vertex_PNCU) * numberOfVertices;
	RenderItems item;

	item.verticesPNCUTB.CreateDefaultBuffer(device, m_RcommandList.Get(), verticesToDraw.data(), vByteSize);


	//--------------CREATING VERTEX BUFFER VIEW------------
	D3D12_VERTEX_BUFFER_VIEW vbv = item.verticesPNCUTB.CreateAndGetVertexBufferView(vByteSize);

	cmdList->IASetVertexBuffers(0, 1, &vbv);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(numberOfVertices, 1, 0, 0);


	ExecuteCommandList(cmdList);
	WaitForGpu();

	ThrowIfFailed(m_RcommandAllocator[m_frameIndex]->Reset(), "Failed while Resetting command allocator");
	ThrowIfFailed(m_RcommandList->Reset(m_RcommandAllocator[m_frameIndex].Get(), m_currentShader->m_pipelineStateObject.Get()), "Failed while Resetting command allocator");

	item.verticesPNCUTB.ResetResources();
	//m_RcommandList->Reset(m_rCommand);
}
void RendererD12::DrawIndexedVertexArray(int numberOfVertices, std::vector<Vertex_PCUTBN>& verticesToDraw, std::vector<unsigned int>& indexes)
{
	auto device = m_Rdevice.Get();
	auto cmdList = m_RcommandList.Get();
	const UINT vByteSize = (UINT)( sizeof(Vertex_PCUTBN) * numberOfVertices );
	const UINT iByteSize = (UINT) ( sizeof(unsigned int) * indexes.size() );
	RenderItems item;
	item.verticesPNCUTB.CreateDefaultBuffer(device, m_RcommandList.Get(), verticesToDraw.data(), vByteSize);
	item.indices.CreateDefaultBuffer(device, m_RcommandList.Get(), indexes.data(), iByteSize);

	//--------------CREATING VERTEX BUFFER VIEW------------
	D3D12_VERTEX_BUFFER_VIEW vbv = item.verticesPNCUTB.CreateAndGetVertexBufferView(vByteSize);
	D3D12_INDEX_BUFFER_VIEW ibv = item.indices.CreateAndGetIndexBufferView(iByteSize);
	cmdList->IASetVertexBuffers(0, 1, &vbv);
	cmdList->IASetIndexBuffer(&ibv);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT indexCount = static_cast<UINT>(indexes.size());
	cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	m_dynamicRenderItems.push_back(item);
}
void RendererD12::DrawVertexArray(int numberOfVertices, VertexArray verticesToDraw)
{
	if (numberOfVertices == 0)
	{
		return;
	}
	auto device = m_Rdevice.Get();
	auto cmdList = m_RcommandList.Get();
	const UINT vByteSize = sizeof(Vertex_PCU) * numberOfVertices;

	RenderItems item;
	item.verticesPCU.CreateDefaultBuffer(device, m_RcommandList.Get(), verticesToDraw.data(), vByteSize);

	//--------------CREATING VERTEX BUFFER VIEW------------
	D3D12_VERTEX_BUFFER_VIEW vbv = item.verticesPCU.CreateAndGetVertexBufferView(vByteSize);
	cmdList->IASetVertexBuffers(0, 1, &vbv);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(numberOfVertices, 1, 0, 0);
	m_dynamicRenderItems.push_back(item);
}
void RendererD12::SetDepthStencilState(DepthTestD12 depthTest, bool writeDepth)
{
	m_depthStencilBuffer.depthDesc.DepthEnable = true;
	m_depthStencilBuffer.depthDesc.StencilEnable = true;
	if (writeDepth)
	{
		m_depthStencilBuffer.depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	}
	else
	{
		m_depthStencilBuffer.depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	}
	switch (depthTest)
	{
	case DepthTestD12::ALWAYS:
		m_depthStencilBuffer.depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		m_depthStencilBuffer.depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		m_depthStencilBuffer.depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	case DepthTestD12::NEVER:
		m_depthStencilBuffer.depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
		m_depthStencilBuffer.depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		m_depthStencilBuffer.depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		break;
	case DepthTestD12::EQUAL:
		m_depthStencilBuffer.depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
		m_depthStencilBuffer.depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
		m_depthStencilBuffer.depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
		break;
	case DepthTestD12::NOTEQUAL:
		m_depthStencilBuffer.depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
		m_depthStencilBuffer.depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
		m_depthStencilBuffer.depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
		break;
	case DepthTestD12::LESS:
		m_depthStencilBuffer.depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		m_depthStencilBuffer.depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;
		m_depthStencilBuffer.depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;
		break;
	case DepthTestD12::LESSEQUAL:
		m_depthStencilBuffer.depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		m_depthStencilBuffer.depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		m_depthStencilBuffer.depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		break;
	case DepthTestD12::GREATER:
		m_depthStencilBuffer.depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
		m_depthStencilBuffer.depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER;
		m_depthStencilBuffer.depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER;
		break;
	case DepthTestD12::GREATEREQUAL:
		m_depthStencilBuffer.depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		m_depthStencilBuffer.depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		m_depthStencilBuffer.depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		break;
	default:
		break;
	}
	// Stencil operations if pixel is front-facing
	m_depthStencilBuffer.depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	m_depthStencilBuffer.depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	m_depthStencilBuffer.depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;


	// Stencil operations if pixel is back-facing
	m_depthStencilBuffer.depthDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	m_depthStencilBuffer.depthDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	m_depthStencilBuffer.depthDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

	m_RcommandList.Get()->OMSetStencilRef(1);
}
//---------------------------GET FUNCTIONS----------------------

RendererD12Config  RendererD12::GetRenderConfig()
{
	return m_renderConfig;
}
IDXGISwapChain3* RendererD12::GetSwapChain()
{
	return m_RswapChain.Get();
}

void RendererD12::PopulateSimpleCubeVertices()
{
	//Texture* chunkTexture = new Texture();
	//IntVec2 dimensions = IntVec2(2048,2048);
	//SpriteSheet m_mapSpriteSheet = SpriteSheet(*chunkTexture, dimensions, IntVec2(64, 64));

	//Vec3 bottomLeft, bottomRight, topLeft, topRight;
	//float minx, miny, minz, maxx, maxy, maxz;
	//AABB3 blockbounds;
	//Vec3 m_chunkPosition = Vec3();
	//UINT32 m_indexCount = 0;
	//std::vector<Vertex_PNCU>		m_vertexes;
	//std::vector<unsigned int>	m_indexes;

	//int WorldPosX = 0;
	//int WorldPosY = 0;
	//for (int WorldPosX = 0; WorldPosX < 5; WorldPosX++)
	//{
	//	for (int WorldPosY = 0; WorldPosY < 5; WorldPosY++)
	//	{
	//		for (int z = 0; z < 128; z++)
	//		{
	//			for (int y = 0; y < 16; y++)
	//			{
	//				for (int x = 0; x < 16; x++)
	//				{
	//					m_chunkPosition.x = WorldPosX * 16.0f;
	//					m_chunkPosition.y = WorldPosY * 16.0f;
	//					blockbounds = AABB3(Vec3(m_chunkPosition.x + (float)x , m_chunkPosition.y + (float)y , (float)z), Vec3(m_chunkPosition.x + x + 1.0f, m_chunkPosition.y + y + 1.0f, z + 1.0f));
	//					minx = blockbounds.m_mins.x;
	//					miny = blockbounds.m_mins.y;
	//					minz = blockbounds.m_mins.z;

	//					maxx = blockbounds.m_maxs.x;
	//					maxy = blockbounds.m_maxs.y;
	//					maxz = blockbounds.m_maxs.z;

	//					bottomLeft = Vec3(maxx, maxy, minz);
	//					bottomRight = Vec3(maxx, miny, minz);
	//					topLeft = Vec3(minx, maxy, minz);
	//					topRight = Vec3(minx, miny, minz);

	//					//------------------------------------FLOOR WALL ---------------------------------------
	//					AddVertsForIndexedQuad3D(m_DXRverts, m_DXRindexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE,
	//						m_mapSpriteSheet.GetSpriteDef(Vec2(32, 34), IntVec2(64, 64)).GetUVs());
	//					m_indexCount += 4;

	//					bottomLeft = Vec3(maxx, miny, maxz);
	//					bottomRight = Vec3(maxx, maxy, maxz);
	//					topLeft = Vec3(minx, miny, maxz);
	//					topRight = Vec3(minx, maxy, maxz);

	//					//----------------------------------ROOF WALL-----------------------------------------------
	//					AddVertsForIndexedQuad3D(m_DXRverts, m_DXRindexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE,
	//						m_mapSpriteSheet.GetSpriteDef(Vec2(32, 34), IntVec2(64, 64)).GetUVs());
	//					m_indexCount += 4;

	//					//----------------------LEFT WALL-------------------------------
	//					bottomLeft = Vec3(maxx, maxy, minz);
	//					bottomRight = Vec3(minx, maxy, minz);
	//					topLeft = Vec3(maxx, maxy, maxz);
	//					topRight = Vec3(minx, maxy, maxz);

	//					AddVertsForIndexedQuad3D(m_DXRverts, m_DXRindexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE,
	//						m_mapSpriteSheet.GetSpriteDef(Vec2(33, 34), IntVec2(64, 64)).GetUVs());
	//					m_indexCount += 4;

	//					//-------------------------RIGHT WALL-----------------------------
	//					bottomLeft = Vec3(minx, miny, minz);
	//					bottomRight = Vec3(maxx, miny, minz);
	//					topLeft = Vec3(minx, miny, maxz);
	//					topRight = Vec3(maxx, miny, maxz);

	//					AddVertsForIndexedQuad3D(m_DXRverts, m_DXRindexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE,
	//						m_mapSpriteSheet.GetSpriteDef(Vec2(33, 34), IntVec2(64, 64)).GetUVs());
	//					m_indexCount += 4;

	//					//------------------------FRONT  WALL---------------------------
	//					bottomLeft = Vec3(minx, maxy, minz);
	//					bottomRight = Vec3(minx, miny, minz);
	//					topLeft = Vec3(minx, maxy, maxz);
	//					topRight = Vec3(minx, miny, maxz);

	//					AddVertsForIndexedQuad3D(m_DXRverts, m_DXRindexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE,
	//						m_mapSpriteSheet.GetSpriteDef(Vec2(33, 34), IntVec2(64, 64)).GetUVs());
	//					m_indexCount += 4;

	//					//----------------------BACK WALL------------------------------
	//					bottomLeft = Vec3(maxx, miny, minz);
	//					bottomRight = Vec3(maxx, maxy, minz);
	//					topLeft = Vec3(maxx, miny, maxz);
	//					topRight = Vec3(maxx, maxy, maxz);

	//					AddVertsForIndexedQuad3D(m_DXRverts, m_DXRindexes, m_indexCount, topLeft, bottomLeft, bottomRight, topRight, Rgba8::WHITE,
	//						m_mapSpriteSheet.GetSpriteDef(Vec2(33, 34), IntVec2(64, 64)).GetUVs());
	//					m_indexCount += 4;

	//				}
	//			}
	//		}
	//	}
	//}

	////Vertex vert;
	////VertexNormalArray pncus;
	////Vertex_PNCU pncu;
	////Index ind;
	////for (int i = 0; i < (int)m_vertexes.size(); i++)
	////{
	////	vert.position = m_vertexes[i].m_position;
	////	vert.normal = m_vertexes[i].m_normal;
	////	vert.uv = m_vertexes[i].m_uvTexCoords;
	////	vertices.push_back(vert);
	////}

	////for (int i = 0; i < (int)m_indexes.size(); i++)
	////{
	////	ind = (int)m_indexes[i];
	////	indexes.push_back(ind);
	////}


}
void RendererD12::AllocateUploadBuffer(ID3D12Device* pDevice, void* pData, UINT64 datasize, ID3D12Resource** ppResource, const wchar_t* resourceName = nullptr)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ppResource)), "Failed while Creating Committed Resource");

	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}
	void* pMappedData;
	(*ppResource)->Map(0, nullptr, &pMappedData);
	memcpy(pMappedData, pData, datasize);
	(*ppResource)->Unmap(0, nullptr);
}
void RendererD12::AllocateUAVBuffer(ID3D12Device* pDevice, UINT64 bufferSize, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, const wchar_t* resourceName = nullptr)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(ppResource)), "Failed while Creating UAV Buffer");
	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}
}
void RendererD12::ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	ThrowIfFailed(commandList.Get()->Close(), "Failed While Closing Command List");
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	m_RcommandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);
}
void RendererD12::ExecuteCommandList(int commandsToExecute, ComPtr<ID3D12GraphicsCommandList> commandList)
{
	UNUSED((void)commandsToExecute);
	//ThrowIfFailed(commandList.Get()->Close(), "Failed While Closing Command List");
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	m_RcommandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);
}
void RendererD12::WaitForGpu()
{
	if (m_RcommandQueue && m_fence && m_fenceEvent.IsValid())
	{
		UINT64 fenceValue = m_fenceValues[m_frameIndex];
		if (SUCCEEDED(m_RcommandQueue->Signal(m_fence.Get(), fenceValue)))
		{
			if (SUCCEEDED(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent.Get())))
			{
				WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
				// Increment the fence value for the current frame.
				m_fenceValues[m_frameIndex]++;
			}
		}
	}
}
void  RendererD12::ThrowIfFailed(HRESULT hr, const char* msg)
{
	if (FAILED(hr))
	{
		ERROR_AND_DIE(msg);
	}
}
void  RendererD12::ThrowIfFalse(bool result, const char* msg)
{
	if (result == false)
	{
		ERROR_AND_DIE(msg);
	}
}

//----------------------------SHADERS AND SHADER COMPILATIONS----------------------
ShaderCompiler::ShaderCompiler()
{
	HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_utils));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));

	hr = m_utils->CreateIncludeHandler(&m_handler);
}
ShaderCompiler::~ShaderCompiler()
{
	m_compiler.Get()->Release();
	m_utils.Get()->Release();
	m_handler.Get()->Release();
}
ComPtr<IDxcBlob> ShaderCompiler::Compile(const char* ShaderFilePath)
{
	wchar_t wtext[40];
	size_t size = strlen(ShaderFilePath) + 1;
	mbstowcs_s(&size, wtext, ShaderFilePath, strlen(ShaderFilePath) + 1);//Plus null
	LPWSTR ptr = wtext;

	uint32_t codePage = CP_UTF8;
	ComPtr<IDxcBlobEncoding> sourceBlob;
	HRESULT hr = m_utils->CreateBlobFromFile(ptr, &codePage, &sourceBlob);
	//if(FAILED(hr)) Handle file loading error...

	ComPtr<IDxcOperationResult> result;
	hr = m_compiler->Compile(
		sourceBlob.Get(), // pSource
		wtext, // pSourceName
		NULL, // pEntryPoint
		L"lib_6_3", // pTargetProfile
		NULL, 0, // pArguments, argCount
		NULL, 0, // pDefines, defineCount
		m_handler.Get(), // pIncludeHandler
		&result); // ppResult
	if (SUCCEEDED(hr))
		result->GetStatus(&hr);

	if (result)
	{
		ComPtr<IDxcBlobEncoding> errorsBlob;
		hr = result->GetErrorBuffer(&errorsBlob);
		if (FAILED(hr) && errorsBlob)
		{
			DebuggerPrintf("Compilation failed with errors:\n%hs\n",
				(const char*)errorsBlob->GetBufferPointer());
		}
		else
		{
			DebuggerPrintf("Shader Compiled successfully");
		}
	}
	// Handle compilation error...
	ComPtr<IDxcBlob> code;
	result->GetResult(&code);
	return code;
}
ComPtr<ID3DBlob> ShaderCompiler::CompileVsPs(const char* filePath, const D3D_SHADER_MACRO* defines, const std::string& entryPoint, const std::string& target)
{
	UINT compileFlags = 0;
#if defined(ENGINE_DEBUG_RENDER)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;

	wchar_t wtext[40];
	size_t size = strlen(filePath) + 1;
	mbstowcs_s(&size, wtext, filePath, strlen(filePath) + 1);//Plus null
	HRESULT hr = D3DCompileFromFile(wtext, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (FAILED(hr))
	{
		DebuggerPrintf("Compilation failed with errors:\n%hs\n",
			(const char*)errors->GetBufferPointer());
	}
	else
	{
		DebuggerPrintf("Shader Compiled successfully");
	}
	if (errors != nullptr)
	{
		ERROR_AND_DIE("Failed While Compiling Shader D12");
	}
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Failed While Compiling Shader D12");
	}
	return byteCode;


	//wchar_t entryTex[40];
	//size_t entrySize = strlen(entryPoint.c_str()) + 1;
	//mbstowcs_s(&entrySize, entryTex, entryPoint.c_str(), strlen(entryPoint.c_str()) + 1);//Plus null
	//LPWSTR entry = entryTex;

	//wchar_t targetText[40];
	//size_t targetSize = strlen(target.c_str()) + 1;
	//mbstowcs_s(&targetSize, targetText, target.c_str(), strlen(target.c_str()) + 1);//Plus null
	//LPWSTR targetLPWSTR = targetText;
	//
	//uint32_t codePage = CP_UTF8;
	//ComPtr<IDxcBlobEncoding> sourceBlob;
	//HRESULT hre = m_utils->CreateBlobFromFile(ptr, &codePage, &sourceBlob);
	//if (!SUCCEEDED(hre))
	//{
	//	ERROR_AND_DIE("Failed while Creating source blob");
	//}
	//ComPtr<IDxcOperationResult> result;
	//HRESULT hr = m_compiler->Compile(
	//	sourceBlob.Get(), // pSource
	//	wtext, // pSourceName
	//	entry, // pEntryPoint
	//	targetLPWSTR, // pTargetProfile
	//	NULL, 0, // pArguments, argCount
	//	NULL, 0, // pDefines, defineCount
	//	m_handler.Get(), // pIncludeHandler
	//	&result); // ppResult

	//if (hr == E_FAIL)
	//{
	//	ERROR_AND_DIE("Failed while compiling Shader");
	//}
	//if (SUCCEEDED(hr))
	//	result->GetStatus(&hr);

	//if (result)
	//{
	//	ComPtr<IDxcBlobEncoding> errorsBlob;
	//	hr = result->GetErrorBuffer(&errorsBlob);
	//	if (FAILED(hr) && errorsBlob)
	//	{
	//		DebuggerPrintf("Compilation failed with errors:\n%hs\n",
	//			(const char*)errorsBlob->GetBufferPointer());
	//	}
	//	else
	//	{
	//		DebuggerPrintf("Shader Compiled successfully");
	//	}
	//}
	//ComPtr<IDxcBlob> code;
	//result->GetResult(&code);
	//return code;
}

ComPtr<IDxcBlob> ShaderCompiler::CompileComputeShader(const char* ShaderFilePath)
{
	wchar_t wtext[40];
	size_t size = strlen(ShaderFilePath) + 1;
	mbstowcs_s(&size, wtext, ShaderFilePath, strlen(ShaderFilePath) + 1);//Plus null
	LPWSTR ptr = wtext;

	uint32_t codePage = CP_UTF8;
	ComPtr<IDxcBlobEncoding> sourceBlob;
	HRESULT hr = m_utils->CreateBlobFromFile(ptr, &codePage, &sourceBlob);
	//if(FAILED(hr)) Handle file loading error...

	ComPtr<IDxcOperationResult> result;
	hr = m_compiler->Compile(
		sourceBlob.Get(), // pSource
		wtext, // pSourceName
		L"main", // pEntryPoint
		L"cs_6_3", // pTargetProfile
		NULL, 0, // pArguments, argCount
		NULL, 0, // pDefines, defineCount
		m_handler.Get(), // pIncludeHandler
		&result); // ppResult
	if (SUCCEEDED(hr))
		result->GetStatus(&hr);

	if (result)
	{
		ComPtr<IDxcBlobEncoding> errorsBlob;
		hr = result->GetErrorBuffer(&errorsBlob);
		if (FAILED(hr) && errorsBlob)
		{
			DebuggerPrintf("Compilation failed with errors:\n%hs\n",
				(const char*)errorsBlob->GetBufferPointer());
		}
		else
		{
			DebuggerPrintf("Shader Compiled successfully");
		}
	}
	ComPtr<IDxcBlob> code;
	result->GetResult(&code);
	return code;
}
IDxcBlob* ShaderCompiler::Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, unsigned long long numargs)
{
	UNUSED((void)args);
	UNUSED((void)numargs);
	UNUSED((void)sourceBlob);
	//DxcBuffer buffer;
	//buffer.Ptr = sourceBlob->GetBufferPointer();
	//buffer.Size = sourceBlob->GetBufferSize();
	//buffer.Encoding = DXC_CP_ACP;

	//HRESULT hr{ S_OK };
	//ComPtr<IDxcResult> result;
	//hr = m_compiler->Compile(&buffer, args, numargs, m_handler.Get(), IID_PPV_ARGS(&result));

	//ComPtr<IDxcBlobUtf8> pErrors = nullptr;
	//result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);

	//if (pErrors && pErrors->GetStringLength())
	//{
	//	DebuggerPrintf("\n Shader Compilation error : \n");
	//	DebuggerPrintf(pErrors->GetStringPointer());
	//}
	//else
	//{
	//	DebuggerPrintf("Shader Compiled Successfully");
	//}
	//HRESULT status{ S_OK };
	//hr = result->GetStatus(&status);

	////if (FAILED(hr) || (FAILED(status))) { ERROR_AND_DIE("Shader Compilation Failed"); }

	//ComPtr<IDxcBlob> finalShaderOutput{ nullptr };
	//hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&finalShaderOutput), nullptr);

	//if (FAILED(hr)) { ERROR_AND_DIE("Shader Compilation Failed") };

	//return finalShaderOutput.Detach();
	return nullptr;
}

ShaderD12* RendererD12::CreateOrGetShader(const char* shaderName, const char* shaderFilePath, bool containsTesselation)
{
	for (int i = 0; i < m_loadedShaders.size(); i++)
	{
		if (m_loadedShaders[i]->m_config.m_name == shaderName)
		{
			return m_loadedShaders[i];
		}
	}
	ShaderConfigD12 shaderConfig;
	shaderConfig.m_name = shaderName;
	shaderConfig.m_shaderFilePath = shaderFilePath;
	shaderConfig.m_isTesselated = containsTesselation;

	ShaderD12* shaderD12 = new ShaderD12(shaderConfig, this);

	shaderD12->CreateShaderObjects();
	m_loadedShaders.push_back(shaderD12);
	return shaderD12;
}
ShaderD12* RendererD12::GetShader(const char* shaderName)
{
	for (int i = 0; i < m_loadedShaders.size(); i++)
	{
		if (m_loadedShaders[i]->m_config.m_name == shaderName)
		{
			return m_loadedShaders[i];
		}
	}

	return nullptr;
}
void RendererD12::BindShader(ShaderD12* shader)
{
	auto commandList = m_RcommandList.Get();
	m_currentShader = shader;
	commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
	commandList->SetGraphicsRootSignature(shader->m_rootSignature.Get());
	if (shader->GetShaderType() == ShaderDetails::Shader3D)
	{
		commandList->SetGraphicsRootConstantBufferView((UINT)Default3DRootSignatureParams::CameraConstantBuffer, m_cameraCB.GpuVirtualAddress(m_frameIndex));
		commandList->SetGraphicsRootConstantBufferView((UINT)Default3DRootSignatureParams::GameConstantBuffer, m_gameDataCB.GpuVirtualAddress(m_frameIndex));
	}
	else
	{
		commandList->SetGraphicsRootConstantBufferView((UINT)DefaultRootSignatureParams::CameraConstantBuffer, m_cameraCB.GpuVirtualAddress(m_frameIndex));
	}
	commandList->SetPipelineState(shader->m_pipelineStateObject.Get());
}
BitmapFont* RendererD12::CreateBitmapFont(const char* fontFilePath)
{
	std::string fontFilePathWithExtension = std::string(fontFilePath) + ".png";
	Texture* bitmapFontTexture = nullptr;
	BitmapFont* newFont = new BitmapFont(fontFilePath, *bitmapFontTexture, IntVec2(256, 256));
	return newFont;
}

//-----------------------------GET FUNCTIONS-------------------------------
ID3D12Device* RendererD12::GetDevice()
{
	return m_Rdevice.Get();
}
ComPtr<ID3D12GraphicsCommandList> RendererD12::GetCommandListComPtr()
{
	return m_RcommandList;
}
ID3D12GraphicsCommandList* RendererD12::GetCommandList()
{
	return m_RcommandList.Get();
}
ID3D12Resource* RendererD12::GetBackBuffer()
{
	return m_backBufferRenderTarget[m_frameIndex].Get();
}
D3D12_CPU_DESCRIPTOR_HANDLE* RendererD12::GetBackBufferCPUHandle()
{
	return &m_backBufferHeapCPUHandle[m_frameIndex];
}
D3D12_CPU_DESCRIPTOR_HANDLE RendererD12::GetDepthStencilViewHandle()
{
	return m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}
D3D12_CPU_DESCRIPTOR_HANDLE* RendererD12::GetImguiHandle()
{
	return &m_resourceManager->m_GpuresourceBuffers[(UINT)GBufferResources::Count].cpuDescriptorHandle;
}
ID3D12Resource* RendererD12::GetImGuiBackBuffer()
{
	return m_imguiBackBufferRenderTarget[m_frameIndex].Get();
}
ID3D12CommandAllocator* RendererD12::GetCommandAllocator()
{
	return m_RcommandAllocator[m_frameIndex].Get();
}
void RendererD12::ResetCommandAllocator()
{
	auto commandList = m_RcommandList.Get();
	auto commandAllocator = m_RcommandAllocator[m_frameIndex].Get();
	commandList->Reset(commandAllocator, nullptr);
}

D3D12_CPU_DESCRIPTOR_HANDLE RendererD12::CreateAndGetImGuiCPUDescriptorHandle()
{
	/*AllocateDescriptor(m_imguiDescriptorHeap.Get(), &m_imGUICpuHandle, 0);*/
	return m_imGUICpuHandle;
}
D3D12_CPU_DESCRIPTOR_HANDLE RendererD12::GetIMGUIRenderTarget()
{
	return m_ImguirenderTargetViewHandle;
}
D3D12_GPU_DESCRIPTOR_HANDLE RendererD12::GetGPUDescriptorHandle(ID3D12DescriptorHeap* heap, int offsetindex)
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), offsetindex, m_descriptorSize);
	return handle;
}
ID3D12DescriptorHeap* RendererD12::GetDescriptorHeap()
{
	return m_descriptorHeap.Get();
}
ResourceManager* RendererD12::GetResourceManager()
{
	return m_resourceManager;
}
ID3D12DescriptorHeap* RendererD12::GetIMGUIDescriptorHeap()
{
	return m_imguiDescriptorHeap.Get();
}
D3D12_GPU_DESCRIPTOR_HANDLE RendererD12::GetOutputResourceGPUHandle()
{
	return m_resourceManager->GetRaytracingOutputBuffer();
}
int RendererD12::GetFrameIndex()
{
	return m_frameIndex;
}

void RendererD12::RaytracingAssets()
{
	//float windowWidth  = GetRenderConfig().m_window->GetClientDimensions().x;
	//float windowHeight  = GetRenderConfig().m_window->GetClientDimensions().y;
	//m_Rviewport = CD3DX12_VIEWPORT(0.0f, 0.0f, windowWidth, windowHeight);
	//m_RscissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(windowWidth), static_cast<LONG>(windowHeight));

	////ROOT SIGNATURES
	//CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//ComPtr<ID3DBlob> signature;
	//ComPtr<ID3DBlob> error;
	//ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error), "Failed While Serializing Root Signature");
	//ThrowIfFailed(m_Rdevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RrootSignature)), "Failed While Creating Root Signature");

	////COMPILING AND LOADING SHADERS
	//ComPtr<ID3DBlob> vertexShader;
	//ComPtr<ID3DBlob> pixelShader;

	//#if defined(ENGINE_DEBUG_RENDER)
	//// Enable better shader debugging with the graphics debugging tools.
	//UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	//#else
	//UINT compileFlags = 0;
	//#endif

	//std::string shaderNameWithExtension = "Data/Shaders/RaytracingUtilities.hlsl";
	//std::string shaderValues;
	//FileReadToString(shaderValues, shaderNameWithExtension);

	//std::vector<unsigned char> vertexByte = {};
	//std::vector<unsigned char> pixelByte = {};

	//CompileShaderToByteCode(vertexByte, shaderNameWithExtension.c_str(), shaderValues.c_str(), "VSMain", "vs_5_0");
	//CompileShaderToByteCode(pixelByte, shaderNameWithExtension.c_str(), shaderValues.c_str(), "PSMain", "ps_5_0");

	////D3DReadFileToBlob((LPCWSTR)L"Data/Shaders/RaytracingUtilities.hlsl", &vertexShader);
	////CD3DX12_SHADER_BYTECODE vertexShaderByte;
	////vertexShaderByte.BytecodeLength = vertexShader->GetBufferSize();
	////vertexShaderByte.pShaderBytecode = vertexShader->GetBufferPointer();

	//D3DReadFileToBlob((LPCWSTR)L"Data/Shaders/RaytracingUtilities.hlsl", &pixelShader);
	//CD3DX12_SHADER_BYTECODE pixelShaderByte;
	//pixelShaderByte.BytecodeLength = pixelShader->GetBufferSize();
	//pixelShaderByte.pShaderBytecode = pixelShader->GetBufferPointer();
	//
	//ThrowIfFailed(D3DCompileFromFile((LPCWSTR)L"Data/Shaders/RaytracingUtilities.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr),
	//"Failed While Creating Vertex Shader");
	//ThrowIfFailed(D3DCompileFromFile((LPCWSTR)L"Data/Shaders/RaytracingUtilities.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr),
	//"Failed While Creating Pixel Shader");

	//// Define the vertex input layout.
	//D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	//};

	//// Describe and create the graphics pipeline state object (PSO).
	//D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	//psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	//psoDesc.pRootSignature = m_RrootSignature.Get();
	//psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	//psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	//psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState.DepthEnable = FALSE;
	//psoDesc.DepthStencilState.StencilEnable = FALSE;
	//psoDesc.SampleMask = UINT_MAX;
	//psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//psoDesc.NumRenderTargets = 1;
	//psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//psoDesc.SampleDesc.Count = 1;
	//ThrowIfFailed(m_Rdevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_RpipelineState)), "Failed While Creating Pipeline State Object");

	//// Create the command list.
	//ThrowIfFailed(m_Rdevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_RcommandAllocator.Get(), m_RpipelineState.Get(), IID_PPV_ARGS(&m_RcommandList)), "Failed While Creating Command List");

	//// Command lists are created in the recording state, but there is nothing
	//// to record yet. The main loop expects it to be closed, so close it now.
	//ThrowIfFailed(m_RcommandList->Close(), "Failed While Closing Command List");

	//Vertex triangleVertices[] =
	//{
	//	{ { 0.0f, 0.25f * 2, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	//	{ {0.25f, -0.25f * 2, 0.0f }, {0.0f, 1.0f, 0.0f, 1.0f } },
	//	{ {-0.25f, -0.25f * 2, 0.0f }, {0.0f, 0.0f, 1.0f, 1.0f } }
	//};

	//const UINT vertexBufferSize = sizeof(triangleVertices);
	//CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	//CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	//// Note: using upload heaps to transfer static data like vert buffers is not 
	//// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	//// over. Please read up on Default Heap usage. An upload heap is used here for 
	//// code simplicity and because there are very few verts to actually transfer.
	//ThrowIfFailed(m_Rdevice->CreateCommittedResource(
	//	&heapProp,
	//	D3D12_HEAP_FLAG_NONE,
	//	&resourceDesc,
	//	D3D12_RESOURCE_STATE_GENERIC_READ,
	//	nullptr,
	//	IID_PPV_ARGS(&m_RvertexBuffer)), "Failed while Creating Committed Resource");

	//// Copy the triangle data to the vertex buffer.
	//UINT8* pVertexDataBegin;
	//CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	//ThrowIfFailed(m_RvertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)), "Failed While Reading");
	//memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
	//m_RvertexBuffer->Unmap(0, nullptr);

	//// Initialize the vertex buffer view.
	//m_RvertexBufferView.BufferLocation = m_RvertexBuffer->GetGPUVirtualAddress();
	//m_RvertexBufferView.StrideInBytes = sizeof(Vertex);
	//m_RvertexBufferView.SizeInBytes = vertexBufferSize;

	//// Create synchronization objects and wait until assets have been uploaded to the GPU.
	//ThrowIfFailed(m_Rdevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Failed Creating Fence");
	//m_fenceValue = 1;

	//// Create an event handle to use for frame synchronization.
	//m_fenceEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	//if (m_fenceEventHandle == nullptr)
	//{
	//	ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "Failed Creating Fence");
	//}

	//// Wait for the command list to execute; we are reusing the same command 
	//// list in our main loop but for now, we just want to wait for setup to 
	//// complete before continuing.
	//WaitForPreviousFrame();
}
UINT RendererD12::CreateBufferSRV(GpuBuffer* buffer, UINT numElements, UINT elementSize, DXGI_FORMAT format, D3D12_BUFFER_SRV_FLAGS flags)
{
	auto device = m_Rdevice.Get();

	// SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Format = format;
	srvDesc.Buffer.Flags = flags;
	srvDesc.Buffer.StructureByteStride = elementSize;
	UINT descriptorIndex = AllocateDescriptor(&buffer->cpuDescriptorHandle);
	device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuDescriptorHandle);
	buffer->gpuReadDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);
	return descriptorIndex;
}
void RendererD12::CreateTextureSRV(TextureD12* texture)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = texture->m_textureSRVDimension;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texture->m_textureResource->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = texture->m_textureResource->GetDesc().MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	texture->m_heapIndex = AllocateDescriptor(&texture->m_cpuDescriptorHandle, texture->m_heapIndex);
	m_Rdevice->CreateShaderResourceView(texture->m_textureResource.Get(), &srvDesc, texture->m_cpuDescriptorHandle);
	texture->m_gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), texture->m_heapIndex, m_descriptorSize);
}
UINT RendererD12::AllocateImguiDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
	auto descriptorHeapCpuBase = m_imguiDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	if (descriptorIndexToUse >= m_imguiDescriptorHeap->GetDesc().NumDescriptors)
	{
		descriptorIndexToUse = m_imguiDescriptorsAllocated++;
	}
	*cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_RrtvDescriptorSize);
	return descriptorIndexToUse;
}
UINT RendererD12::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
	auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	UINT numDesc = m_descriptorHeap->GetDesc().NumDescriptors;
	if (descriptorIndexToUse >= numDesc)
	{
		descriptorIndexToUse = m_descriptorsAllocated++;
	}
	*cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
	return descriptorIndexToUse;
}
UINT RendererD12::AllocateDepthDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
	auto descriptorHeapCpuBase = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	UINT numDepthDescriptors = m_dsvDescriptorHeap->GetDesc().NumDescriptors;
	if (descriptorIndexToUse >= numDepthDescriptors)
	{
		descriptorIndexToUse = m_depthDescriptorsAllocated++;
	}
	*cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_depthDescriptorSize);
	return descriptorIndexToUse;
}
UINT RendererD12::AllocateRenderTargetDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
	auto descriptorHeapCpuBase = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	UINT numDescriptors = m_rtvDescriptorHeap->GetDesc().NumDescriptors;
	if (descriptorIndexToUse >= numDescriptors)
	{
		descriptorIndexToUse = m_rtvDescriptorsAllocated++;
	}
	*cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_depthDescriptorSize);
	return descriptorIndexToUse;
}
UINT RendererD12::AllocateDescriptor(ID3D12DescriptorHeap* heap, D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT& allocatedDescriptors, UINT descriptorIndexToUse)
{
	auto descriptorHeapCpuBase = heap->GetCPUDescriptorHandleForHeapStart();
	if (descriptorIndexToUse >= heap->GetDesc().NumDescriptors)
	{
		descriptorIndexToUse = allocatedDescriptors++;
	}
	*cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
	return descriptorIndexToUse;
}
void RendererD12::CopyTextureResourceFromBuffer(GpuBuffer* source, GpuBuffer* dest)
{
	auto commandList = GetCommandList();
	D3D12_RESOURCE_BARRIER preCopyBarriers[] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(source->GetResource(), source->m_UsageState, D3D12_RESOURCE_STATE_COPY_SOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(dest->GetResource(), dest->m_UsageState, D3D12_RESOURCE_STATE_COPY_DEST)
	};

	commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

	CD3DX12_TEXTURE_COPY_LOCATION srcTexture(source->GetResource());
	CD3DX12_TEXTURE_COPY_LOCATION destTexture(dest->GetResource());

	CD3DX12_BOX box = CD3DX12_BOX(0, 0, m_dimensions.x, m_dimensions.y);

	commandList->CopyTextureRegion(&destTexture, 0, 0, 0, &srcTexture, &box);

	D3D12_RESOURCE_BARRIER postCopyBarriers[] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(source->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, source->m_UsageState),
		CD3DX12_RESOURCE_BARRIER::Transition(dest->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, dest->m_UsageState)
	};
	commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}
void RendererD12::CopyTextureResourceFromBuffer(GpuBuffer* source, GpuBuffer* dest, IntVec2 dimensionsToCopy)
{
	auto commandList = GetCommandList();
	D3D12_RESOURCE_BARRIER preCopyBarriers[] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(source->GetResource(), source->m_UsageState, D3D12_RESOURCE_STATE_COPY_SOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(dest->GetResource(), dest->m_UsageState, D3D12_RESOURCE_STATE_COPY_DEST)
	};

	commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

	CD3DX12_TEXTURE_COPY_LOCATION srcTexture(source->GetResource());
	CD3DX12_TEXTURE_COPY_LOCATION destTexture(dest->GetResource());

	CD3DX12_BOX box = CD3DX12_BOX(0, 0, dimensionsToCopy.x, dimensionsToCopy.y);

	commandList->CopyTextureRegion(&destTexture, 0, 0, 0, &srcTexture, &box);

	D3D12_RESOURCE_BARRIER postCopyBarriers[] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(source->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, source->m_UsageState),
		CD3DX12_RESOURCE_BARRIER::Transition(dest->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, dest->m_UsageState)
	};
	commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}
void RendererD12::RenderFrame()
{
	//// Record all the commands we need to render the scene into the command list.
	//PopulateCommandList();
	//// Execute the command list.
	//ID3D12CommandList* ppCommandLists[] = { m_RcommandList.Get() };
	//m_RcommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//// Present the frame.
	//ThrowIfFailed(m_RswapChain->Present(1, 0), "Failed While Presenting");

	//WaitForPreviousFrame();
}
void RendererD12::PopulateCommandList()
{
	//ThrowIfFailed(m_RcommandAllocator->Reset(), "Failed While Resetting Command Allocator");
	//ThrowIfFailed(m_RcommandList->Reset(m_RcommandAllocator.Get(), m_RpipelineState.Get()), "Failed While Resetting command List");

	////SET NECESSARY STATES
	//m_RcommandList->SetGraphicsRootSignature(m_RrootSignature.Get());
	//m_RcommandList->RSSetViewports(1, &m_Rviewport);
	//m_RcommandList->RSSetScissorRects(1, &m_RscissorRect);

	////INDICATING BACK BUFFER TO BE USED AS RENDER TARGET
	//CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_RrenderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	//m_RcommandList->ResourceBarrier(1, &barrier);

	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_RrtvDescriptorSize);
	//m_RcommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	//const float ClearColor[] = {0.0f,0.2f,0.1f,1.0f};
	//m_RcommandList->ClearRenderTargetView(rtvHandle, ClearColor, 0, nullptr);
	//m_RcommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_RcommandList->IASetVertexBuffers(0, 1, &m_RvertexBufferView);
	//m_RcommandList->DrawInstanced(3, 1, 0, 0);

	////INDICATE BACK BUFFER TO BE PRESENTED
	//CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_RrenderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	//m_RcommandList->ResourceBarrier(1, &barrier2);
	//ThrowIfFailed(m_RcommandList->Close(), "Failed While Closing Command List");
}
void RendererD12::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_RcommandQueue->Signal(m_fence.Get(), fence), "Failed Command queue Signal");
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEventHandle), "Failed Fence event");
		WaitForSingleObject(m_fenceEventHandle, INFINITE);
	}

	m_frameIndex = m_RswapChain->GetCurrentBackBufferIndex();
}
void RendererD12::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; pFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}
		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}
	*ppAdapter = adapter.Detach();
}
inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
{
	if (path == nullptr)
	{
		throw std::exception();
	}

	DWORD size = GetModuleFileName(nullptr, path, pathSize);
	if (size == 0 || size == pathSize)
	{
		// Method failed or path was truncated.
		throw std::exception();
	}

	WCHAR* lastSlash = wcsrchr(path, L'\\');
	if (lastSlash)
	{
		*(lastSlash + 1) = L'\0';
	}
}
std::wstring RendererD12::GetAssetFullPath(LPCWSTR assetName)
{
	std::wstring m_assetsPath = L"Run/Data/Shaders";
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath));
	m_assetsPath = assetsPath;
	return m_assetsPath + assetName;
}

//---------------------------------IMGUI-----------------------------
void RendererD12::CreateIMGUIRenderTarget()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.NumDescriptors = m_backBufferCount;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ThrowIfFailed(m_Rdevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&m_imguiDescriptorHeap)), "Failed while creating descriptor heap");
	/*for (UINT n = 0; n < m_backBufferCount; n++)
	{
		ThrowIfFailed(m_RswapChain->GetBuffer(n, IID_PPV_ARGS(&m_imguiBackBufferRenderTarget[n])), "Failed while getting render targets");

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = m_backBufferFormat;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_imguiDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), n, m_RrtvDescriptorSize);
		m_Rdevice->CreateRenderTargetView(m_imguiBackBufferRenderTarget[n].Get(), &rtvDesc, rtvDescriptor);
	}*/

}
void RendererD12::SetupImGuiRenderTarget()
{
	auto device = m_Rdevice.Get();
	auto commandList = m_RcommandList.Get();
	unsigned int renderTargetViewDescriptorSize;
	float color[4];
	m_ImguirenderTargetViewHandle = m_imguiDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetViewDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	if (m_frameIndex == 0)
	{
		m_ImguirenderTargetViewHandle.ptr += renderTargetViewDescriptorSize;
	}

	// Set the back buffer as the render target.
	commandList->OMSetRenderTargets(m_frameIndex, &m_ImguirenderTargetViewHandle, FALSE, NULL);
	// Then set the color to clear the window to.
	color[0] = 0.5;
	color[1] = 0.5;
	color[2] = 0.5;
	color[3] = 1.0;
	commandList->ClearRenderTargetView(m_ImguirenderTargetViewHandle, color, 0, NULL);
}
