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
class GBuffer
{
	friend class Renderer;

public:
	GBuffer(int width, int height);
	GBuffer(const GBuffer& copy) = delete;
	~GBuffer();

private:
	int							m_width;
	int							m_height;
	ID3D11Texture2D*			m_textures[3];
	ID3D11RenderTargetView*		m_RTV[3];
	ID3D11ShaderResourceView*	m_SRV[3];
	ID3D11DepthStencilView*		m_DSV;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------