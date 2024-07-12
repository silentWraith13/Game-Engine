#pragma once

//--------------------------------------------------------------------------------------------------------------------------------------------------------
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
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class RenderTexture
{
	friend class Renderer;
public:
	RenderTexture( int width, int height);
	RenderTexture(const RenderTexture& copy) = delete;
	virtual ~RenderTexture();

	ID3D11Texture2D*			m_renderTargetTexture;
	ID3D11RenderTargetView*		m_renderTargetView;
	ID3D11ShaderResourceView*	m_shaderResourceView;
	int							m_width;
	int							m_height;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------