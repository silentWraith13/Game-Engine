#include "Engine/Renderer/PostProcess.hpp"

extern RendererD12* g_theRendererDx12;

void PostProcess::InitializeGodRays(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
{
	//----------------CREATE TEXTURE DIMENSION BUFFER---------
	m_postProcessCB.Create(device, frameCount * numCallsPerFrame, L"Constant Buffer: Texture dimensions");

	//--------------COMPILE SHADER FOR GAUSSIAN FILTER--------------
	ComPtr<IDxcBlob> shaderBlob;
	CD3DX12_SHADER_BYTECODE byteCode;
	shaderBlob = g_theRendererDx12->m_shaderCompiler->CompileComputeShader("Data/Shaders/PostProcessCS.hlsl");
	byteCode.BytecodeLength = shaderBlob->GetBufferSize();
	byteCode.pShaderBytecode = shaderBlob->GetBufferPointer();

	CD3DX12_DESCRIPTOR_RANGE ranges[6];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
	ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
	ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[7] = {};
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0]);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1]);
	rootParameters[2].InitAsDescriptorTable(1, &ranges[2]);
	rootParameters[3].InitAsDescriptorTable(1, &ranges[3]);
	rootParameters[4].InitAsDescriptorTable(1, &ranges[4]);
	rootParameters[5].InitAsDescriptorTable(1, &ranges[5]);
	rootParameters[6].InitAsConstantBufferView(0);

	//CD3DX12_STATIC_SAMPLER_DESC staticSampler(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR);
	CD3DX12_STATIC_SAMPLER_DESC staticSampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, 1, &staticSampler);
	g_theRendererDx12->SerializeAndCreateRaytracingRootSignature(rootSignatureDesc, &m_rootSignature);

	D3D12_COMPUTE_PIPELINE_STATE_DESC csDesc = {};
	csDesc.pRootSignature = m_rootSignature.Get();
	csDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	csDesc.CS = byteCode;
	HRESULT hr = device->CreateComputePipelineState(&csDesc, IID_PPV_ARGS(&m_pipelineStateObject));
	g_theRendererDx12->ThrowIfFailed(hr, "Failed while creating Pipeline State object for post process");
	m_pipelineStateObject->SetName(L"Post Process PSO");

}
void PostProcess::RunGodRays(ID3D12GraphicsCommandList4* commandList,
	ID3D12DescriptorHeap* descriptorHeap)
{
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	auto outputResource = resourceManager->GetRaytracingOutputBufferResource();
	auto resourceDesc = outputResource->GetResource()->GetDesc();
	//IntVec2 resourceDimensions = IntVec2(static_cast<int>(resourceDesc.Width), static_cast<int>(resourceDesc.Height));
	Vec2 resourceDimensions = Vec2(static_cast<float>(resourceDesc.Width), static_cast<float>(resourceDesc.Height));

	m_postProcessCB->textureDim = resourceDimensions;
	m_postProcessCB->invTextureDim = Vec2(1.f / resourceDimensions.x, 1.f / resourceDimensions.y);
	m_postProcessCB->GIColor = g_theRendererDx12->m_gameValues.GIColor;
	m_postProcessCB->lightPosition = g_theRendererDx12->m_lightPosition;
	//m_postProcessCB->cameraPosition = g_theRenderer->m_currentCamera.m_position;
	m_postProcessCB->viewMatrix = g_theRendererDx12->m_currentCamera.GetViewMatrix();
	m_postProcessCB->projectionMatrix = g_theRendererDx12->m_currentCamera.GetProjectionMatrix();
	//m_postProcessCB->inversedViewMatrix = m_postProcessCB->viewMatrix.GetNonOrthonormalInverse();
	//m_postProcessCB->inversedProjectionMatrix = m_postProcessCB->projectionMatrix.GetNonOrthonormalInverse();
	m_instanceID = (m_instanceID + 1) % m_postProcessCB.NumInstances();
	m_postProcessCB.CopyStagingToGpu(m_instanceID);

	{
		resourceManager->TransitionResource(&resourceManager->m_GpuresourceBuffers[(int)GBufferResources::CompositorOutput], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	//Set all the resources for the Pipeline state object of Gaussian filter
	commandList->SetDescriptorHeaps(1, &descriptorHeap);
	commandList->SetComputeRootSignature(m_rootSignature.Get());
	commandList->SetComputeRootDescriptorTable(0, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::CompositorOutput].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(1, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::Depth].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(2, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::OcclusionTexture].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(3, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexNormal].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(4, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::DirectLight].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(5, outputResource->gpuWriteDescriptorHandle);
	commandList->SetComputeRootConstantBufferView(6, m_postProcessCB.GpuVirtualAddress(m_instanceID));
	commandList->SetPipelineState(m_pipelineStateObject.Get());

	int dispatchx = (int)(resourceDimensions.x + m_dispatchDim.x - 1) / m_dispatchDim.x;
	int dispatchy = (int)(resourceDimensions.y + m_dispatchDim.y - 1) / m_dispatchDim.y;
	commandList->Dispatch(dispatchx, dispatchy, 1);
}
