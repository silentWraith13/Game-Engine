#pragma once

#include <unordered_map>
#include <d3d12.h>
#include "ThirdParty/D3D12DXR/d3dx12.h"
#include <Engine/Core/ErrorWarningAssert.hpp>
#include <d3d12.h>
#include "Engine/Math/Vec3.hpp"
#include "RaytracingHelpers.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"

//#include <atlbase.h>


struct AlignedHemisphereSample
{
    Vec3 sampleValue;
    float padding;
};
inline UINT Align(UINT size, UINT alignment)
{
    return (size + (alignment - 1)) & ~(alignment - 1);
}
class GpuUploadBuffer
{
public:
    Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() { return m_resource; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetDefaultBuffer() { return m_defaultBuffer; }

public:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_resource; // This is the upload buffer for Vertex buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> m_defaultBuffer;

    GpuUploadBuffer() {}
    ~GpuUploadBuffer()
    {
        if (m_resource.Get())
        {
           //m_resource->Unmap(0, nullptr);
        }
    }

    void ResetResources()
    {
        m_defaultBuffer.Reset();
        m_resource.Reset();
    }
    void Allocate(ID3D12Device* device, UINT bufferSize, LPCWSTR resourceName = nullptr)
    {
        auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        if (FAILED(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_resource))))
        {
            ERROR_AND_DIE("Failed while creating committed resource");
        }

        m_resource->SetName(resourceName);
    }

    uint8_t* MapCpuWriteOnly()
    {
        uint8_t* mappedData;
        // We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        if (FAILED(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedData))))
        {
            ERROR_AND_DIE("Failed while creating committed resource");
        }
        return mappedData;
    }
};

struct GpuBuffer
{
    enum ReadWriteFlags {
        None = 0x0,
        AllowRead = 0x1,
        AllowWrite = 0x2,
    };

    UINT rwFlags = ReadWriteFlags::AllowRead | ReadWriteFlags::AllowWrite;
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuReadDescriptorHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuWriteDescriptorHandle;
    UINT srvHeapIndex = UINT_MAX;
    UINT uavHeapIndex = UINT_MAX;
    D3D12_RESOURCE_STATES m_UsageState = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_STATES m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
    ID3D12Resource* GetResource() {return resource.Get();}
    Microsoft::WRL::ComPtr<ID3D12Resource> GetResourceComPtr() {return resource;}
    D3D12_DEPTH_STENCIL_DESC depthDesc;

    void ResetResource()
    {
        resource.Reset();
    }
};


class ShaderRecord
{
public:
    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize) :
        shaderData(pShaderIdentifier, shaderIdentifierSize)
    {
    }

    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize, void* pLocalRootArguments, UINT localRootArgumentsSize) :
        shaderData(pShaderIdentifier, shaderIdentifierSize),
        localRootArguments(pLocalRootArguments, localRootArgumentsSize)
    {
    }

    void CopyTo(void* dest) const;
    void CopyToIndex(void* dest) const;

    struct PointerWithSize 
    {
        void* ptr;
        UINT size;
        PointerWithSize() : ptr(nullptr), size(0) {}
        PointerWithSize(void* _ptr, UINT _size) : ptr(_ptr), size(_size) {};
    };
    PointerWithSize shaderData;
    PointerWithSize localRootArguments;
};

class ShaderTable : public GpuUploadBuffer
{
public:
    ShaderTable() {}
    ~ShaderTable() {ResetResources();}
    ShaderTable(ID3D12Device* device, UINT numShaderRecords, UINT shaderRecordSize, LPCWSTR resourceName = nullptr);
    void push_back(const ShaderRecord& shaderRecord);
    void AddShaderRecordOrPushback(const ShaderRecord& shaderRecord, int instanceIndex);
    void AddShaderRecordAtIndex(const ShaderRecord& shaderRecord, UINT instanceIndex);
    UINT GetShaderRecordSize() { return m_shaderRecordSize; }
    void DebugPrint();
public:
    uint8_t* m_mappedShaderRecords;
    UINT m_shaderRecordSize;
    UINT  m_totalNumShaderRecords = 0;
    std::wstring m_name;
    std::vector<ShaderRecord> m_shaderRecords;
};

template <class T>
class StructuredBuffer : public GpuUploadBuffer
{
    T* m_mappedBuffers;
    std::vector<T> m_staging;
    UINT m_numInstances;

public:
    
