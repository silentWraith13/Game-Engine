#pragma once

#include "Engine/Renderer/RendererD12.hpp"

struct RaytracerPayload
{
    float tHit;
    Vec3 startDirection;
    Vec4 color;
    float reflectionIndex;
    float raytype; // 0 is Camera, 1 is radiance , 2 is shadow 
    Vec3 lastHitPosition;
    Vec3 lastHitNormal;
    bool didHitGeometry;
    bool didHitEmissiveSurface;
    bool didHitSky;
    Vec2 padding;
};

class ResourceManager
{
public:
    ~ResourceManager() 
    {
        FlushResourceBarriers();
        m_denoiserOutput.resource.Reset();
    }
    void Bind(ID3D12GraphicsCommandList* commandList) ;
    void Reset();
    void CreateGBufferResource(DXGI_FORMAT format, IntVec2 resourceDimensions, D3D12_RESOURCE_STATES startingResource, D3D12_RESOURCE_FLAGS flags, UINT handleIndex, LPCWSTR name = L"",bool isPreviousFrameResource = false);
    void CreateDenoiserGBufferResource(DXGI_FORMAT format, IntVec2 resourceDimensions, D3D12_RESOURCE_FLAGS flags,LPCWSTR name = L"Denoiser Buffer");
    void CreateGBufferResource(GpuBuffer* buffer,DXGI_FORMAT format, IntVec2 resourceDimensions, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state,  LPCWSTR name = L"", bool allowWrite = false);
    void TransitionResource(GpuBuffer* Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = true);
    void BeginResourceTransition(GpuBuffer* Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = true);
    void InsertUAVBarrier(GpuBuffer* Resource, bool FlushImmediate = false);
    void FlushResourceBarriers();
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetRaytracingOutputBuffer();
    GpuBuffer* GetRaytracingOutputBufferResource();
    GpuBuffer* GetDenoiserOutputResource();

public:
    //ComPtr<ID3D12Resource>					             m_raytracingOutput[7];
    //CD3DX12_GPU_DESCRIPTOR_HANDLE	                     m_raytracingGBufferGPUHandles[7];

    GpuBuffer                                            m_GpuresourceBuffers[15];
    GpuBuffer                                            m_previousFrameGpuResourceBuffers[7];
    GpuBuffer                                            m_denoiserOutput;
    static const UINT                                    m_maxBarriers = 16;
    D3D12_RESOURCE_BARRIER                               m_ResourceBarrierBuffer[m_maxBarriers];
    UINT                                                 m_currentBarriers = 0;
};