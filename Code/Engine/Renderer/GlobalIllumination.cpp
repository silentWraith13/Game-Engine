#include "Engine/Renderer/GlobalIllumination.hpp"
#include "Engine/Renderer/ResourceManager.hpp"
#include <dxcapi.h>

extern RendererD12* g_theRendererDx12;

const wchar_t* GlobalIllumination::c_hitGroupName = L"GIHitGroup";
const wchar_t* GlobalIllumination::c_raygenShaderName = L"GIRayGenShader";
const wchar_t* GlobalIllumination::c_closestHitShaderName = L"GIClosestHitShader";
const wchar_t* GlobalIllumination::c_missShaderName = L"GIMissShader";

GlobalIllumination::~GlobalIllumination()
{
	m_GIGlobalRootSignature.Reset();
	m_GILocalRootSignature.Reset();
	m_GIDxrStateObject.Reset();
}

void GlobalIllumination::Initialize(ID3D12Device5* device, UINT frameCount)
{
	CreateRootSignatures();
	CreatePipelineStateObject(device, frameCount);
	BuildShaderTables(device);
}

void GlobalIllumination::Run(ID3D12GraphicsCommandList* commandList, int frameIndex)
{
	ResourceManager* resourceManager = g_theRendererDx12->m_resourceManager;
	commandList->SetComputeRootSignature(m_GIGlobalRootSignature.Get());
	// Copy the updated scene constant buffer to GPU.
	auto GetGBuffers = g_theRendererDx12->m_resourceManager->m_GpuresourceBuffers;
	//m_irradianceCacheGPUBuffer.CopyFromCPUToGPU(m_frameIndex);
	commandList->SetComputeRootConstantBufferView((UINT)GIGlobalRSParams::SceneConstantBuffer, g_theRendererDx12->m_sceneCB.GpuVirtualAddress(frameIndex));
	//commandList->SetComputeRootConstantBufferView((UINT)GIGlobalRSParams::LightBuffer, g_theRenderer->m_lightCB.GpuVirtualAddress(frameIndex));
	commandList->SetComputeRootConstantBufferView((UINT)GIGlobalRSParams::LightBuffer, g_theRendererDx12->m_lightCB.GpuVirtualAddress(frameIndex));
	//Bind the heaps, acceleration structure and dispatch rays.
	commandList->SetDescriptorHeaps(1, g_theRendererDx12->m_descriptorHeap.GetAddressOf());
	commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::GBufferVertexPositionSlot, GetGBuffers[(UINT)GBufferResources::VertexPosition].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::GBufferVertexNormalSlot, GetGBuffers[(UINT)GBufferResources::VertexNormal].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::GBufferGlobalIllumination, GetGBuffers[(UINT)GBufferResources::GI].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::GBufferDirectIllumination, GetGBuffers[(UINT)GBufferResources::DirectLight].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::GBufferIndirectAlbedo, GetGBuffers[(UINT)GBufferResources::VertexIndirectAlbedo].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::GBufferAlbedoSlot, GetGBuffers[(UINT)GBufferResources::VertexAlbedo].gpuReadDescriptorHandle);

	if (g_theRendererDx12->m_currentScene == Scenes::Minecraft)
	{
		//commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::IndexBuffersSlot, m_indexBuffer.gpuDescriptorHandle);
		commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::VertexAndIndexBuffers, g_theRendererDx12->m_indexBuffer.gpuReadDescriptorHandle);
		commandList->SetComputeRootShaderResourceView((UINT)GIGlobalRSParams::AccelerationStructure, g_theRendererDx12->m_topLevelAccelerationStructure->GetGPUVirtualAddress());
	}
	else if (g_theRendererDx12->m_currentScene == Scenes::Bunny)
	{
		//commandList->SetComputeRootDescriptorTable((UINT)GlobalRootSignatureParams::IndexBuffersSlot, m_bunnyIndexBuffer.gpuDescriptorHandle);
		commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::VertexAndIndexBuffers, g_theRendererDx12->m_bunnyIndexBuffer.gpuReadDescriptorHandle);
		commandList->SetComputeRootShaderResourceView((UINT)GIGlobalRSParams::AccelerationStructure, g_theRendererDx12->m_bunnyTlas->GetGPUVirtualAddress());
	}

	commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::DiffuseTexture, g_theRendererDx12->m_loadedTextures[0]->m_gpuDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::NormalMapTexture, g_theRendererDx12->m_loadedTextures[1]->m_gpuDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((UINT)GIGlobalRSParams::SpecularMapTexture, g_theRendererDx12->m_loadedTextures[2]->m_gpuDescriptorHandle);
	//commandList->SetComputeRootShaderResourceView((UINT)GIGlobalRSParams::HemisphereSamples, g_theRenderer->m_hemisphereSamplesGPUBuffer.GpuVirtualAddress(frameIndex));
	
	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	dispatchDesc.HitGroupTable.StartAddress = HitShaderTable.GetResource()->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = HitShaderTable.GetResource()->GetDesc().Width;
	dispatchDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
	dispatchDesc.MissShaderTable.StartAddress = MissShaderTable.GetResource()->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = MissShaderTable.GetResource()->GetDesc().Width;
	dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
	dispatchDesc.RayGenerationShaderRecord.StartAddress = RaygenShaderTable.GetResource()->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = RaygenShaderTable.GetResource()->GetDesc().Width;
	dispatchDesc.Width = g_theRendererDx12->m_dimensions.x;
	dispatchDesc.Height = g_theRendererDx12->m_dimensions.y;
	dispatchDesc.Depth = 1;
	g_theRendererDx12->m_dxrCommandList.Get()->SetPipelineState1(m_GIDxrStateObject.Get());
	g_theRendererDx12->m_dxrCommandList.Get()->DispatchRays(&dispatchDesc);

	resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::GI], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::VertexIndirectAlbedo], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void GlobalIllumination::CreateRootSignatures()
{
	CD3DX12_DESCRIPTOR_RANGE ranges[(int)GIGlobalRSParams::Count]; // Perfomance TIP: Order from most frequent to least frequent.

	//---------------------------UAVS---------------------------------------
	ranges[(int)GIGlobalRSParams::GBufferGlobalIllumination].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture
	ranges[(int)GIGlobalRSParams::GBufferIndirectAlbedo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);  // 1 texture buffer.

	//---------------------------SRVS------------------------------------------------
	ranges[(int)GIGlobalRSParams::VertexAndIndexBuffers].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);  // 1 texture buffer.
	ranges[(int)GIGlobalRSParams::DiffuseTexture].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);  // 1 texture buffer.
	ranges[(int)GIGlobalRSParams::NormalMapTexture].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);  // 1 texture buffer.
	ranges[(int)GIGlobalRSParams::SpecularMapTexture].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);  // 1 texture buffer.
	ranges[(int)GIGlobalRSParams::GBufferVertexPositionSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);  // 1 output texture
	ranges[(int)GIGlobalRSParams::GBufferVertexNormalSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8);  // 1 texture buffer.
	ranges[(int)GIGlobalRSParams::GBufferDirectIllumination].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1,9);  // 1 texture buffer.
	ranges[(int)GIGlobalRSParams::GBufferAlbedoSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10);  // 1 texture buffer.

	CD3DX12_ROOT_PARAMETER rootParameters[(UINT)GIGlobalRSParams::Count] = {};

	rootParameters[(int)GIGlobalRSParams::GBufferGlobalIllumination].InitAsDescriptorTable(1, &ranges[(int)GIGlobalRSParams::GBufferGlobalIllumination]);
	rootParameters[(int)GIGlobalRSParams::GBufferVertexPositionSlot].InitAsDescriptorTable(1, &ranges[(int)GIGlobalRSParams::GBufferVertexPositionSlot]);
	rootParameters[(int)GIGlobalRSParams::GBufferVertexNormalSlot].InitAsDescriptorTable(1, &ranges[(int)GIGlobalRSParams::GBufferVertexNormalSlot]);
	rootParameters[(int)GIGlobalRSParams::GBufferDirectIllumination].InitAsDescriptorTable(1, &ranges[(int)GIGlobalRSParams::GBufferDirectIllumination]);
	rootParameters[(int)GIGlobalRSParams::GBufferIndirectAlbedo].InitAsDescriptorTable(1, &ranges[(int)GIGlobalRSParams::GBufferIndirectAlbedo]);

	rootParameters[(int)GIGlobalRSParams::AccelerationStructure].InitAsShaderResourceView(0);
	rootParameters[(UINT)GIGlobalRSParams::VertexAndIndexBuffers].InitAsDescriptorTable(1, &ranges[(UINT)GIGlobalRSParams::VertexAndIndexBuffers]);
	rootParameters[(UINT)GIGlobalRSParams::DiffuseTexture].InitAsDescriptorTable(1, &ranges[(UINT)GIGlobalRSParams::DiffuseTexture]);
	rootParameters[(UINT)GIGlobalRSParams::NormalMapTexture].InitAsDescriptorTable(1, &ranges[(UINT)GIGlobalRSParams::NormalMapTexture]);
	rootParameters[(UINT)GIGlobalRSParams::SpecularMapTexture].InitAsDescriptorTable(1, &ranges[(UINT)GIGlobalRSParams::SpecularMapTexture]);
	rootParameters[(UINT)GIGlobalRSParams::GBufferAlbedoSlot].InitAsDescriptorTable(1, &ranges[(UINT)GIGlobalRSParams::GBufferAlbedoSlot]);
	rootParameters[(int)GIGlobalRSParams::SceneConstantBuffer].InitAsConstantBufferView(0);
	rootParameters[(int)GIGlobalRSParams::LightBuffer].InitAsConstantBufferView(1);

	CD3DX12_STATIC_SAMPLER_DESC staticSamplers[] =
	{
		// LinearWrapSampler
		CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT),
		//CD3DX12_STATIC_SAMPLER_DESC(1, D3D12_FILTER_ANISOTROPIC),
	};

	CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, ARRAYSIZE(staticSamplers), staticSamplers);
	g_theRendererDx12->SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_GIGlobalRootSignature);
}