    StructuredBuffer() : m_mappedBuffers(nullptr), m_numInstances(0) {}

    void Create(ID3D12Device* device, UINT numElements, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
    {
        m_numInstances = numInstances;
        m_staging.resize(numElements);
        UINT bufferSize = numInstances * numElements * sizeof(T);
        Allocate(device, bufferSize, resourceName);
        m_mappedBuffers = reinterpret_cast<T*>(MapCpuWriteOnly());
    }
    void CopyFromCPUToGPU(UINT instanceIndex = 0)
    {
        memcpy(m_mappedBuffers + instanceIndex * NumElements(), &m_staging[0], InstanceSize());
    }
    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0, UINT elementIndex = 0)
    {
        return m_resource->GetGPUVirtualAddress() + instanceIndex * InstanceSize() + elementIndex * ElementSize();
    }

    T& operator[](UINT elementIndex) { return m_staging[elementIndex]; }
    const T& operator[](UINT elementIndex) const { return m_staging[elementIndex]; }
    size_t NumElements() const { return m_staging.size(); }
    size_t InstanceSize() const { return NumElements() * ElementSize(); }
    UINT ElementSize() const { return sizeof(T); }
    UINT NumInstances() const { return m_numInstances; }
};

template <class T>
class ConstantBufferD12 : public GpuUploadBuffer
{
    uint8_t* m_mappedConstantData;
    UINT m_alignedInstanceSize;
    UINT m_numInstances;

public:
    ConstantBufferD12() : m_alignedInstanceSize(0), m_numInstances(0), m_mappedConstantData(nullptr) {}

    void Create(ID3D12Device* device, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
    {
        m_numInstances = numInstances;
        m_alignedInstanceSize = Align(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        UINT bufferSize = numInstances * m_alignedInstanceSize;
        Allocate(device, bufferSize, resourceName);
        m_mappedConstantData = MapCpuWriteOnly();
    }

    void CopyStagingToGpu(UINT instanceIndex = 0)
    {
        memcpy(m_mappedConstantData + instanceIndex * m_alignedInstanceSize, &staging, sizeof(T));
    }

    // Accessors
    // Align staging object on 16B boundary for faster mempcy to the memory returned by Map()
    //alignas(16) T staging;
    T staging;
    T* operator->() { return &staging; }
    UINT NumInstances() { return m_numInstances; }
    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
    {
        return m_resource->GetGPUVirtualAddress() + instanceIndex * m_alignedInstanceSize;
    }
};


template <class T>
class VertexBufferD12 : public GpuUploadBuffer
{
    public:
    uint8_t* m_mappedConstantData;
    UINT m_alignedInstanceSize;
    UINT m_numInstances;

     VertexBufferD12() : m_alignedInstanceSize(0), m_numInstances(0), m_mappedConstantData(nullptr) {}
     
    void CreateDefaultBuffer(ID3D12Device* device,ID3D12GraphicsCommandList* cmdList, void* vertexData, UINT bufferSize, UINT numInstances = 1)
    {
		m_numInstances = numInstances;
		//m_alignedInstanceSize = Align(sizeof(T), D3D12_VERTEX_BUFFER_VIEW);

		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		if (FAILED(device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(m_defaultBuffer.GetAddressOf()))))
		{
			ERROR_AND_DIE("Failed while creating committed resource");
		}

		m_defaultBuffer->SetName(L"Vertex shader default buffer");

		uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		if (FAILED(device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_resource.GetAddressOf()))))
		{
			ERROR_AND_DIE("Failed while creating committed resource");
		}

		m_resource->SetName(L"Vertex shader upload buffer");

		D3D12_SUBRESOURCE_DATA subresource = {};
		subresource.pData = vertexData;
		subresource.RowPitch = bufferSize;
		subresource.SlicePitch = subresource.RowPitch;

