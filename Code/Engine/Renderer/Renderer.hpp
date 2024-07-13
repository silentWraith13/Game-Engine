#pragma once
#include "Engine/Renderer/camera.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
#define DX_SAFE_RELEASE(dxObject) \
{								  \
	if ((dxObject) != nullptr)	  \
	{							  \
		(dxObject)->Release();	  \
		(dxObject) = nullptr;	  \
	}							  \
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

//Dx11 declarations
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11Texture2D;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;

//My engine's Dx11 component classes
class  Texture;
class  BitmapFont;
class  ConstantBuffer;
class  VertexBuffer;
class  IndexBuffer;
class  Shader;
class  Image;
struct ShaderConfig;
class  UnorderedAccessBuffer;
class  RenderTexture;
class  GBuffer;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
#if defined(OPAQUE)
#undef OPAQUE
#endif

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct RendererConfig
{

};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class BlendMode
{
	OPAQUE,
	ALPHA,
	ADDITIVE,
	COUNT
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class SamplerMode
{
	POINT_CLAMP,
	BILINEAR_WRAP,
	COUNT
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class RasterizerMode
{
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	SOLID_CULL_BACK_FRONT_CW,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class DepthMode
{
	DISABLED,
	ENABLED,
	COUNT
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class VertexType
{
	PCU,
	PNCU,
	PCUTBN,
	COUNT
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Renderer
{
public:
	Renderer(RendererConfig const& config);
	~Renderer();
	
	//Various Renderer functions
	void					Startup();
	void					BeginFrame();
	void					Endframe();
	void					Shutdown();

	void					ClearScreen(const Rgba8& clearColor);
	void					BeginCamera(const Camera& camera);
	void					EndCamera(const Camera& camera);

	//PCU draw functions
	void					DrawVertexArray(int numVertexes, const Vertex_PCU* vertexes);

	//Draw using vertex and index buffers
	void					DrawVertexArrayUsingLinePrimitives(int numVertexes, VertexBuffer* vbo);
	void					DrawVertexBuffer(int numVertexes, VertexBuffer* vbo,int numIndices, IndexBuffer* ibo);

	//PNCU draw functions
	void					DrawVertexArray(int numVertexes, const Vertex_PNCU* vertexes);

	//Texture functions
	void					BindTexture(const Texture* texture, unsigned int slot = 0);
	void					BindTextures(const Texture* texture0, const Texture* texture1, const Texture* texture2);
	Texture*				CreateTextureFromFile(char const* imageFilePath);
	Texture*				CreateOrGetTextureFromFile(char const* imageFilePath);
	Texture*				GetTextureForFileName(char const* imageFilePath);
	Texture*				CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData);
	Texture*				CreateTextureFromImage(const Image& image);
	void					DestroyTexture(Texture* texture);

	//Bitmap functions
	BitmapFont*				GetBitmapForFileName(char const* bitmapFontFilePath);
	BitmapFont*				CreateOrGetBitmapFontFromFile(char const* bitmapFontFilePathWithoutExtension);
	BitmapFont*				CreateBitmapFontFromFile(char const* bitmapFontFilePath);

	RendererConfig const&	GetConfig() const { return m_config; }

	//Shader functions
	void					BindShader(Shader* shader);
	Shader*					GetShaderByName(const char* shaderName);
	Shader*					CreateShader(char const* shaderName, char const* source, VertexType vertexType = VertexType::PCU);
	Shader*					CreateShaderOrGetFromFile(char const* shaderName, VertexType vertexType = VertexType::PCU);
	Shader*					CreateComputeShaderOnly(char const* shaderName, char const* entryPoint);
	bool					CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target);

	//Vertex Buffer functions
	VertexBuffer*			CreateVertexBuffer(const size_t numVerts, const unsigned int stride);
	void					CopyCPUToGPU(const void* data, size_t size, VertexBuffer*& vbo);
	void					BindVertexBuffer(VertexBuffer* vbo);
	void					DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset = 0);
	void					DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, IndexBuffer* ibo, int indexCount);

	//Index buffer functions
	IndexBuffer*			CreateIndexBuffer(const size_t numIndexes, const unsigned int stride);
	void					CopyCPUToGPU(const void* data, size_t size, IndexBuffer*& ibo);
	void					BindIndexBuffer(IndexBuffer* ibo);
	void					DrawIndexBuffer(IndexBuffer* ibo, int indexCount, int indexOffset = 0);

	//Constant Buffer functions
 	ConstantBuffer*			CreateConstantBuffer(const size_t size);
	void					CopyCPUtoGPU(const void* data, size_t size, ConstantBuffer* cbo);
	void					BindConstantBuffer(int slot, ConstantBuffer* cbo);
	void					BindConstantBufferWithGS(int slot, ConstantBuffer* cbo);

	//Compute shader functions
	UnorderedAccessBuffer*	CreateUnorderedAccessBuffer(const size_t numElements, const unsigned int stride, void* data);
	void					VSSetShaderResource(int slot, int numViews, UnorderedAccessBuffer* uav);
	void					IASetVertexBuffers( VertexBuffer* vbo);
	void					Draw(int numVertexes);
	void					IASetPrimitiveTopology();
	void					CSSetUnorderedAccessViews(int numSlot, UnorderedAccessBuffer* uav);
	void					CSSetShaderResources(int slot, int numViews, UnorderedAccessBuffer* uav);
	void					CSSetShader(Shader* cs);
	void					CSSetConstantBuffers(int startSlot, int numBuffers, ConstantBuffer* cb);
	void					DispatchCS(int threadX, int threadY, int threadZ);
	void					CopyResource(UnorderedAccessBuffer* toCopyTo, UnorderedAccessBuffer* toCopyFrom);

	//Set constants for modeL and light constant buffer
	void					SetModelConstants(const Mat44& modelMatrix = Mat44(), const Rgba8& modelColor = Rgba8(255, 255, 255, 255));
	void					SetLightConstants(Vec3 sunDirection, float sunIntensity, float ambientIntensity, Vec3 worldEyePos, int normalMode, int specularMode, float specularIntensity, float specularPower);
	void					SetPointLightConstants(Vec3 Position, float intensity, Rgba8 color, float radius);

	//Creating and setting blend states
	void					CreateBlendStates();
	void					SetBlendMode(BlendMode blendMode);

	//Creating and setting sampler modes
	void					CreateSamplerModes();
	void					SetSamplerModes(SamplerMode samplerMode);

	//Creating and setting the rasterizer modes
	void					CreateRasterizerModes();
	void					SetRasterizerModes(RasterizerMode rasterizerMode);

	//Creating and setting the depth modes
	void					CreateDepthModes();
	void					SetDepthModes(DepthMode depthMode);

	//Set DX11 states when changed
	void					SetStatesIfChanged();
	
	//Render Texture
	RenderTexture*			CreateRenderTexture(int width, int height);
	void					SetRenderTarget(RenderTexture* renderTexture);
	void					ClearRenderTarget(RenderTexture* renderTexture, Rgba8 clearColor);
	Texture*				CreateOrGetTextureFromNothing(std::string name, IntVec2 dimensions,ID3D11Texture2D* textureR, ID3D11ShaderResourceView* shaderResourceView);
	void					SetBackBufferRenderTarget();
	void					BindRenderTexture(RenderTexture* texture, unsigned int slot = 0);

	//Cubemap
	Texture*				CreateCubeMapFomFile(char const* imagePath);
	Texture*				CreateCubemapFromImages(std::vector<Image> const& images);
	Texture*				CreateCubemap(std::vector<Image> const& images);

	//Deferred Rendering - GBuffer

public:
	Vec2						bottomLeft;
	Vec2						topRight;

	std::vector<Texture*>		m_loadedTextures;
	std::vector<BitmapFont*>	m_loadedFonts;

	RendererConfig				m_config;

	void*						m_dxgiDebugModule;
	void*						m_dxgiDebug;

	std::vector<Shader*>		m_loadedShaders;			 
	Shader const*				m_currentShader = nullptr;
	Shader*						m_defaultShader = nullptr;
	
	ID3D11RenderTargetView*		m_renderTargetView = nullptr;
	ID3D11Texture2D*			m_backbuffer = nullptr;
	ID3D11Device*				m_device = nullptr;
	ID3D11DeviceContext*		m_deviceContext = nullptr;
	IDXGISwapChain*				m_swapChain = nullptr;
	ID3D11RasterizerState*		m_rasterizerState = nullptr;
	ID3D11BlendState*			m_blendState = nullptr;
	ID3D11SamplerState*			m_samplerState = nullptr;
	ID3D11DepthStencilState*	m_depthStencilState = nullptr;
	ID3D11DepthStencilView*		m_depthStencilView = nullptr;
	ID3D11Texture2D*			m_depthStencilTexture = nullptr;
	
	VertexBuffer*				m_immediateVBO;
	VertexBuffer*				m_immediatePNCUVBO;
	ConstantBuffer*				m_cameraCBO = nullptr;
	ConstantBuffer*				m_modelCBO = nullptr;
	ConstantBuffer*				m_lightCBO = nullptr;
	ConstantBuffer*				m_pointLightCBO = nullptr;
	
	const Texture*				m_defaultTexture = nullptr;

	BlendMode					m_desiredBlendMode = BlendMode::ALPHA;
	SamplerMode					m_desiredSamplerMode = SamplerMode::POINT_CLAMP;
	RasterizerMode				m_desiredRasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	DepthMode					m_desiredDepthMode = DepthMode::ENABLED;

	ID3D11BlendState*			m_blendStates[(int)(BlendMode::COUNT)] = {};
	ID3D11SamplerState*			m_samplerStates[(int)(SamplerMode::COUNT)] = {};
	ID3D11RasterizerState*		m_rasterizerStates[(int)(RasterizerMode::COUNT)] = {};
	ID3D11DepthStencilState*	m_depthStencilStates[(int)(DepthMode::COUNT)] = {};

	GBuffer*					m_gBuffer;
};

