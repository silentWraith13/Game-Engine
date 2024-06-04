//#include "AccelerationStructureHelper.hpp"
//#include "Engine/Renderer/RendererD12.hpp"
//
//extern RendererD12* g_theRenderer;
//ID3D12Resource* AccelerationStructure::GetResource()
//{
//	return m_accelerationStructure.Get();
//}
//
//void AccelerationStructure::ReleaseResource()
//{
//	m_accelerationStructure.Reset();
//}
//
//long AccelerationStructure::RequiredScratchSize()
//{
//	return max(m_prebuildInfo.ScratchDataSizeInBytes, m_prebuildInfo.UpdateScratchDataSizeInBytes);
//}
//
//long AccelerationStructure::RequiredResultDataSizeInBytes()
//{
//	return m_prebuildInfo.ResultDataMaxSizeInBytes;
//}
//
//void AccelerationStructure::SetDirty(bool dirty)
//{
//	m_isDirty = dirty;
//}
//
//void AccelerationStructure::SetBuilt(bool built)
//{
//	m_isBuilt = built;
//}
//
//bool AccelerationStructure::GetIsDirty()
//{
//	return m_isDirty;
//}
//
//void AccelerationStructure::AllocateResource(ID3D12Device5* device)
//{
//	D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
//	g_theRenderer->AllocateUAVBuffer(device, m_prebuildInfo.ResultDataMaxSizeInBytes, &m_accelerationStructure, state, m_name.c_str());
//}
//
//void TopLevelAccelerationStructure::Initialize(ID3D12Device5* device, int numBottomLevleASInstances, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, bool allowUpdate, bool updateOnBuild, const wchar_t* resourceName)
//{
//	m_allowUpdate = allowUpdate;
//	m_updateOnBuild = updateOnBuild;
//	m_buildFlags = buildFlags;
//	m_name = resourceName;
//
//	if (m_allowUpdate)
//	{
//		m_buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
//	}
//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelDesc = {};
//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &inputs = topLevelDesc.Inputs;
//	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
//	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
//	inputs.Flags = m_buildFlags;
//	inputs.NumDescs = numBottomLevleASInstances;
//
//	device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &m_prebuildInfo);
//	AllocateResource(device);
//
//	m_isDirty = true;
//	m_isBuilt = false;
//}
//
//void TopLevelAccelerationStructure::Build(ID3D12GraphicsCommandList4* commandList, int numInstances, D3D12_GPU_VIRTUAL_ADDRESS instanceDescription, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool Update)
//{
//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &inputs = topLevelBuildDesc.Inputs;
//
//	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
//	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
//	inputs.Flags = m_buildFlags;
//	if (m_isBuilt && m_allowUpdate && m_updateOnBuild)
//	{
//		inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
//	}
//	inputs.NumDescs = numInstances;
//	commandList->SetDescriptorHeaps(1, &descriptorHeap);
//	commandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc,0, nullptr);
//
//	m_isDirty = false;
//	m_isBuilt = true;
//}
//
//void BottomLevelAccelerationStructure::Initialize(ID3D12Device5* device, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, BLASGeometry& bottomLevelASGeometry,const std::wstring& name , bool allowUpdate, bool updateOnBuild)
//{
//	m_allowUpdate = allowUpdate;
//	m_updateOnBuild = updateOnBuild;
//	m_buildFlags = buildFlags;
//	m_name = name;
//
//	if (m_allowUpdate)
//	{
//		m_buildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
//	}
//
//	BuildGeometryDescriptions(bottomLevelASGeometry);
//	ComputePrebuildInfo(device);
//	AllocateResource(device);
//
//	m_isDirty = true;
//	m_isBuilt = false;
//}
//
//void BottomLevelAccelerationStructure::Build(ID3D12GraphicsCommandList4* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress)
//{
//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc;
//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs = desc.Inputs;
//
//	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
//	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
//	inputs.Flags = m_buildFlags;
//	if (m_isBuilt && m_allowUpdate && m_updateOnBuild)
//	{
//		inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
//		desc.SourceAccelerationStructureData  = m_accelerationStructure->GetGPUVirtualAddress();
//	}
//
//	inputs.NumDescs = static_cast<UINT>(m_geometryDescriptions.size());
//	inputs.pGeometryDescs = m_geometryDescriptions.data();
//
//	desc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
//	desc.DestAccelerationStructureData = m_accelerationStructure->GetGPUVirtualAddress();
//
//	commandList->SetDescriptorHeaps(1, &descriptorHeap);
//	commandList->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
//
//	m_isDirty = false;
//	m_isBuilt = true;
//}
//
//void BottomLevelAccelerationStructure::BuildGeometryDescriptions(BLASGeometry& geometry)
//{
//	D3D12_RAYTRACING_GEOMETRY_DESC desc = {};
//	desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
//	desc.Triangles.IndexFormat = geometry.m_indexFormat;
//	desc.Triangles.VertexFormat = geometry.m_vertexFormat;
//
//	GpuBuffer& indexBuffer= geometry.m_geometry.m_ib;
//	GpuBuffer& vertexBuffer= geometry.m_geometry.m_vb;
//
//	desc.Triangles.IndexBuffer = indexBuffer.GetResource()->GetGPUVirtualAddress();
//	desc.Triangles.IndexCount = static_cast<UINT>(indexBuffer.resource->GetDesc().Width) / sizeof(UINT);
//	desc.Triangles.VertexBuffer.StartAddress = vertexBuffer.GetResource()->GetGPUVirtualAddress();
//	desc.Triangles.VertexBuffer.StrideInBytes = geometry.m_vbStride;
//	desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
//	desc.Triangles.Transform3x4 = 0;
//
//	m_geometryDescriptions.push_back(desc);
//}
//
//void BottomLevelAccelerationStructure::ComputePrebuildInfo(ID3D12Device5* device)
//{
//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc;
//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs = desc.Inputs;
//
//	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
//	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
//	inputs.Flags = m_buildFlags;
//
//	inputs.NumDescs = static_cast<UINT>(m_geometryDescriptions.size());
//	inputs.pGeometryDescs = m_geometryDescriptions.data();
//
//	device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &m_prebuildInfo);
//	g_theRenderer->ThrowIfFalse(m_prebuildInfo.ResultDataMaxSizeInBytes > 0, "Prebuild For bottom level acceleration failed ");
//}
//
//RaytracingASManager::RaytracingASManager(ID3D12Device5* device, int maxBLAS, int frameCount)
//{
//	m_bottomLevelASInstanceDescs.Create(device, maxBLAS, frameCount, L"BLAS instance Descriptions");
//}
//
//
//void RaytracingASManager::AddBottomLevelAS(ID3D12Device5* device,D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, BLASGeometry& bottomLevelASGeometry,
//	bool allowUpdate,
//	bool performUpdateOnBuild)
//{
//	BottomLevelAccelerationStructure blas;
//	blas.Initialize(device, buildFlags, bottomLevelASGeometry,NULL, allowUpdate);
//
//	m_scratchResourceSize = max(blas.RequiredScratchSize(), m_scratchResourceSize);
//
//	m_BLStructures.push_back(blas);
//}