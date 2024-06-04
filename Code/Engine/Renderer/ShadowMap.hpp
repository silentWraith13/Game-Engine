#pragma once
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Math/IntVec2.hpp"

class ShadowMap
{
	friend class RendererD12;
	public:
	ShadowMap(RendererD12* renderer,ID3D12Device* device,UINT width, UINT height);

	ShadowMap(const ShadowMap& shadowMap)=delete;
	ShadowMap& operator=(const ShadowMap& shadowMap)=delete;
	~ShadowMap();

	UINT				 GetWidth() { return m_width; };
	UINT				 GetHeight() { return m_height; } ;
	IntVec2				 GetDimensions() {return IntVec2(m_width , m_height);}
	D3D12_VIEWPORT		 GetViewport() {return m_viewport;} ;
	D3D12_RECT			 ScissorRect() {return m_scissorRect;} ;

	public:
	void				 BuildBuffer();
	GpuBuffer*			 GetShaderResourceBuffer();
	void				 UpdateCameraPosition(Vec3 position, EulerAngles orientation);
	public:
	ID3D12Device*			m_device = nullptr;
	D3D12_VIEWPORT			m_viewport;
	D3D12_RECT				m_scissorRect;
	GpuBuffer				m_shadowBuffer;
	UINT					m_shadowFenceValue = 0;
	RendererD12*			m_renderer;
	UINT					m_width = 0;
	UINT					m_height = 0;
	DXGI_FORMAT				m_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	Camera					m_shadowCamera;

};