#pragma once	

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Renderer/RaytracingHelpers.hpp"
#include "Engine/Renderer/TextureD12.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/AccelerationStructureHelper.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "ThirdParty/D3D12DXR/d3dx12.h"
//#include "ThirdParty/D3D12DXR/DDSTextureLoader.h"
#include "ThirdParty/D3D12DXR/WICTextureLoader.h"
#include "ThirdParty/D3D12DXR/ResourceUploadBatch.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include <wrl.h>
#include <string.h>
#include <stdlib.h>
#include <DirectXMath.h>




using namespace Microsoft::WRL;

class ResourceManager;
class Denoiser;
class ShadowMap;
class ShaderD12;
class GlobalIllumination;
class Composition;
class PostProcess;
struct IDxcBlob;
struct IDxcBlobEncoding;
struct IDxcCompiler;
struct IDxcLibrary;
struct IDxcUtils;
struct IDxcIncludeHandler;
struct IDXGIDebug;
//struct DataForChunkBuild
//{
//	std::vector<Vertex_PNCU> verts;
//	std::vector<UINT> indices;
//	GpuBuffer* vertexBuffer; 
//	GpuBuffer* indexBuffer;
//	int instanceIndex;
//};

struct RenderItems
{
	VertexBufferD12<Vertex_PCUTBN> verticesPNCUTB;
	VertexBufferD12<Vertex_PCU> verticesPCU;
	IndexBufferD12<unsigned int> indices;
	ID3D12Fence* fence;
	UINT64 fenceValue = 1;
	HANDLE fenceEvent;
};
enum class RenderingPipeline
{
	Rasterization,
	Raytracing
};

enum class CompiledShaderByteCodes
{
	Raytracing3D = 0,
	GaussianFilter,
	Count
};

struct RaytracedPointLights
{
	Vec4 PointLightPosition[20];
	int	 Counter = -1;
	int  MaxLights = 20;
};

class ShaderCompiler
{

	public:
	ShaderCompiler();
	~ShaderCompiler();
	
	ComPtr<IDxcBlob>  Compile(const char* ShaderFilePath);
	ComPtr<ID3DBlob>  CompileVsPs(const char* filePath, const D3D_SHADER_MACRO* defines, const std::string& entryPoint, const std::string& target);
	ComPtr<IDxcBlob>  CompileComputeShader(const char* ShaderFilePath);
	IDxcBlob*		  Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, unsigned long long numargs);

	public:
		//CD3DX12_SHADER_BYTECODE					m_shaderByteCodes[(int)CompiledShaderByteCodes::Count];

	private:
		ComPtr<IDxcCompiler>					m_compiler;
		ComPtr<IDxcLibrary>						m_utils;
		ComPtr<IDxcIncludeHandler>				m_handler;

};
typedef UINT Index;
typedef DirectX::XMMATRIX XMMATRIX;
typedef DirectX::XMVECTOR XMVECTOR;
typedef DirectX::XMFLOAT4 XMFLOAT4;
typedef DirectX::XMFLOAT2 XMFLOAT2;
typedef DirectX::XMFLOAT3 XMFLOAT3;

struct IrradianceCache
{
	Vec3 vertexPosition;
	Vec3 vertexNormal;
	Vec4 GI;
};

struct Vertex
{
	Vec4 position;
	Vec3 normal;
	Vec4 color;
	Vec2 uv;
};

enum class DefaultRootSignatureParams {

	//----SRV---
	DiffuseTexture,
	//---CBV-----
	CameraConstantBuffer,
	Count
};

enum class Default3DRootSignatureParams {

	//----SRV---
	DiffuseTexture,
	ShadowMapTexture,
	//---CBV-----
	CameraConstantBuffer,
	GameConstantBuffer,
	Count
};

struct CameraConstantBuffer
{
	Mat44 projectionMatrix;
	Mat44 viewMatrix;
	Mat44 lightViewMatrix;
	Mat44 lightProjMatrix;
	Vec4 cameraPosition;
	Vec4 additionalData;
};
struct GameDataBuffer
{
	Vec4 globalLightPosition;
	Vec4 globalLightDirection;
	Vec4 globalLightColor;
	Vec4 ViewX_GIOnY_ShadowPassZ;
	float  renderOutput; // ------- 0 final output 1 Normals 
};

