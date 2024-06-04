
#include "Engine/Renderer/ResourceManager.hpp"
#include "Engine/Renderer/RaytracingHelpers.hpp"

extern RendererD12* g_theRendererDx12;

void ResourceManager::Bind(ID3D12GraphicsCommandList* commandList)
{
    (void)commandList;
   // m_commandList = commandList;
}

void ResourceManager::Reset()
{
    m_currentBarriers = 0;
    for (int i = 0; i < 8; i++)
    {
        m_GpuresourceBuffers[i].resource.Reset();
    }
}

void ResourceManager::TransitionResource(GpuBuffer* Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate)
{
    D3D12_RESOURCE_STATES OldState = Resource->m_UsageState;
    if (m_currentBarriers == m_maxBarriers)
        FlushResourceBarriers();

    
    if (OldState != NewState)
    {
        D3D12_RESOURCE_BARRIER& barrierDesc = m_ResourceBarrierBuffer[m_currentBarriers];
        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrierDesc.Transition.pResource = Resource->GetResource();
        barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrierDesc.Transition.StateAfter = NewState;
        barrierDesc.Transition.StateBefore = OldState;

        if (OldState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS || NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
            InsertUAVBarrier(Resource);

        if (NewState == Resource->m_TransitioningState)
        {
            barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            Resource->m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
        }
        else
            barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        Resource->m_UsageState = NewState;
    }
  /*  else if (NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        InsertUAVBarrier(Resource);*/
        
    D3D12_RESOURCE_DESC desc = Resource->resource.Get()->GetDesc();
   /* D3D12_RESOURCE_STATES currentState = desc.ResourceStates;*/
    if (FlushImmediate)
        FlushResourceBarriers();
}

void ResourceManager::InsertUAVBarrier(GpuBuffer* Resource, bool FlushImmediate)
{
    if (m_currentBarriers == m_maxBarriers)
        FlushResourceBarriers();

    m_currentBarriers += 1;
    D3D12_RESOURCE_BARRIER& barrierDesc = m_ResourceBarrierBuffer[m_currentBarriers];

    barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrierDesc.UAV.pResource = Resource->GetResource();

    if (FlushImmediate)
        FlushResourceBarriers();
}

void ResourceManager::FlushResourceBarriers()
{
    if (m_currentBarriers > 0)
    {
        g_theRendererDx12->m_RcommandList->ResourceBarrier(m_currentBarriers, m_ResourceBarrierBuffer);
        m_currentBarriers = 0;
    }
}

CD3DX12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetRaytracingOutputBuffer()
{
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_GpuresourceBuffers[(int) GBufferResources::OutputResource].gpuWriteDescriptorHandle);
}
GpuBuffer* ResourceManager::GetRaytracingOutputBufferResource()
{
    return &m_GpuresourceBuffers[(int)GBufferResources::OutputResource];
}
GpuBuffer* ResourceManager::GetDenoiserOutputResource()
{
    return &m_denoiserOutput;
}
void ResourceManager::CreateGBufferResource(DXGI_FORMAT format, IntVec2 resourceDimensions, D3D12_RESOURCE_STATES startingResource, D3D12_RESOURCE_FLAGS flags, UINT handleIndex, LPCWSTR name, bool isPreviousFrameResource)
{
    UNUSED((void)name);
    auto device = g_theRendererDx12->GetDevice();
    ID3D12DescriptorHeap* descriptorHeap = nullptr;
    descriptorHeap = g_theRendererDx12->GetDescriptorHeap();
   /* if (handleIndex == (UINT)GBufferResources::Count)
    {
        descriptorHeap = g_theRenderer->GetIMGUIDescriptorHeap();
    }
    else
    {
        descriptorHeap = g_theRenderer->GetDescriptorHeap();
    }*/
    if (!isPreviousFrameResource)
    {
        //------------------------CREATING UAV-----------------------
        auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDimensions.x,
            resourceDimensions.y, 1, 1, 1, 0, flags);

        auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        g_theRendererDx12->ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, startingResource, nullptr, IID_PPV_ARGS(&m_GpuresourceBuffers[handleIndex].resource)), "Failed While Getting Raytracing output");

        //Output Resource
        D3D12_CPU_DESCRIPTOR_HANDLE& handleUAV = m_GpuresourceBuffers[handleIndex].cpuDescriptorHandle;
        m_GpuresourceBuffers[handleIndex].uavHeapIndex = g_theRendererDx12->AllocateDescriptor(&handleUAV, UINT_MAX);

        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
        UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        device->CreateUnorderedAccessView(m_GpuresourceBuffers[handleIndex].GetResource(), nullptr, &UAVDesc, handleUAV);
        m_GpuresourceBuffers[handleIndex].gpuWriteDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_GpuresourceBuffers[handleIndex].uavHeapIndex, g_theRendererDx12->m_descriptorSize);
        m_GpuresourceBuffers[handleIndex].m_UsageState = startingResource;


        //------------------------CREATING SRV-----------------------
        D3D12_CPU_DESCRIPTOR_HANDLE handleSRV;
        m_GpuresourceBuffers[handleIndex].srvHeapIndex = g_theRendererDx12->AllocateDescriptor(&handleSRV, UINT_MAX);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = m_GpuresourceBuffers[handleIndex].resource->GetDesc().Format;
        srvDesc.Texture2D.MipLevels = m_GpuresourceBuffers[handleIndex].resource->GetDesc().MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        device->CreateShaderResourceView(m_GpuresourceBuffers[handleIndex].resource.Get(), &srvDesc, handleSRV);
        m_GpuresourceBuffers[handleIndex].gpuReadDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
            m_GpuresourceBuffers[handleIndex].srvHeapIndex, g_theRendererDx12->m_descriptorSize);
        m_GpuresourceBuffers[handleIndex].GetResource()->SetName(name);
    }
    else
    {
        auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDimensions.x,
            resourceDimensions.y, 1, 1, 1, 0, flags);

        auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        g_theRendererDx12->ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, startingResource, nullptr, IID_PPV_ARGS(&m_previousFrameGpuResourceBuffers[handleIndex].resource)), "Failed While Getting Raytracing output");

        //Output Resource
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
        m_previousFrameGpuResourceBuffers[handleIndex].srvHeapIndex = g_theRendererDx12->AllocateDescriptor(&handle, UINT_MAX);

       /* device->CreateUnorderedAccessView(m_previousFrameGpuResourceBuffers[handleIndex].GetResource(), nullptr, &UAVDesc, handle);
        m_previousFrameGpuResourceBuffers[handleIndex].gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), heapIndex, g_theRenderer->m_descriptorSize);
        m_previousFrameGpuResourceBuffers[handleIndex].m_UsageState = startingResource;
        m_GpuresourceBuffers[handleIndex].cpuDescriptorHandle = handle;*/

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = m_previousFrameGpuResourceBuffers[handleIndex].resource->GetDesc().Format;
        srvDesc.Texture2D.MipLevels = m_previousFrameGpuResourceBuffers[handleIndex].resource->GetDesc().MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        device->CreateShaderResourceView(m_previousFrameGpuResourceBuffers[handleIndex].resource.Get(), &srvDesc, handle);
        m_previousFrameGpuResourceBuffers[handleIndex].gpuReadDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
            m_previousFrameGpuResourceBuffers[handleIndex].srvHeapIndex, g_theRendererDx12->m_descriptorSize);
        m_previousFrameGpuResourceBuffers[handleIndex].m_UsageState = startingResource;
        m_previousFrameGpuResourceBuffers[handleIndex].GetResource()->SetName(name);
    }
}

