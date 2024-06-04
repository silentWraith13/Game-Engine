//#pragma once
//#include "Engine/Renderer/RaytracingHelpers.hpp"
//#include <vector>
//
//struct ASGeometry
//{
//	GpuBuffer m_vb;
//	GpuBuffer m_ib;
//};
//
//class BLASGeometry
//{
//	public:
//		std::wstring             m_name;
//		ASGeometry				m_geometry;
//		UINT                     m_numTriangles = 0;
//		DXGI_FORMAT              m_indexFormat = DXGI_FORMAT_UNKNOWN;
//		UINT                     m_ibStride = 0;
//		DXGI_FORMAT              m_vertexFormat = DXGI_FORMAT_UNKNOWN;
//		UINT                     m_vbStride = 0;
//};
//
//class AccelerationStructure
//{
//	public:
//		AccelerationStructure() {};
//		~AccelerationStructure() {};
//		ID3D12Resource* GetResource();
//		void ReleaseResource ();
//		long RequiredScratchSize();
//		long RequiredResultDataSizeInBytes();
//		void SetDirty(bool dirty);
//		void SetBuilt(bool built);
//		bool GetIsDirty();
//	public:
//		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS	  m_buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
//		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO m_prebuildInfo = {};
//		Microsoft::WRL::ComPtr<ID3D12Resource>			      m_accelerationStructure;
//
//		bool m_isDirty = false;
//		bool m_isBuilt = false;
//		bool m_updateOnBuild = false;
//		bool m_allowUpdate = false;
//		void AllocateResource(ID3D12Device5* device);
//		std::wstring m_name;
//};
//
//class TopLevelAccelerationStructure : public AccelerationStructure
//{
//	public:
//		TopLevelAccelerationStructure() {};
//		~TopLevelAccelerationStructure() {};
//		
//	public:
//		void Initialize(ID3D12Device5* device, int numBottomLevleASInstances, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, bool allowUpdate = false, bool bUpdateOnBuild = false, const wchar_t* resourceName = nullptr);
//		void Build(ID3D12GraphicsCommandList4* commandList, int numInstances,D3D12_GPU_VIRTUAL_ADDRESS instanceDescription, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, bool Update = false);
//};
//
//class BottomLevelAccelerationStructure : public AccelerationStructure
//{
//	public:
//		BottomLevelAccelerationStructure() {};
//	public:
//		void Initialize(ID3D12Device5* device, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, BLASGeometry& bottomLevelASGeometry,const std::wstring& name, bool allowUpdate = false, bool bUpdateOnBuild = false);
//		void Build(ID3D12GraphicsCommandList4* commandList, ID3D12Resource* scratch, ID3D12DescriptorHeap* descriptorHeap, D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress = 0);
//
//	public:
//		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> m_geometryDescriptions;
//		int											m_currentId = 0;
//
//		void BuildGeometryDescriptions(BLASGeometry& geometry);
//		void ComputePrebuildInfo(ID3D12Device5* device);
//};
//
//class RaytracingASManager
//{
//	public:
//		RaytracingASManager(ID3D12Device5* device, int maxBLAS, int frameCount);
//		void AddBottomLevelAS(ID3D12Device5* device, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, BLASGeometry& bottomLevelASGeometry, bool allowUpdate, bool performUpdateOnBuild);
//		~RaytracingASManager();
//	public:
//		std::vector<BottomLevelAccelerationStructure>	 m_BLStructures;
//		StructuredBuffer<D3D12_RAYTRACING_INSTANCE_DESC> m_bottomLevelASInstanceDescs;
//		UINT											 m_numBottomLevelASInstances = 0;
//		Microsoft::WRL::ComPtr<ID3D12Resource>			 m_accelerationStructureScratch;
//		UINT64											 m_scratchResourceSize = 0;
//};