struct SceneConstantBuffer
{
	Mat44 inversedProjectionMatrix;
	Mat44 projectionMatrix;
	Mat44 inversedViewMatrix;
	Mat44 viewMatrix;
	Mat44 _viewMatrix;
	Mat44 inversedViewMatrixOrigin;
	Vec4 cameraPosition;
	Vec4 _cameraPosition;
	Vec4 lightPosition;
	Vec4 GIColor;
	Vec4 samplingData;
	Vec4  lightBools;
	Vec4 textureMappings;
	Vec4 lightfallOff_AmbientIntensity_CosineSampling;
};
struct CubeConstantBuffer
{
	Vec4 albedo;
};
enum class GlobalRootSignatureParams {
	GBufferVertexPositionSlot,
	GBufferVertexNormalSlot,
	GBufferVertexAlbedoSlot,
	GBufferMotionVectorSlot,
	GBufferDirectLightSlot,
	GBufferDepthSlot,
	GBufferOcclusionSlot,
	GBufferVariance, // --------JUST TO CLEAR IT-----------
	VertexBuffersSlot,
	TextureBufferSlot,
	NormalMapBufferSlot,
	SpecularMapTextureSlot,
	SkyboxTextureSlot,
	AccelerationStructureSlot,
	SceneConstantSlot,
	Count
};
enum class LocalRootSignatureParams
{
	IndexBuffer = 0,
	VertexBuffer = 1,
	Count
};

struct LocalRootArgumentsGeometry {
	D3D12_GPU_DESCRIPTOR_HANDLE m_indexBufferGPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_vertexBufferGPUHandle;
};

struct BottomLevelAccelerationStructureInstanceDesc : public D3D12_RAYTRACING_INSTANCE_DESC
{
	void SetTransform(const DirectX::XMMATRIX& transform);
	void GetTransform(DirectX::XMMATRIX* transform);
};
static_assert(sizeof(BottomLevelAccelerationStructureInstanceDesc) == sizeof(D3D12_RAYTRACING_INSTANCE_DESC), L"This is a wrapper used in place of the desc. It has to have the same size");


namespace TextureRP {
	enum Value {
		TextureSRV,
		TextureSampler,
		ConstantBuffer,
		RootParameterCount
	};
}

enum class GBufferResources
{
	OutputResource,
	VertexPosition,
	VertexNormal,
	VertexAlbedo,
	MotionVector,
	GI,
	DirectLight,
	HistoryLength,
	Depth,
	Moments,
	DenoiserInput,
	PartialDerivates,
	OcclusionTexture,
	VertexIndirectAlbedo,
	CompositorOutput,
	Count
};


enum class Scenes
{
	Minecraft,
	Bunny
};

enum class GBufferResourcesPreviousFrame
{
	VertexNormal, 
	Depth,
	GI,
	Moments,
	History,
	Count
};

struct AccelerationStructureBuffers
{
	ComPtr<ID3D12Resource> scratch;
	ComPtr<ID3D12Resource> accelerationStructure;
	ComPtr<ID3D12Resource> instanceDesc;    // Used only for top-level AS
	UINT64                 ResultDataMaxSizeInBytes;
	bool				   isValid;
};


enum class DepthTestD12
{
	ALWAYS,
	NEVER,
	EQUAL,
	NOTEQUAL,
	LESS,
	LESSEQUAL,
	GREATER,
	GREATEREQUAL,
};

enum class ShaderType
{
	VertexOrPixelShader,
	ComputeShader,
	RaytracingShader
};


struct RendererD12Config
{
	Window* m_window;
	bool m_VR = false;
	bool m_isFullscreen = false;
	bool m_isVsyncEnabled = false;
	bool m_isRaytracingEnabled = false;
};

struct Viewport
{
	float left;
	float top;
	float right;
	float bottom;
};


struct RayGenConstantBuffer
{
	Viewport viewport;
	Viewport stencil;
};

class RendererD12
{
	public:
		RendererD12();
		RendererD12(RendererD12Config const& rendererConfig);

	public:
		 //---------------------- MAIN FUNCTIONS -----------------------------
		 void			Startup();
		 void			BeginFrame();
		 void			EndFrame();
		 void			ShutDown();
		 void			RenderFrame();

