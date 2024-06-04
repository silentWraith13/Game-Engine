#include "Engine/Renderer/ShadowMap.hpp"
#include "Engine/Renderer/RendererD12.hpp"

ShadowMap::ShadowMap(RendererD12* renderer,ID3D12Device* device, UINT width, UINT height)
{
	m_device = device;
	m_width = width;
	m_height = height;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = (float)width;
	m_viewport.Height = (float)height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = (int)width;
	m_scissorRect.bottom = (int)height;

	//BuildDepthResource();
	//BuildShaderResource();
	m_renderer = renderer;
	BuildBuffer();
	Vec3 ibasis = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 jbasis = Vec3(-1.0f, 0.0f, 0.0f);
	Vec3 kbasis = Vec3(0.0f, 1.0f, 0.0f);
	m_shadowCamera.SetViewToRenderTransform(ibasis, jbasis, kbasis);
	m_shadowCamera.SetPerspectiveView(2.0f, 60.0f, 0.1f, 1000.0f);
}

ShadowMap::~ShadowMap()
{
	m_shadowBuffer.ResetResource();
}


void ShadowMap::BuildBuffer()
{
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = GetDimensions().x;
	depthStencilDesc.Height = GetDimensions().y;
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
	m_renderer->ThrowIfFailed(m_renderer->m_Rdevice->CreateCommittedResource(&resource, D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthClear,
		IID_PPV_ARGS(&m_shadowBuffer.resource)
	), "Failed while getting render targets");

	//------------------------UAV-----------------------
	/*D3D12_CPU_DESCRIPTOR_HANDLE& handleUAV = m_shadowBuffer.cpuDescriptorHandle;
	m_shadowBuffer.uavHeapIndex = m_renderer->AllocateDescriptor(&handleUAV, UINT_MAX);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	m_renderer->m_Rdevice->CreateUnorderedAccessView(m_shadowBuffer.GetResource(), nullptr, &UAVDesc, handleUAV);
	m_shadowBuffer.gpuWriteDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_renderer->m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_shadowBuffer.uavHeapIndex, m_renderer->m_descriptorSize);
	m_shadowBuffer.m_UsageState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;*/

	//--------------------------------SRV------------------------------
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	UINT heapIndex = UINT_MAX;
	heapIndex = m_renderer->AllocateDescriptor(&m_shadowBuffer.cpuDescriptorHandle, heapIndex);
	m_renderer->m_Rdevice->CreateShaderResourceView(m_shadowBuffer.resource.Get(), &srvDesc, m_shadowBuffer.cpuDescriptorHandle);
	m_shadowBuffer.m_UsageState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	m_shadowBuffer.gpuReadDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_renderer->m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), heapIndex, m_renderer->m_descriptorSize);

}

GpuBuffer* ShadowMap::GetShaderResourceBuffer()
{
	return &m_shadowBuffer;
}
void ShadowMap::UpdateCameraPosition(Vec3 position, EulerAngles orientation)
{
	m_shadowCamera.SetTransform(position, orientation);
}


