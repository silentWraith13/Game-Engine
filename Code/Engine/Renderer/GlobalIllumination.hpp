#pragma once
#include <string>

#include "Engine/Renderer/RendererD12.hpp"


struct GIRayPayload
{
	Vec4 InidrectIllumination;
	Vec4 IndirectVertexAlbedo;
	float ReflectionIndex;
	float Raytype;
	float tHit;
	bool  DidHitEmissiveSurface;
};

enum class GIGlobalRSParams {
	//UAV
	GBufferGlobalIllumination,
	GBufferVertexPositionSlot,
	GBufferVertexNormalSlot,
	GBufferDirectIllumination,
	GBufferIndirectAlbedo,

	//SRVS
	AccelerationStructure,
	VertexAndIndexBuffers,
	DiffuseTexture,
	NormalMapTexture,
	SpecularMapTexture,
	LightBuffer,
	GBufferAlbedoSlot,

	//CBVS
	SceneConstantBuffer,

	Count
};

class GlobalIllumination
{
	public:
		~GlobalIllumination();
		void Initialize(ID3D12Device5* device, UINT frameCount);
		void Run(ID3D12GraphicsCommandList* commandList, int frameIndex);
		void CreatePipelineStateObject(ID3D12Device5* device, UINT frameCount);
		void BuildShaderTables(ID3D12Device5* device);
		void CreateRootSignatures();
	public:
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_GIGlobalRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_GILocalRootSignature;	
		Microsoft::WRL::ComPtr<ID3D12StateObject>	m_GIDxrStateObject;
		void*										rayGenShaderID = nullptr;
		void*										missShaderID = nullptr;
		void*										hitGroupShaderID = nullptr;
		ShaderTable									HitShaderTable;
		ShaderTable									RaygenShaderTable;
		ShaderTable								    MissShaderTable;
		static const wchar_t*						c_hitGroupName;
		static const wchar_t*						c_raygenShaderName;
		static const wchar_t*						c_closestHitShaderName;
		static const wchar_t*						c_missShaderName;
		UINT									    m_hitGroupShaderTableStrideInBytes = UINT_MAX;
		UINT									    m_raygenGroupShaderTableStrideInBytes = UINT_MAX;

};