		 //-----------------DXR  RAYTRACING FUNCTIONS----------------------------------------
		 void			D3D12InterfaceInitialization();
		 void			InitializeDenoising();
		 void			CreateRootSignatures();
		 void			InitializeComposition();
		 void			InitializePostProcess();
		 void			InitializeAdapterAndCheckRaytracingSupport();
		 void			CreateRaytracingInterfaces();
		 void	    	CompileShaders();
		 void			CreateRaytracingPipelineStateObject();
		 void			InitializeBasicBottomLevelAS();
		 //void			InitializeSampler();
		 void			InitializeGlobalIllumination();
		 void			InitializeIrradianceCaching();
		 void			BuildGeometryAndAS(std::vector<Vertex_PCUTBN>& verts, std::vector<UINT>& indices, int index = 0);
		 void			BuildModelGeometryAndAS(std::vector<Vertex_PCUTBN>& verts, std::vector<UINT>& indices, int index);
		 void			BuildBLAS(std::vector<Vertex_PCUTBN>& verts, std::vector<UINT>& indices, int index);
		 void			BuildASInstances();
		 void			BuildTLAS();
		 void			AddPointLights(Vec4 pointLightPosition);
		 void			BuildGeometryAndASForChunk(std::vector<Vertex_PCUTBN>& verts, std::vector<UINT>& indices,  ComPtr<ID3D12Resource>& topLevel,
		 ComPtr<ID3D12Resource>& bottomLevel, GpuBuffer& vertexBuffer, GpuBuffer& indexBuffer, int instanceIndex);
		 void			BuildAS(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC ASDesc, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topASDesc,
						ID3D12Resource* bottomLevelAS);
		 void			BuildShaderTables();
		 void			CreateRaytracingOutputResources();
		 void			CreateIMGUIRenderTarget();
		 void			SetupImGuiRenderTarget();

		 //----------------------------MESH FUNCTIONS--------------------------
		 void			PopulateCommandList();
		 void			WaitForPreviousFrame();
		 void			GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
		 std::wstring	GetAssetFullPath(LPCWSTR assetName);
		 void			RaytracingAssets();

	
		 //-----------------TEXTURES-----------------------------
		 UINT			CreateBufferSRV(GpuBuffer* buffer, UINT numElements, UINT elementSize, DXGI_FORMAT format, D3D12_BUFFER_SRV_FLAGS flags);
		 void			CreateTextureSRV(TextureD12* texture);
		 UINT			AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
		 UINT			AllocateDepthDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
		 UINT			AllocateRenderTargetDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
		 UINT			AllocateImguiDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
		 UINT			AllocateDescriptor(ID3D12DescriptorHeap* heap, D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT& allocatedDescriptors, UINT descriptorIndexToUse = UINT_MAX);
		 void			CopyTextureResourceFromBuffer(GpuBuffer* source, GpuBuffer* dest);
		 void			CopyTextureResourceFromBuffer(GpuBuffer* source, GpuBuffer* dest , IntVec2 dimensionsToCopy);

		 TextureD12*	LoadTexture(std::string fileName, std::string filePath, TextureType type);
		 TextureD12*	GetTextureForFileName(char const* name);
		 TextureD12*	GetTextureForFileNameOrPath(char const* fileName, char const* imageFilePath);
		 TextureD12*	CreateTextureFromImage(const Image& image);;
		 TextureD12*	CreateOrGetTextureFromFile(char const* name, char const* imageFilePath);
		 TextureD12*	CreateTextureFromFile(char const* imageFilePath);
		 void	     	BindTexture(int index, TextureD12* textureToBind);
		 void	     	BindHandle(int index, D3D12_GPU_DESCRIPTOR_HANDLE& handle);
		 //----------------SHADERS----------------------
		 ShaderD12*		CreateOrGetShader(const char* shaderName, const char* shaderFilePath, bool containsTesselation = false);
		 ShaderD12*		GetShader(const char* shaderName);
		 void			BindShader(ShaderD12* shader);

		 //----------------BITMAP FONTS----------------------
		 BitmapFont*	CreateBitmapFont(const char* bitmapFontFilePathWithNoExtension);

		 //----------------CAMERA FUNCTIONS----------------------------
		 void			BeginCamera(const Camera& camera);
		 void			BeginRasterizerCamera(const Camera& camera);
		 void			EndCamera(const Camera& camera);
		 void			SetRaytraceQuadCamera(Vec3 topLeft, Vec3 bottomLeft, Vec3 topRight, Vec3 bottomRight);