void GlobalIllumination::CreatePipelineStateObject(ID3D12Device5* device, UINT frameCount)
{
	UNUSED((void) frameCount);
	UNUSED((void)device);
	ComPtr<IDxcBlob> mainShaderBlob;
	CD3DX12_SHADER_BYTECODE GI;
	mainShaderBlob = g_theRendererDx12->m_shaderCompiler->Compile("Data/Shaders/GlobalIllumination.hlsl");
	GI.BytecodeLength = mainShaderBlob->GetBufferSize();
	GI.pShaderBytecode = mainShaderBlob->GetBufferPointer();

	//CREATING PSO
	CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

	auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	lib->SetDXILLibrary(&GI);
	lib->DefineExport(c_raygenShaderName);
	lib->DefineExport(c_closestHitShaderName);
	lib->DefineExport(c_missShaderName);

	auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
	hitGroup->SetClosestHitShaderImport(c_closestHitShaderName);
	hitGroup->SetHitGroupExport(c_hitGroupName);
	hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

	auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = sizeof(GIRayPayload);
	UINT attributeSize = 2 * sizeof(Vec2);
	shaderConfig->Config(payloadSize, attributeSize);

	// Local root signature and shader association
	/*auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	localRootSignature->SetRootSignature(m_raytracingLocalRootSignature.Get());

	auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
	rootSignatureAssociation->AddExport(c_hitGroupName);*/
	//rootSignatureAssociation->AddExport(c_closestHitShaderName);

	auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_GIGlobalRootSignature.Get());

	auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	UINT maxRecursionDepth = 30;
	pipelineConfig->Config(maxRecursionDepth);
	PrintStateObjectDesc(raytracingPipeline);
	g_theRendererDx12->ThrowIfFailed(g_theRendererDx12->m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_GIDxrStateObject)), "Couldn't create DirectX Raytracing state object.\n");
}

