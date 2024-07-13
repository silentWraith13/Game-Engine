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
class ShadowMap
{
	friend class Renderer;

public:
	ShadowMap(int width, int height);
	ShadowMap(const ShadowMap& copy) = delete;
	~ShadowMap();

private:
	int							m_width;
	int							m_height;
	ID3D11Texture2D*			m_texture;
	ID3D11DepthStencilView*		m_DSV;
	ID3D11ShaderResourceView*	m_SRV;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------