		 //---------------------MAIN RENDER FUNCTIONS------------------------------
		 void			ClearScreen(Rgba8 color);
		 void			Present(Rgba8 color);
		 void			DrawVertexArray(int size, VertexNormalArray array);
		 void			DrawIndexedVertexArray(int numberOfVertices, std::vector<Vertex_PCUTBN>& verticesToDraw, std::vector<unsigned int>& indexes);
		 void			DrawVertexArray(int size, VertexArray array);
		 void			SetDepthStencilState(DepthTestD12 depthTest, bool writeDepth);

		 //---------------------RASTERIZATION FUNCTIONS--------------
		 void				InitializeRasterization();
		 void				BeginShadowMapRender();
		 void				EndShadowMapRender();
		 void				FinishUpGPUWork();
		// void				SetRasterizationState(CullModeD12 cullMode, FillModeD12 fillMode, WindingOrderD12 windingOrder);

		  //---------------------DXR FUNCTIONS----------------------
		 void			Prepare();
		 void			RunRaytracer();
		 void			RunGI();
		 void			RunDenoiser(bool temporalSamplerOn, bool denoiserOn);
		 void			RunCompositor(bool denoiserOn, bool godRaysOn = true);
		 void		    CopyRaytracingOutputToBackbuffer();
		 void		    FinishRaytraceCopyToBackBuffer();
		 void		    SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
		 void		    SerializeAndCreateRaytracingRootSignature(ID3D12Device5* device, D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
		 void			Present();
		 void			MoveToNextFrame();

		 //----------------GET FUNCTIONS----------------
		 RenderingPipeline					m_renderingPipeline = RenderingPipeline::Raytracing;
		 ID3D12Device*						GetDevice();
		 ID3D12GraphicsCommandList*			GetCommandList();
		 ComPtr<ID3D12GraphicsCommandList>  GetCommandListComPtr();
		 ID3D12Resource*					GetBackBuffer();
		 D3D12_CPU_DESCRIPTOR_HANDLE*		GetBackBufferCPUHandle();
		 D3D12_CPU_DESCRIPTOR_HANDLE*		GetImguiHandle();
		 ID3D12Resource*					GetImGuiBackBuffer();
		 ID3D12CommandAllocator*			GetCommandAllocator();
		 void								ResetCommandAllocator();
		 ID3D12DescriptorHeap*				GetDescriptorHeap();
		 ResourceManager*					GetResourceManager();
		 ID3D12DescriptorHeap*				GetIMGUIDescriptorHeap();
		 D3D12_CPU_DESCRIPTOR_HANDLE		CreateAndGetImGuiCPUDescriptorHandle();
		 D3D12_CPU_DESCRIPTOR_HANDLE		GetIMGUIRenderTarget();
		 D3D12_GPU_DESCRIPTOR_HANDLE		GetGPUDescriptorHandle(ID3D12DescriptorHeap* heap, int offsetindex);
		 D3D12_GPU_DESCRIPTOR_HANDLE		GetOutputResourceGPUHandle();
		 int								GetFrameIndex();
		 RendererD12Config					GetRenderConfig();
		 IDXGISwapChain3*					GetSwapChain();
		 D3D12_CPU_DESCRIPTOR_HANDLE		GetDepthStencilViewHandle();
		 void								PopulateSimpleCubeVertices();
		 void								AllocateUploadBuffer(ID3D12Device* pDevice, void* pData, UINT64 datasize, ID3D12Resource** ppResource, const wchar_t* resourceName);
		 void								AllocateUAVBuffer(ID3D12Device* pDevice, UINT64 bufferSize, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initialResourceState, const wchar_t* resourceName);
		 void								ExecuteCommandList(ComPtr<ID3D12GraphicsCommandList> m_commandList);
		 void								ExecuteCommandList(int commandsToExecute,ComPtr<ID3D12GraphicsCommandList> m_commandList);
		 void								WaitForGpu();
		 void								ThrowIfFailed(HRESULT hr, const char* msg);
		 void								ThrowIfFalse(bool result, const char* msg);

	//--------------------SCENE VARIABLES-----------------------
	public:	
		Scenes							 m_currentScene = Scenes::Minecraft;
		const static unsigned int		 MINECRAFTCHUNKS = 600;
		bool							 m_isFirstFrame = true;
		IntVec2							 m_dimensions;
		IntVec2							 m_windowDimensions;
		Camera							 m_currentCamera;
		SceneConstantBuffer				 m_gameValues;
		Vec4							 m_lightPosition;
		float							 angleToRotateBy  = 20.0f;
		ConstantBufferD12<SceneConstantBuffer>	m_sceneCB;
		ConstantBufferD12<RaytracedPointLights>	 m_lightCB;
		CubeConstantBuffer				 m_cubeCB;

		D3D12_CPU_DESCRIPTOR_HANDLE		 m_imGUICpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE		 m_imGUIGpuHandle;

		TextureD12						 m_minecraftTexture;
		float							m_dispatchRayRuntime;
		float							m_gpuWaitTime;
		std::vector<Vertex_PCUTBN>		m_DXRverts;
		std::vector<unsigned int>		m_DXRindexes;

		//----------------------------SAMPLING VARIABLES--------------------
		StructuredBuffer<AlignedHemisphereSample> m_hemisphereSamplesGPUBuffer;
		GlobalIllumination*						  m_globalIllumination = nullptr;

		//-----------------------------IRRADIANCE CACHING VARIABLES----------
		StructuredBuffer<IrradianceCache>					m_irradianceCacheGPUBuffer;
		//-----------------------------RESOURCE MANAGEMENT---------------
		ResourceManager*						 m_resourceManager = nullptr;
		//----------------------------DENOISING VARIABLES-------------------
		Denoiser*								  m_denoiser = nullptr;
		Composition*						      m_compositor = nullptr;
		PostProcess*						      m_postProcess = nullptr;
	public:
		//------------------VARIABLES SPECIFIC TO DX12 DXR---------------------------
		std::vector<TextureD12*>		 m_loadedTextures;
		std::vector<ShaderD12*>		     m_loadedShaders;
		ShaderD12*						 m_currentShader;
		ShaderCompiler*					 m_shaderCompiler = nullptr;
		UINT							 m_frameIndex;
		float							 m_temporalFade = 0.0f;
		//-----DESCRIPTOR VARIABLES------------------
		UINT							 m_descriptorsAllocated;
		UINT							 m_depthDescriptorsAllocated;
		UINT							 m_rtvDescriptorsAllocated;
		UINT							 m_imguiDescriptorsAllocated;
		UINT							 m_descriptorSize;
		UINT							 m_depthDescriptorSize;
		UINT							 m_imGuidescriptorSize;
		UINT							 m_RrtvDescriptorSize;
		ComPtr<ID3D12DescriptorHeap>     m_dsvDescriptorHeap;
		ComPtr<ID3D12DescriptorHeap>	 m_descriptorHeap;	
		ComPtr<ID3D12DescriptorHeap>	 m_imguiDescriptorHeap;	
		ComPtr<ID3D12DescriptorHeap>	 m_rtvDescriptorHeap;

		ComPtr<IDXGISwapChain3>			 m_RswapChain;
		ComPtr<ID3D12Device>			 m_Rdevice;
		ComPtr<ID3D12Device5>			 m_dxrDevice;
		ComPtr<IDXGIFactory4>            m_dxgiFactory;
		UINT                             m_adapterIDoverride;
		UINT                             m_adapterID;
		ComPtr<IDXGIAdapter1>            m_adapter;
		std::wstring                     m_adapterDescription;
		D3D_FEATURE_LEVEL                m_d3dMinFeatureLevel = D3D_FEATURE_LEVEL_11_0;
		const static UINT               m_backBufferCount = 2;
		ComPtr<ID3D12CommandAllocator>   m_RcommandAllocator[m_backBufferCount];
		
		ComPtr<ID3D12CommandQueue>		 m_RcommandQueue;
		ComPtr<ID3D12GraphicsCommandList>m_RcommandList;
		ComPtr<ID3D12GraphicsCommandList4>m_dxrCommandList;
		ComPtr<ID3D12Resource>			 m_RvertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW		 m_RvertexBufferView;
		GpuBuffer						 m_indexBuffer;
		GpuBuffer						 m_vertexBuffer;
		VertexBufferD12<Vertex_PNCU>	 m_rasterizedVertices;
		std::vector<RenderItems>		 m_staticRenderItems;
		std::vector<RenderItems>		 m_dynamicRenderItems;
		GpuBuffer						 m_bunnyVertexBuffer;
		GpuBuffer						 m_bunnyIndexBuffer;
		D3D12_VIEWPORT                   m_screenViewport;
		D3D12_RECT                       m_scissorRect;
		ComPtr<ID3D12Resource>			 m_backBufferRenderTarget[m_backBufferCount] = {};
		GpuBuffer						 m_uselessRenderTarget;

		IDXGIDebug*						m_dxgiDebug = nullptr;
		void*								m_dxgiDebugModule = nullptr;
		GpuBuffer						 m_depthStencilBuffer;
		D3D12_RASTERIZER_DESC			 m_rasterizerDesc;
		CD3DX12_CPU_DESCRIPTOR_HANDLE	 m_backBufferHeapCPUHandle[m_backBufferCount] = {};

		ComPtr<ID3D12Resource>			 m_imguiBackBufferRenderTarget[m_backBufferCount] = {};
		DXGI_FORMAT                      m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT                      m_depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		
		

		//------------------ACCELERATION STRUCTURE VARIABLES------------------------
		//RaytracingASManager*			 m_raytracingASManager = nullptr;
		//std::vector<BLASGeometry>		 m_geomtries;
		//------------------VARIABLES SPECIFIC TO D3D12---------------------------								
		D3D12_CPU_DESCRIPTOR_HANDLE		m_renderTargetViewHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE		m_ImguirenderTargetViewHandle;
		D3D12_RESOURCE_STATES			m_beforeState;
		HANDLE							m_fenceEventHandle;
		unsigned long long				m_fenceValue;
		D3D_FEATURE_LEVEL				m_featureLevel = D3D_FEATURE_LEVEL_12_1;

		//----------------PRESENTATION FENCE OBJECTS-----------------------
		ComPtr<ID3D12Fence>								 m_fence;
		UINT64                                           m_fenceValues[m_backBufferCount];
		Microsoft::WRL::Wrappers::Event                  m_fenceEvent;

		//------------------------DXR RAYTRACING VARIABLES-----------------
		ComPtr<ID3D12RootSignature>				m_raytracingGlobalRootSignature;
		ComPtr<ID3D12RootSignature> 			m_raytracingLocalRootSignature;
		ComPtr<ID3D12StateObject>				m_dxrStateObject;
		ComPtr<ID3D12Resource>					m_bottomLevelAccelerationStructure;
		ComPtr<ID3D12Resource>					m_topLevelAccelerationStructure;

		ComPtr<ID3D12Resource>					m_bunnyBlas;
		ComPtr<ID3D12Resource>					m_bunnyTlas;

		bool									m_isTopLevelASRebuildRequired= false;
		AccelerationStructureBuffers			m_bottomLevelAccelerationStructureBuffers[MINECRAFTCHUNKS];
		//D3D12_GPU_DESCRIPTOR_HANDLE				m_raytracingOutputResourceUAVGpuDescriptor;
		//D3D12_CPU_DESCRIPTOR_HANDLE				m_raytracingOutputResourceUAVCpuDescriptor;
		//UINT									m_raytracingOutputResourceUAVDescriptorHeapIndex = UINT_MAX;

		//-------------------------SHADER TABLE VARIABLES------------------------
		bool									m_isShaderTableBuilt = false;
		static const wchar_t*					c_hitGroupName;
		static const wchar_t*					c_raygenShaderName;
		static const wchar_t*					c_closestHitShaderName;
		static const wchar_t*					c_missShaderName;
		ShaderTable                             HitShaderTable;
		ShaderTable                             RaygenShaderTable;
		ShaderTable                             MissShaderTable;
		UINT									m_hitGroupShaderTableStrideInBytes = UINT_MAX;
		UINT									m_raygenGroupShaderTableStrideInBytes = UINT_MAX;
		void*									rayGenShaderID = nullptr;
		void*								    missShaderID = nullptr;
		void*									hitGroupShaderID = nullptr;
		StructuredBuffer<BottomLevelAccelerationStructureInstanceDesc> m_bottomLevelASInstanceDescs;


		//------------------------RASTERIZATION VARIABLES-----------------
		ConstantBufferD12<CameraConstantBuffer>	m_cameraCB;
		ConstantBufferD12<GameDataBuffer>		m_gameDataCB;
		ShadowMap*								m_shadowMap;

		
	private:
		RayGenConstantBuffer					m_rayGenCB;
		RendererD12Config						m_renderConfig;



};