void GlobalIllumination::BuildShaderTables(ID3D12Device5* device)
{
	ComPtr<ID3D12StateObjectProperties> stateObjectProps;
	g_theRendererDx12->ThrowIfFailed(m_GIDxrStateObject.As(&stateObjectProps), "Failed to create properties for shader table");
	rayGenShaderID = stateObjectProps->GetShaderIdentifier(c_raygenShaderName);
	missShaderID = stateObjectProps->GetShaderIdentifier(c_missShaderName);
	hitGroupShaderID = stateObjectProps->GetShaderIdentifier(c_hitGroupName);

	UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	UINT numShaderRecords = 1;
	UINT shaderRecordSize = shaderIdentifierSize;

	// Ray gen shader table
	{
		shaderRecordSize = shaderIdentifierSize;
		RaygenShaderTable = ShaderTable(device, numShaderRecords, shaderRecordSize, L"GI Ray Gen Shader Table Resource");
		RaygenShaderTable.push_back(ShaderRecord(rayGenShaderID, shaderIdentifierSize, nullptr, 0));
		m_raygenGroupShaderTableStrideInBytes = RaygenShaderTable.GetShaderRecordSize();

	}

	// Miss shader table
	{
		MissShaderTable = ShaderTable(device, numShaderRecords, shaderRecordSize, L"GI Miss shader table resource");
		MissShaderTable.push_back(ShaderRecord(missShaderID, shaderIdentifierSize, nullptr, 0));
	}

	// Hit group shader table
	{
		LocalRootArgumentsGeometry rs = {};
		//numShaderRecords = MINECRAFTCHUNKS;
		//shaderRecordSize = shaderIdentifierSize + sizeof(LocalRootArgumentsGeometry);
		HitShaderTable = ShaderTable(device, numShaderRecords, shaderRecordSize, L"Hit Group shader table");
		HitShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIdentifierSize));
		//for (int i = 0; i < MINECRAFTCHUNKS; i++)
		//{
		//	HitShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIdentifierSize, &rs, sizeof(rs)));
		//}

		m_hitGroupShaderTableStrideInBytes = HitShaderTable.GetShaderRecordSize();
	}
}