void ResourceManager::CreateDenoiserGBufferResource(DXGI_FORMAT format, IntVec2 resourceDimensions, D3D12_RESOURCE_FLAGS flags, LPCWSTR name)
{
    UNUSED((void) name);

   // auto device = g_theRendererDx12->GetDevice();
   // auto descriptorHeap = g_theRendererDx12->GetDescriptorHeap();

    CreateGBufferResource(&m_denoiserOutput, format, resourceDimensions, flags, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, name, true);
}

void ResourceManager::CreateGBufferResource(GpuBuffer* buffer, DXGI_FORMAT format, IntVec2 resourceDimensions, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, LPCWSTR name, bool allowWrite)
{
    (void)(state);
    auto device = g_theRendererDx12->GetDevice();
    ID3D12DescriptorHeap* descriptorHeap = nullptr;
    descriptorHeap = g_theRendererDx12->GetDescriptorHeap();

    auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDimensions.x,
        resourceDimensions.y, 1, 1, 1, 0, flags);

    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    g_theRendererDx12->ThrowIfFailed(device->CreateCommittedResource(
        &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&buffer->resource)), "Failed While Getting Raytracing output");

    //------------SRV-------------------
    D3D12_CPU_DESCRIPTOR_HANDLE handle;
    buffer->srvHeapIndex = g_theRendererDx12->AllocateDescriptor(&handle, UINT_MAX);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = buffer->resource->GetDesc().Format;
    srvDesc.Texture2D.MipLevels  =  buffer->resource->GetDesc().MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, handle);
    buffer->gpuReadDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
        buffer->srvHeapIndex, g_theRendererDx12->m_descriptorSize);
    buffer->m_UsageState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    //---------------UAV-------------------
    if (allowWrite)
    {
        uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDimensions.x,
            resourceDimensions.y, 1, 1, 1, 0, flags);

       defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        g_theRendererDx12->ThrowIfFailed(device->CreateCommittedResource(
            &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&buffer->resource)), "Failed While Getting Raytracing output");

        //Output Resource
        D3D12_CPU_DESCRIPTOR_HANDLE handleUAV;
        buffer->uavHeapIndex = g_theRendererDx12->AllocateDescriptor(&handleUAV, UINT_MAX);

        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
        UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        device->CreateUnorderedAccessView(buffer->resource.Get(), nullptr, &UAVDesc, handleUAV);
        buffer->gpuWriteDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), buffer->uavHeapIndex, g_theRendererDx12->m_descriptorSize);
    }
    buffer->GetResource()->SetName(name);

}