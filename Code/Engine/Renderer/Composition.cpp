#include "Composition.hpp"
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/ResourceManager.hpp"
#include "Engine/Renderer/RaytracingHelpers.hpp"
#include <dxcapi.h>

extern RendererD12* g_theRendererDx12;

//----------------------------------COMPOSITION------------------------------------
void Composition::Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
{
	//----------------CREATE TEXTURE DIMENSION BUFFER---------
	m_compositionCB.Create(device, frameCount * numCallsPerFrame, L"Constant Buffer: Composition");

	//--------------COMPILE SHADER FOR COMPOSITION--------------
	ComPtr<IDxcBlob> GaussianFilterShaderBlob;
	CD3DX12_SHADER_BYTECODE compositorByteCode;
	GaussianFilterShaderBlob = g_theRendererDx12->m_shaderCompiler->CompileComputeShader("Data/Shaders/CompositionCS.hlsl");
	compositorByteCode.BytecodeLength = GaussianFilterShaderBlob->GetBufferSize();
	compositorByteCode.pShaderBytecode = GaussianFilterShaderBlob->GetBufferPointer();

	CD3DX12_DESCRIPTOR_RANGE ranges[CompositionRootSignatures::CountCompositor];
	ranges[(int)CompositionRootSignatures::FinalOutput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	ranges[(int)CompositionRootSignatures::CompositorOuput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV,1, 1);

	ranges[(int)CompositionRootSignatures::DI].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	ranges[(int)CompositionRootSignatures::GI].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	ranges[(int)CompositionRootSignatures::Albedo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	ranges[(int)CompositionRootSignatures::Normals].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
	ranges[(int)CompositionRootSignatures::Position].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
	ranges[(int)CompositionRootSignatures::MotionVectorOutput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);
	ranges[(int)CompositionRootSignatures::Depth].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);
	ranges[(int)CompositionRootSignatures::TemporalGBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);
	ranges[(int)CompositionRootSignatures::DenoiserOutput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8);
	ranges[(int)CompositionRootSignatures::MomentsBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9);
	ranges[(int)CompositionRootSignatures::HistoryBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10);
	ranges[(int)CompositionRootSignatures::PartialDerivatesBuffer].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 11);
	ranges[(int)CompositionRootSignatures::VarianceEstimation].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 12);
	ranges[(int)CompositionRootSignatures::IndirectAlbedo].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 13);

	CD3DX12_ROOT_PARAMETER rootParameters[CompositionRootSignatures::CountCompositor] = {};
	rootParameters[(UINT)CompositionRootSignatures::FinalOutput].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::FinalOutput]);
	rootParameters[(UINT)CompositionRootSignatures::GI].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::GI]);
	rootParameters[(UINT)CompositionRootSignatures::DI].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::DI]);
	rootParameters[(UINT)CompositionRootSignatures::Albedo].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::Albedo]);
	rootParameters[(UINT)CompositionRootSignatures::Normals].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::Normals]);
	rootParameters[(UINT)CompositionRootSignatures::Position].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::Position]);
	rootParameters[(UINT)CompositionRootSignatures::MotionVectorOutput].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::MotionVectorOutput]);
	rootParameters[(UINT)CompositionRootSignatures::Depth].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::Depth]);
	rootParameters[(UINT)CompositionRootSignatures::TemporalGBuffer].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::TemporalGBuffer]);
	rootParameters[(UINT)CompositionRootSignatures::DenoiserOutput].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::DenoiserOutput]);
	rootParameters[(UINT)CompositionRootSignatures::MomentsBuffer].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::MomentsBuffer]);
	rootParameters[(UINT)CompositionRootSignatures::HistoryBuffer].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::HistoryBuffer]);
	rootParameters[(UINT)CompositionRootSignatures::CompositorOuput].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::CompositorOuput]);
	rootParameters[(UINT)CompositionRootSignatures::PartialDerivatesBuffer].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::PartialDerivatesBuffer]);
	rootParameters[(UINT)CompositionRootSignatures::VarianceEstimation].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::VarianceEstimation]);
	rootParameters[(UINT)CompositionRootSignatures::IndirectAlbedo].InitAsDescriptorTable(1, &ranges[(int)CompositionRootSignatures::IndirectAlbedo]);
	rootParameters[(UINT)CompositionRootSignatures::ConstantBuffer].InitAsConstantBufferView(0);

	//CD3DX12_STATIC_SAMPLER_DESC staticSampler(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR);
	CD3DX12_STATIC_SAMPLER_DESC staticSampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, 1, &staticSampler);
	g_theRendererDx12->SerializeAndCreateRaytracingRootSignature(rootSignatureDesc, &m_rootSignature);

	D3D12_COMPUTE_PIPELINE_STATE_DESC csDesc = {};
	csDesc.pRootSignature = m_rootSignature.Get();
	csDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	csDesc.CS = compositorByteCode;
	HRESULT hr = device->CreateComputePipelineState(&csDesc, IID_PPV_ARGS(&m_pipelineStateObject));
	g_theRendererDx12->ThrowIfFailed(hr, "Failed while creating Pipeline State object for Compositor");
	m_pipelineStateObject->SetName(L"Composition PSO");

}
void Composition::Run(ID3D12GraphicsCommandList4* commandList,ID3D12DescriptorHeap* descriptorHeap, bool denoiserOn)
{
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	auto compositorOutput = resourceManager->m_GpuresourceBuffers[(int)GBufferResources::CompositorOutput];
	auto resourceDesc = compositorOutput.GetResource()->GetDesc();
	IntVec2 resourceDimensions = IntVec2(static_cast<int>(resourceDesc.Width), static_cast<int>(resourceDesc.Height));
	auto outputResource = resourceManager->GetRaytracingOutputBufferResource();

	m_compositionCB->textureDim = resourceDimensions;
	m_compositionCB->invTextureDim = Vec2(1.f / resourceDimensions.x, 1.f / resourceDimensions.y);
	m_compositionCB->denoiserOn = denoiserOn;
	m_compositionCB->renderOutput = (float)(m_renderOutput);
	m_instanceID = (m_instanceID + 1) % m_compositionCB.NumInstances();
	m_compositionCB.CopyStagingToGpu(m_instanceID);

	//Set all the resources for the Pipeline state object of Gaussian filter
	commandList->SetDescriptorHeaps(1, &descriptorHeap);
	commandList->SetComputeRootSignature(m_rootSignature.Get());
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::CompositorOuput, compositorOutput.gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::FinalOutput, outputResource->gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::GI, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::GI].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::DI, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::DirectLight].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::Albedo, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexAlbedo].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::Normals, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexNormal].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::Position, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexPosition].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::MotionVectorOutput, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::MotionVector].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::Depth, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::Depth].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::TemporalGBuffer, g_theRendererDx12->m_denoiser->m_temporalSampler->m_temporalOutput.gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::DenoiserOutput, resourceManager->GetDenoiserOutputResource()->gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::MomentsBuffer, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::Moments].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::HistoryBuffer, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::HistoryLength].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::PartialDerivatesBuffer, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::PartialDerivates].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::VarianceEstimation, g_theRendererDx12->m_denoiser->m_varianceEstimator->m_varianceResource.gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)CompositionRootSignatures::IndirectAlbedo, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexIndirectAlbedo].gpuReadDescriptorHandle);
	commandList->SetComputeRootConstantBufferView((int)CompositionRootSignatures::ConstantBuffer, m_compositionCB.GpuVirtualAddress(m_instanceID));
	commandList->SetPipelineState(m_pipelineStateObject.Get());

	int dispatchx = (resourceDimensions.x + m_dispatchDim.x - 1) / m_dispatchDim.x;
	int dispatchy = (resourceDimensions.y + m_dispatchDim.y - 1) / m_dispatchDim.y;
	commandList->Dispatch(dispatchx, dispatchy, 1);
}