		CD3DX12_RESOURCE_BARRIER barr = CD3DX12_RESOURCE_BARRIER::Transition(m_defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		cmdList->ResourceBarrier(1, &barr);
		UpdateSubresources<1>(cmdList, m_defaultBuffer.Get(), m_resource.Get(), 0, 0, 1, &subresource);
		barr = CD3DX12_RESOURCE_BARRIER::Transition(m_defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdList->ResourceBarrier(1, &barr);

		//---------------MAPPING RESOURCE TO GPU-----------
		CD3DX12_RANGE readRange(0, 0);
		(m_resource.Get())->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantData));
		memcpy(m_mappedConstantData, vertexData, bufferSize);
		m_resource.Get()->Unmap(0, nullptr);
    }

    T staging;
    T* operator->() { return &staging; }
    UINT NumInstances() { return m_numInstances; }
    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
    {
        return m_resource->GetGPUVirtualAddress() + instanceIndex * m_alignedInstanceSize;
    }
    UINT ElementSize() const { return sizeof(T); }

    D3D12_VERTEX_BUFFER_VIEW CreateAndGetVertexBufferView(const UINT vByteSize)
    {
        //--------------CREATING VERTEX BUFFER VIEW------------
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = m_resource->GetGPUVirtualAddress();
        vbv.StrideInBytes = ElementSize();
        vbv.SizeInBytes = vByteSize;
        return vbv;
    }
	//void Create(ID3D12Device* device, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
	//{
	//	m_numInstances = numInstances;
	//	m_alignedInstanceSize = Align(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	//	UINT bufferSize = numInstances * m_alignedInstanceSize;
	//	Allocate(device, bufferSize, resourceName);
	//	m_mappedConstantData = MapCpuWriteOnly();
	//}

	void CopyStagingToGpu(UINT instanceIndex = 0)
	{
		memcpy(m_mappedConstantData + instanceIndex * m_alignedInstanceSize, &staging, sizeof(T));
	}

};

template <class T>
class IndexBufferD12 : public GpuUploadBuffer
{
public:
    uint8_t* m_mappedConstantData;
    UINT m_alignedInstanceSize;
    UINT m_numInstances;

    IndexBufferD12() : m_alignedInstanceSize(0), m_numInstances(0), m_mappedConstantData(nullptr) {}

    void CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, void* data, UINT bufferSize, UINT numInstances = 1)
    {
        m_numInstances = numInstances;
        //m_alignedInstanceSize = Align(sizeof(T), D3D12_VERTEX_BUFFER_VIEW);

        auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        if (FAILED(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(m_defaultBuffer.GetAddressOf()))))
        {
            ERROR_AND_DIE("Failed while creating committed resource");
        }

        m_defaultBuffer->SetName(L"Index default Buffer");

        uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        if (FAILED(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_resource.GetAddressOf()))))
        {
            ERROR_AND_DIE("Failed while creating committed resource");
        }

        m_resource->SetName(L"Index Upload Buffer");

        D3D12_SUBRESOURCE_DATA subresource = {};
        subresource.pData = data;
        subresource.RowPitch = bufferSize;
        subresource.SlicePitch = subresource.RowPitch;

        CD3DX12_RESOURCE_BARRIER barr = CD3DX12_RESOURCE_BARRIER::Transition(m_defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
        cmdList->ResourceBarrier(1, &barr);
        UpdateSubresources<1>(cmdList, m_defaultBuffer.Get(), m_resource.Get(), 0, 0, 1, &subresource);
        barr = CD3DX12_RESOURCE_BARRIER::Transition(m_defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        cmdList->ResourceBarrier(1, &barr);

        //---------------MAPPING RESOURCE TO GPU-----------
        CD3DX12_RANGE readRange(0, 0);
        (m_resource.Get())->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantData));
        memcpy(m_mappedConstantData, data, bufferSize);
        m_resource.Get()->Unmap(0, nullptr);
    }

    T staging;
    T* operator->() { return &staging; }
    UINT NumInstances() { return m_numInstances; }
    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
    {
        return m_resource->GetGPUVirtualAddress() + instanceIndex * m_alignedInstanceSize;
    }
    UINT ElementSize() const { return sizeof(T); }

    D3D12_INDEX_BUFFER_VIEW CreateAndGetIndexBufferView(const UINT iByteSize)
    {
        //--------------CREATING VERTEX BUFFER VIEW------------
        D3D12_INDEX_BUFFER_VIEW ibv;
        ibv.BufferLocation = m_resource->GetGPUVirtualAddress();
        ibv.Format = DXGI_FORMAT_R32_UINT;
        ibv.SizeInBytes = iByteSize;
        return ibv;
    }
};

void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc);
