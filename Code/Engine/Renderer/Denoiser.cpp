#include "Denoiser.hpp"
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/ResourceManager.hpp"
#include "Engine/Renderer/RaytracingHelpers.hpp"
#include <dxcapi.h>
#include <Game/GameCommon.hpp>

extern RendererD12* g_theRendererDx12;

//-------------------------------------DENOISER---------------------------------
Denoiser::Denoiser()
{

}
Denoiser::~Denoiser()
{
	delete m_gaussianFilter;
	m_gaussianFilter = nullptr;
	delete m_atrousWaveletFilter;
	m_atrousWaveletFilter = nullptr;
	delete m_temporalSampler;
	m_temporalSampler = nullptr;
	delete m_varianceEstimator;
	m_varianceEstimator = nullptr;

}
void Denoiser::InitializeDenoiser(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
{
	m_temporalSampler = new TemporalSampler();
	m_temporalSampler->Initialize(device, frameCount, numCallsPerFrame);

	m_varianceEstimator = new VarianceEstimator();
	m_varianceEstimator->Initialize(device, frameCount, numCallsPerFrame);
	m_varianceEstimator->InitializeParitalDerivatives(device, frameCount, numCallsPerFrame);

	m_gaussianFilter = new GaussianFilter();
	m_gaussianFilter->Initialize(device, frameCount, numCallsPerFrame);

	m_atrousWaveletFilter = new AtrouWaveletFilter();
	m_atrousWaveletFilter->Initialize(device, frameCount, numCallsPerFrame);
}
void Denoiser::Run(ID3D12GraphicsCommandList4* commandList, UINT filterStep, ID3D12DescriptorHeap* descriptorHeap, bool temporalSamplerOn, bool denoiserOn)
{
	GpuBuffer* src = nullptr;
	GpuBuffer* dest = nullptr;
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	m_temporalSamplerOn = temporalSamplerOn;
	if (temporalSamplerOn)
	{
		m_temporalSampler->Run(commandList, filterStep, descriptorHeap);
		if (m_varianceFilteringOn)
		{
			GpuBuffer* temporalOutput = &g_theRendererDx12->m_denoiser->m_temporalSampler->m_temporalOutput;
			m_varianceEstimator->Run(commandList, filterStep, temporalOutput);
		}
		
	}
	if (denoiserOn)
	{
		if (temporalSamplerOn)
		{
			src = &g_theRendererDx12->m_denoiser->m_temporalSampler->m_temporalOutput;
		}
		else
		{
			src = &resourceManager->m_GpuresourceBuffers[(int)GBufferResources::GI];
		}
		dest = &resourceManager->m_GpuresourceBuffers[(int)GBufferResources::DenoiserInput];
		g_theRendererDx12->CopyTextureResourceFromBuffer(src, dest);

		if (m_denoiserType == DenoiserType::GaussianFilter)
		{
			m_gaussianFilter->m_filterSize = m_filterSize;
			m_gaussianFilter->Run(commandList, filterStep, descriptorHeap);
		}
		else if (m_denoiserType == DenoiserType::AtrousBilateral)
		{
			for (int i = 0; i < m_totalAtrousSteps; i++)
			/*for (int i = m_totalAtrousSteps - 1; i >= 0; i--)*/
			{
				GpuBuffer* denoiserOutput = resourceManager->GetDenoiserOutputResource();
				if (m_varianceFilteringOn && i != 0.0f)
				{
					m_varianceEstimator->Run(commandList, filterStep, denoiserOutput);
				}
				m_atrousWaveletFilter->m_filterSize = m_filterSize;
				m_atrousWaveletFilter->m_atrousStepSize = i;
				m_atrousWaveletFilter->Run(commandList, filterStep, descriptorHeap);
				
			
				CopyDenoiserOutputToInputForNextStage();
				if(i == 0)
				/*if (i == m_totalAtrousSteps - 1)*/
				{
					auto denoiserFilterOutput = resourceManager->GetDenoiserOutputResource();
					src = denoiserFilterOutput;
					dest = &m_temporalSampler->m_previousTemporalOutput;
					g_theRendererDx12->CopyTextureResourceFromBuffer(src, dest);
				}
			}
		}
	}
	

	//Copy Previous Frame Resources to GPUBuffers 
	src = &resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexNormal];
	dest = &resourceManager->m_previousFrameGpuResourceBuffers[(int)GBufferResourcesPreviousFrame::VertexNormal];
	g_theRendererDx12->CopyTextureResourceFromBuffer(src, dest);

	src = &resourceManager->m_GpuresourceBuffers[(int)GBufferResources::Depth];
	dest = &resourceManager->m_previousFrameGpuResourceBuffers[(int)GBufferResourcesPreviousFrame::Depth];
	g_theRendererDx12->CopyTextureResourceFromBuffer(src, dest);

	//auto denoiserOutput = resourceManager->GetDenoiserOutputResource();
	src = &resourceManager->m_GpuresourceBuffers[(int)GBufferResources::GI];
	dest = &resourceManager->m_previousFrameGpuResourceBuffers[(int)GBufferResourcesPreviousFrame::GI];
	g_theRendererDx12->CopyTextureResourceFromBuffer(src, dest);

	src = &resourceManager->m_GpuresourceBuffers[(int)GBufferResources::Moments];
	dest = &resourceManager->m_previousFrameGpuResourceBuffers[(int)GBufferResourcesPreviousFrame::Moments];
	g_theRendererDx12->CopyTextureResourceFromBuffer(src, dest);

	src = &resourceManager->m_GpuresourceBuffers[(int)GBufferResources::HistoryLength];
	dest = &resourceManager->m_previousFrameGpuResourceBuffers[(int)GBufferResourcesPreviousFrame::History];
	g_theRendererDx12->CopyTextureResourceFromBuffer(src, dest);
}
void Denoiser::CopyDenoiserOutputToInputForNextStage()
{
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	auto denoiserOutput = resourceManager->GetDenoiserOutputResource();
	GpuBuffer* dest = nullptr;
	dest = &resourceManager->m_GpuresourceBuffers[(int)GBufferResources::DenoiserInput];
	g_theRendererDx12->CopyTextureResourceFromBuffer(denoiserOutput, dest);
}
void Denoiser::SetFilterSize(int size)
{
	m_filterSize = size;
	m_gaussianFilter->m_filterSize = size;
	m_atrousWaveletFilter->m_filterSize = size;
	m_temporalSampler->m_filterSize = size;
	m_varianceEstimator->m_filterSize = size;
}
DenoiserStages::DenoiserStages(int type)
{
	m_filterSize = type;
}
DenoiserStages::~DenoiserStages()
{
	m_rootSignature.Reset();
	m_partialDerivativesRootSignature.Reset();
	m_partialDerivatesPipelineStateObject.Reset();
	m_pipelineStateObject.Reset();
}

//--------------------------------------TEMPORAL SAMPLING--------------------------------
void TemporalSampler::Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
{
	//----------------CREATE G BUFFER FOR TEMPORAL OUTPUT-------------------
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	IntVec2 gbufferDimensions = g_theRendererDx12->m_dimensions;
	//D3D12_RESOURCE_STATES startingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	resourceManager->CreateGBufferResource(DXGI_FORMAT_R8G8B8A8_UNORM, gbufferDimensions, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, (UINT)GBufferResources::OutputResource, L"Output Resource");
	resourceManager->CreateGBufferResource(&m_temporalOutput, DXGI_FORMAT_R8G8B8A8_UNORM, gbufferDimensions, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"Temporal Output", true);

	resourceManager->CreateGBufferResource(&m_previousTemporalOutput, DXGI_FORMAT_R8G8B8A8_UNORM, gbufferDimensions, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"Temporal Output");

	//----------------CREATE TEXTURE DIMENSION BUFFER---------
	m_denoiserCB.Create(device, frameCount * numCallsPerFrame, L"Constant Buffer: Texture dimensions");

	//--------------COMPILE SHADER FOR TEMPORAL SAMPLER--------------
	ComPtr<IDxcBlob> temporalSamplingShaderBlob;
	CD3DX12_SHADER_BYTECODE temporalSampleShaderByteCode;
	temporalSamplingShaderBlob = g_theRendererDx12->m_shaderCompiler->CompileComputeShader("Data/Shaders/TemporalSamplingCS.hlsl");
	temporalSampleShaderByteCode.BytecodeLength = temporalSamplingShaderBlob->GetBufferSize();
	temporalSampleShaderByteCode.pShaderBytecode = temporalSamplingShaderBlob->GetBufferPointer();

	CD3DX12_DESCRIPTOR_RANGE ranges[DenoiserRootSignatures::Count]; // TSPP AND MOTION VECTOR
	ranges[(int)DenoiserRootSignatures::DIInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	ranges[(int)DenoiserRootSignatures::AlbedoInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	ranges[(int)DenoiserRootSignatures::NormalInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	ranges[(int)DenoiserRootSignatures::_GIInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
	ranges[(int)DenoiserRootSignatures::_NormalInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
	ranges[(int)DenoiserRootSignatures::MotionVector].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5); // Motion Vector
	ranges[(int)DenoiserRootSignatures::DepthInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6); // Depth
	ranges[(int)DenoiserRootSignatures::_DepthInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7); // Previous Frame depth
	ranges[(int)DenoiserRootSignatures::_TemporalOutput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8); // Previous Frame temporal values
	ranges[(int)DenoiserRootSignatures::VertexPositionInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9);
	ranges[(int)DenoiserRootSignatures::_Moments].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10);
	ranges[(int)DenoiserRootSignatures::_HistoryLength].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 11); // 
	ranges[(int)DenoiserRootSignatures::PartialDerivates].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 12); // Partial Derivates
	ranges[(int)DenoiserRootSignatures::GIInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 13);  // 0 register is output 1 register is UAV GI so we can write to it.

	ranges[(int)DenoiserRootSignatures::Output].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // output
	ranges[TemporalOutput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1); // Temporal output
	ranges[(int)DenoiserRootSignatures::Moments].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
	ranges[(int)DenoiserRootSignatures::HistoryLength].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3); // Variance input


	CD3DX12_ROOT_PARAMETER rootParameters[DenoiserRootSignatures::Count] = {};// TSPP AND MOTION VECTOR
	rootParameters[(UINT)DenoiserRootSignatures::Output].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::Output]);
	rootParameters[(UINT)DenoiserRootSignatures::GIInput].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::GIInput]);
	rootParameters[(UINT)DenoiserRootSignatures::DIInput].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::DIInput]);
	rootParameters[(UINT)DenoiserRootSignatures::AlbedoInput].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::AlbedoInput]);
	rootParameters[(UINT)DenoiserRootSignatures::NormalInput].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::NormalInput]);
	rootParameters[(UINT)DenoiserRootSignatures::_GIInput].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::_GIInput]);
	rootParameters[(UINT)DenoiserRootSignatures::_NormalInput].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::_NormalInput]);
	rootParameters[(int)DenoiserRootSignatures::MotionVector].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::MotionVector]); 
	rootParameters[(int)DenoiserRootSignatures::DepthInput].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::DepthInput]);
	rootParameters[(int)DenoiserRootSignatures::_DepthInput].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::_DepthInput]); 
	rootParameters[(int)DenoiserRootSignatures::_TemporalOutput].InitAsDescriptorTable(1, &ranges[_TemporalOutput]);
	rootParameters[(int)DenoiserRootSignatures::HistoryLength].InitAsDescriptorTable(1, &ranges[HistoryLength]);
	rootParameters[(int)DenoiserRootSignatures::_HistoryLength].InitAsDescriptorTable(1, &ranges[_HistoryLength]);
	rootParameters[(int)DenoiserRootSignatures::VertexPositionInput].InitAsDescriptorTable(1, &ranges[VertexPositionInput]);
	rootParameters[(int)DenoiserRootSignatures::Moments].InitAsDescriptorTable(1, &ranges[Moments]);
	rootParameters[(int)DenoiserRootSignatures::_Moments].InitAsDescriptorTable(1, &ranges[_Moments]);
	rootParameters[(int)DenoiserRootSignatures::PartialDerivates].InitAsDescriptorTable(1, &ranges[PartialDerivates]);

	rootParameters[(int)DenoiserRootSignatures::ConstantBuffers].InitAsConstantBufferView(0); // The Denoiser Constant buffer
	rootParameters[(int)DenoiserRootSignatures::TemporalOutput].InitAsDescriptorTable(1, &ranges[(int)DenoiserRootSignatures::TemporalOutput]); // Temporal output

	//CD3DX12_STATIC_SAMPLER_DESC staticSampler(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR);
	CD3DX12_STATIC_SAMPLER_DESC staticSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT);
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, 1, &staticSampler);
	g_theRendererDx12->SerializeAndCreateRaytracingRootSignature(rootSignatureDesc, &m_rootSignature);

	D3D12_COMPUTE_PIPELINE_STATE_DESC csDesc = {};
	csDesc.pRootSignature = m_rootSignature.Get();
	csDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	csDesc.CS = temporalSampleShaderByteCode;
	HRESULT hr = device->CreateComputePipelineState(&csDesc, IID_PPV_ARGS(&m_pipelineStateObject));
	g_theRendererDx12->ThrowIfFailed(hr, "Failed while creating Pipeline State object for denoiser");
	m_pipelineStateObject->SetName(L"Denoiser PSO");
}
void TemporalSampler::Run(ID3D12GraphicsCommandList4* commandList, UINT filterStep, ID3D12DescriptorHeap* descriptorHeap)
{

	UNUSED((void)filterStep);
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	auto GetGBuffers = (resourceManager->m_GpuresourceBuffers);
	auto resourceDesc = m_temporalOutput.GetResource()->GetDesc();
	Vec2 resourceDimensions = Vec2((float)g_theRendererDx12->m_dimensions.x, (float)g_theRendererDx12->m_dimensions.y);

	m_denoiserCB->textureDim = resourceDimensions;
	m_denoiserCB->invTextureDim = Vec2(1.f / resourceDimensions.x, 1.f / resourceDimensions.y);
	m_denoiserCB->kernelSize = (float)m_filterSize;
	m_denoiserCB->temporalFade = g_theRendererDx12->m_temporalFade;
	m_instanceID = (m_instanceID + 1) % m_denoiserCB.NumInstances();
	m_denoiserCB.CopyStagingToGpu(m_instanceID);


	{
		resourceManager->TransitionResource(&m_temporalOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::Moments], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::HistoryLength], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::PartialDerivates], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	//Set all the resources for the Pipeline state object of Gaussian filter
	commandList->SetDescriptorHeaps(1, &descriptorHeap);
	commandList->SetComputeRootSignature(m_rootSignature.Get());
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::GIInput, GetGBuffers[(int)GBufferResources::GI].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::_GIInput, resourceManager->m_previousFrameGpuResourceBuffers[(int)GBufferResourcesPreviousFrame::GI].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::DIInput, GetGBuffers[(int)GBufferResources::DirectLight].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::AlbedoInput, GetGBuffers[(int)GBufferResources::VertexAlbedo].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::_NormalInput, resourceManager->m_previousFrameGpuResourceBuffers[(int)GBufferResourcesPreviousFrame::VertexNormal].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::MotionVector, GetGBuffers[(int)GBufferResources::MotionVector].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::DepthInput, GetGBuffers[(int)GBufferResources::Depth].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::_DepthInput, resourceManager->m_previousFrameGpuResourceBuffers[(int)GBufferResourcesPreviousFrame::Depth].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::HistoryLength, GetGBuffers[(int)GBufferResources::HistoryLength].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::_HistoryLength, resourceManager->m_previousFrameGpuResourceBuffers[(int)GBufferResourcesPreviousFrame::History].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::TemporalOutput, m_temporalOutput.gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::_TemporalOutput, m_previousTemporalOutput.gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::VertexPositionInput, GetGBuffers[(int)GBufferResources::VertexPosition].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::Moments, GetGBuffers[(int)GBufferResources::Moments].gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::_Moments, resourceManager->m_previousFrameGpuResourceBuffers[(int)GBufferResourcesPreviousFrame::Moments].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)DenoiserRootSignatures::PartialDerivates, GetGBuffers[(int)GBufferResources::PartialDerivates].gpuReadDescriptorHandle);
	commandList->SetComputeRootConstantBufferView((int)DenoiserRootSignatures::ConstantBuffers, m_denoiserCB.GpuVirtualAddress(m_instanceID));
	commandList->SetPipelineState(m_pipelineStateObject.Get());

	int dispatchx = (int)(resourceDimensions.x + m_dispatchDim.x - 1) / m_dispatchDim.x;
	int dispatchy = (int)(resourceDimensions.y + m_dispatchDim.y - 1) / m_dispatchDim.y;
	commandList->Dispatch(dispatchx, dispatchy, 1);

	{
		resourceManager->TransitionResource(&m_temporalOutput, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::Moments], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		resourceManager->TransitionResource(&GetGBuffers[(UINT)GBufferResources::HistoryLength], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}


}

void VarianceEstimator::Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
{
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	IntVec2 gbufferDimensions = g_theRendererDx12->m_dimensions;
	resourceManager->CreateGBufferResource(&m_varianceResource, DXGI_FORMAT_R16_FLOAT, gbufferDimensions, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"Variance Output", true);
	resourceManager->CreateGBufferResource(&m_momentResource, DXGI_FORMAT_R16G16_FLOAT, gbufferDimensions, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"Variance Output");
	
	//----------------CREATE TEXTURE DIMENSION BUFFER---------
	m_denoiserCB.Create(device, frameCount * numCallsPerFrame, L"Constant Buffer: Texture dimensions");

	//--------------COMPILE SHADER FOR TEMPORAL SAMPLER--------------
	ComPtr<IDxcBlob> varianceBlob;
	CD3DX12_SHADER_BYTECODE varianceShaderByteCode;
	varianceBlob = g_theRendererDx12->m_shaderCompiler->CompileComputeShader("Data/Shaders/VarianceEstimationCS.hlsl");
	varianceShaderByteCode.BytecodeLength = varianceBlob->GetBufferSize();
	varianceShaderByteCode.pShaderBytecode = varianceBlob->GetBufferPointer();


	CD3DX12_DESCRIPTOR_RANGE ranges[7]; 
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // Output Variance

	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // GI Input
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // Motion vector
	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2); // History length
	ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3); // Depth
	ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4); // Partial Derivative
	ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5); // DI input


	CD3DX12_ROOT_PARAMETER rootParameters[8] = {};
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0]);// Output Variance
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1]);//  GI Input
	rootParameters[2].InitAsDescriptorTable(1, &ranges[2]);// Motion vector
	rootParameters[3].InitAsDescriptorTable(1, &ranges[3]);// History
	rootParameters[4].InitAsDescriptorTable(1, &ranges[4]);// Depth
	rootParameters[5].InitAsDescriptorTable(1, &ranges[5]);// Partial Derivative
	rootParameters[6].InitAsDescriptorTable(1, &ranges[6]);// DI input

	rootParameters[7].InitAsConstantBufferView(0); // The Denoiser Constant buffer

	CD3DX12_STATIC_SAMPLER_DESC staticSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT);
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, 1, &staticSampler);
	g_theRendererDx12->SerializeAndCreateRaytracingRootSignature(rootSignatureDesc, &m_rootSignature);

	D3D12_COMPUTE_PIPELINE_STATE_DESC csDesc = {};
	csDesc.pRootSignature = m_rootSignature.Get();
	csDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	csDesc.CS = varianceShaderByteCode;
	HRESULT hr = device->CreateComputePipelineState(&csDesc, IID_PPV_ARGS(&m_pipelineStateObject));
	g_theRendererDx12->ThrowIfFailed(hr, "Failed while creating Pipeline State object for Variance estimator");
	m_pipelineStateObject->SetName(L"Variance PSO");

}
void VarianceEstimator::Run(ID3D12GraphicsCommandList4* commandList, UINT filterStep, GpuBuffer* inputBuffer)
{
	UNUSED((void)filterStep);
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	auto GetGBuffers = (resourceManager->m_GpuresourceBuffers);
	//auto temporalOutput = g_theRenderer->m_denoiser->m_temporalSampler->m_temporalOutput;
	/*auto resourceDesc = m_varianceResource.GetResource()->GetDesc();*/
	Vec2 resourceDimensions = Vec2((float)g_theRendererDx12->m_dimensions.x, (float)g_theRendererDx12->m_dimensions.y);

	m_denoiserCB->textureDim = resourceDimensions;
	m_denoiserCB->invTextureDim = Vec2(1.f / resourceDimensions.x, 1.f / resourceDimensions.y);
	m_denoiserCB->kernelSize = (float)m_filterSize;
	m_denoiserCB->temporalFade = g_theRendererDx12->m_temporalFade;
	m_instanceID = (m_instanceID + 1) % m_denoiserCB.NumInstances();
	m_denoiserCB.CopyStagingToGpu(m_instanceID);

	{
		resourceManager->TransitionResource(&m_varianceResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		resourceManager->TransitionResource(inputBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	
	//Set all the resources for the Pipeline state object of Gaussian filter
	commandList->SetDescriptorHeaps(1, g_theRendererDx12->m_descriptorHeap.GetAddressOf());
	commandList->SetComputeRootSignature(m_rootSignature.Get());
	commandList->SetComputeRootDescriptorTable(0, m_varianceResource.gpuWriteDescriptorHandle);
	//commandList->SetComputeRootDescriptorTable(2, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::GI].gpuDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(1, inputBuffer->gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(2, GetGBuffers[(int)GBufferResources::MotionVector].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(3, GetGBuffers[(int)GBufferResources::HistoryLength].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(4, GetGBuffers[(int)GBufferResources::Depth].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(5, GetGBuffers[(int)GBufferResources::PartialDerivates].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(6, GetGBuffers[(int)GBufferResources::DirectLight].gpuReadDescriptorHandle);
	commandList->SetComputeRootConstantBufferView(7, m_denoiserCB.GpuVirtualAddress(m_instanceID));
	commandList->SetPipelineState(m_pipelineStateObject.Get());

	int dispatchx = (int)(resourceDimensions.x + m_dispatchDim.x - 1) / m_dispatchDim.x;
	int dispatchy = (int)(resourceDimensions.y + m_dispatchDim.y - 1) / m_dispatchDim.y;
	commandList->Dispatch(dispatchx, dispatchy, 1);

}
void VarianceEstimator::InitializeParitalDerivatives(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
{
	UNUSED((void) numCallsPerFrame);
	UNUSED((void)frameCount);
	//auto resourceManager = g_theRenderer->GetResourceManager();
	IntVec2 gbufferDimensions = g_theRendererDx12->GetRenderConfig().m_window->GetClientDimensions();
	
	//--------------COMPILE SHADER FOR TEMPORAL SAMPLER--------------
	ComPtr<IDxcBlob> partialDerivativesBlob;
	CD3DX12_SHADER_BYTECODE partialDerivativesShaderByteCode;
	partialDerivativesBlob = g_theRendererDx12->m_shaderCompiler->CompileComputeShader("Data/Shaders/PartialDerivativesCS.hlsl");
	partialDerivativesShaderByteCode.BytecodeLength = partialDerivativesBlob->GetBufferSize();
	partialDerivativesShaderByteCode.pShaderBytecode = partialDerivativesBlob->GetBufferPointer();


	CD3DX12_DESCRIPTOR_RANGE ranges[3];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); 
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); 

	CD3DX12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0]);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1]);

	rootParameters[2].InitAsConstantBufferView(0); // The Denoiser Constant buffer

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
	g_theRendererDx12->SerializeAndCreateRaytracingRootSignature(rootSignatureDesc, &m_partialDerivativesRootSignature);

	D3D12_COMPUTE_PIPELINE_STATE_DESC csDesc = {};
	csDesc.pRootSignature = m_partialDerivativesRootSignature.Get();
	csDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	csDesc.CS = partialDerivativesShaderByteCode;
	HRESULT hr = device->CreateComputePipelineState(&csDesc, IID_PPV_ARGS(&m_partialDerivatesPipelineStateObject));
	g_theRendererDx12->ThrowIfFailed(hr, "Failed while creating Pipeline State object for Variance estimator");
	m_partialDerivatesPipelineStateObject->SetName(L"Partial Derivatives PSO");
}
void VarianceEstimator::RunPartialDerivatives(ID3D12GraphicsCommandList4* commandList, UINT filterStep)
{
	UNUSED((void)filterStep);
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	Vec2 resourceDimensions = Vec2((float)g_theRendererDx12->m_dimensions.x, (float)g_theRendererDx12->m_dimensions.y);
	m_denoiserCB->textureDim = resourceDimensions;
	m_denoiserCB->invTextureDim = Vec2(1.f / g_theRendererDx12->m_dimensions.x, 1.f / g_theRendererDx12->m_dimensions.y);
	m_denoiserCB->kernelSize = (float)m_filterSize;
	m_denoiserCB->temporalFade = g_theRendererDx12->m_temporalFade;
	m_instanceID = (m_instanceID + 1) % m_denoiserCB.NumInstances();
	m_denoiserCB.CopyStagingToGpu(m_instanceID);

	{
		resourceManager->TransitionResource(&resourceManager->m_GpuresourceBuffers[(int)GBufferResources::PartialDerivates], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	//Set all the resources for the Pipeline state object of Gaussian filter
	commandList->SetDescriptorHeaps(1, g_theRendererDx12->m_descriptorHeap.GetAddressOf());
	commandList->SetComputeRootSignature(m_partialDerivativesRootSignature.Get());
	commandList->SetComputeRootDescriptorTable(0, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::Depth].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(1, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::PartialDerivates].gpuWriteDescriptorHandle);
	commandList->SetComputeRootConstantBufferView(2, m_denoiserCB.GpuVirtualAddress(m_instanceID));
	commandList->SetPipelineState(m_partialDerivatesPipelineStateObject.Get());

	int dispatchx = (int)(resourceDimensions.x + m_dispatchDim.x - 1) / m_dispatchDim.x;
	int dispatchy = (int)(resourceDimensions.y + m_dispatchDim.y - 1) / m_dispatchDim.y;
	commandList->Dispatch(dispatchx, dispatchy, 1);
	{
		resourceManager->TransitionResource(&resourceManager->m_GpuresourceBuffers[(int)GBufferResources::PartialDerivates], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

}


//------------------------------------------GAUSSIAN FILTER--------------------------
void GaussianFilter::Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
{
	//----------------CREATE TEXTURE DIMENSION BUFFER---------
	m_denoiserCB.Create(device, frameCount * numCallsPerFrame, L"Constant Buffer: Texture dimensions");

	//--------------COMPILE SHADER FOR GAUSSIAN FILTER--------------
	ComPtr<IDxcBlob> GaussianFilterShaderBlob;
	CD3DX12_SHADER_BYTECODE gaussianFilter;
	GaussianFilterShaderBlob = g_theRendererDx12->m_shaderCompiler->CompileComputeShader("Data/Shaders/GaussianFilterCS.hlsl");
	gaussianFilter.BytecodeLength = GaussianFilterShaderBlob->GetBufferSize();
	gaussianFilter.pShaderBytecode = GaussianFilterShaderBlob->GetBufferPointer();

	CD3DX12_DESCRIPTOR_RANGE ranges[6];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);


	CD3DX12_ROOT_PARAMETER rootParameters[6] = {};
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0]);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1]);
	rootParameters[2].InitAsDescriptorTable(1, &ranges[2]);
	rootParameters[3].InitAsDescriptorTable(1, &ranges[3]);
	rootParameters[4].InitAsDescriptorTable(1, &ranges[4]);
	rootParameters[5].InitAsConstantBufferView(0);

	//CD3DX12_STATIC_SAMPLER_DESC staticSampler(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR);
	CD3DX12_STATIC_SAMPLER_DESC staticSampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP) ;
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, 1, &staticSampler);
	g_theRendererDx12->SerializeAndCreateRaytracingRootSignature(rootSignatureDesc, &m_rootSignature);

	D3D12_COMPUTE_PIPELINE_STATE_DESC csDesc = {};
	csDesc.pRootSignature = m_rootSignature.Get();
	csDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE; 
	csDesc.CS = gaussianFilter;
	HRESULT hr = device->CreateComputePipelineState(&csDesc, IID_PPV_ARGS(&m_pipelineStateObject));
	g_theRendererDx12->ThrowIfFailed(hr, "Failed while creating Pipeline State object for denoiser");
	m_pipelineStateObject->SetName(L"Denoiser PSO");

}
void GaussianFilter::Run(ID3D12GraphicsCommandList4* commandList,UINT filterStep, 
ID3D12DescriptorHeap* descriptorHeap)
{
	UNUSED((void)filterStep);
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	auto outputResource = resourceManager->GetDenoiserOutputResource();
	GpuBuffer GIInput;
	if (g_theRendererDx12->m_denoiser->m_temporalSamplerOn)
	{
		GIInput = g_theRendererDx12->m_denoiser->m_temporalSampler->m_temporalOutput;
	}
	else
	{
		GIInput = resourceManager->m_GpuresourceBuffers[(int)GBufferResources::GI];
	}
	Vec2 resourceDimensions = Vec2((float)g_theRendererDx12->m_dimensions.x, (float)g_theRendererDx12->m_dimensions.y);
	m_denoiserCB->textureDim = resourceDimensions;
	m_denoiserCB->invTextureDim = Vec2(1.f / resourceDimensions.x, 1.f / resourceDimensions.y);
	m_denoiserCB->kernelSize = (float)m_filterSize;
	m_denoiserCB->temporalFade = g_theRendererDx12->m_temporalFade;
	m_instanceID = (m_instanceID + 1) % m_denoiserCB.NumInstances();
	m_denoiserCB.CopyStagingToGpu(m_instanceID);

	//Set all the resources for the Pipeline state object of Gaussian filter
	commandList->SetDescriptorHeaps(1, &descriptorHeap);
	commandList->SetComputeRootSignature(m_rootSignature.Get());
	commandList->SetComputeRootDescriptorTable(0, outputResource->gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(1, GIInput.gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(2, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::DirectLight].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(3, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexAlbedo].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(4, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexNormal].gpuReadDescriptorHandle);
	commandList->SetComputeRootConstantBufferView(5, m_denoiserCB.GpuVirtualAddress(m_instanceID));
	commandList->SetPipelineState(m_pipelineStateObject.Get());

	int dispatchx = (int)(resourceDimensions.x + m_dispatchDim.x - 1 )/ m_dispatchDim.x;
	int dispatchy = (int)(resourceDimensions.y + m_dispatchDim.y - 1) / m_dispatchDim.y;
	commandList->Dispatch(dispatchx , dispatchy,1);
}


//----------------------------------ATROUS FILTER------------------------------------
void AtrouWaveletFilter::Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame)
{
	//----------------CREATE TEXTURE DIMENSION BUFFER---------
	m_denoiserCB.Create(device, frameCount * numCallsPerFrame, L"Constant Buffer: Texture dimensions");

	//--------------COMPILE SHADER FOR GAUSSIAN FILTER--------------
	ComPtr<IDxcBlob> GaussianFilterShaderBlob;
	CD3DX12_SHADER_BYTECODE gaussianFilter;
	GaussianFilterShaderBlob = g_theRendererDx12->m_shaderCompiler->CompileComputeShader("Data/Shaders/AtrousWaveletFilterCS.hlsl");
	gaussianFilter.BytecodeLength = GaussianFilterShaderBlob->GetBufferSize();
	gaussianFilter.pShaderBytecode = GaussianFilterShaderBlob->GetBufferPointer();

	CD3DX12_DESCRIPTOR_RANGE ranges[AtrousRootSignatures::AtrousCount];
	ranges[(int)AtrousRootSignatures::AtrousOutput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	ranges[(int)AtrousRootSignatures::AtrousDIInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	ranges[(int)AtrousRootSignatures::AtrousAlbedoInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	ranges[(int)AtrousRootSignatures::AtrousNormalInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	ranges[(int)AtrousRootSignatures::AtrousVertexPositionInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
	ranges[(int)AtrousRootSignatures::AtrousDepthInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
	ranges[(int)AtrousRootSignatures::AtrousVarianceInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);
	ranges[(int)AtrousRootSignatures::AtrousPartialDepth].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);
	ranges[(int)AtrousRootSignatures::AtrousDenoiserInput].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);

	CD3DX12_ROOT_PARAMETER rootParameters[AtrousRootSignatures::AtrousCount] = {};
	rootParameters[(UINT)AtrousRootSignatures::AtrousOutput].InitAsDescriptorTable(1, &ranges[(int)AtrousRootSignatures::AtrousOutput]);
	rootParameters[(UINT)AtrousRootSignatures::AtrousDenoiserInput].InitAsDescriptorTable(1, &ranges[(int)AtrousRootSignatures::AtrousDenoiserInput]);
	rootParameters[(UINT)AtrousRootSignatures::AtrousDIInput].InitAsDescriptorTable(1, &ranges[(int)AtrousRootSignatures::AtrousDIInput]);
	rootParameters[(UINT)AtrousRootSignatures::AtrousAlbedoInput].InitAsDescriptorTable(1, &ranges[(int)AtrousRootSignatures::AtrousAlbedoInput]);
	rootParameters[(UINT)AtrousRootSignatures::AtrousNormalInput].InitAsDescriptorTable(1, &ranges[(int)AtrousRootSignatures::AtrousNormalInput]);
	rootParameters[(UINT)AtrousRootSignatures::AtrousVertexPositionInput].InitAsDescriptorTable(1, &ranges[(int)AtrousRootSignatures::AtrousVertexPositionInput]);
	rootParameters[(UINT)AtrousRootSignatures::AtrousDepthInput].InitAsDescriptorTable(1, &ranges[(int)AtrousRootSignatures::AtrousDepthInput]);
	rootParameters[(UINT)AtrousRootSignatures::AtrousVarianceInput].InitAsDescriptorTable(1, &ranges[(int)AtrousRootSignatures::AtrousVarianceInput]);
	rootParameters[(UINT)AtrousRootSignatures::AtrousPartialDepth].InitAsDescriptorTable(1, &ranges[(int)AtrousRootSignatures::AtrousPartialDepth]);
	rootParameters[(UINT)AtrousRootSignatures::AtrousConstantBuffers].InitAsConstantBufferView(0);

	//CD3DX12_STATIC_SAMPLER_DESC staticSampler(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR);
	CD3DX12_STATIC_SAMPLER_DESC staticSampler = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters, 1, &staticSampler);
	g_theRendererDx12->SerializeAndCreateRaytracingRootSignature(rootSignatureDesc, &m_rootSignature);

	D3D12_COMPUTE_PIPELINE_STATE_DESC csDesc = {};
	csDesc.pRootSignature = m_rootSignature.Get();
	csDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	csDesc.CS = gaussianFilter;
	HRESULT hr = device->CreateComputePipelineState(&csDesc, IID_PPV_ARGS(&m_pipelineStateObject));
	g_theRendererDx12->ThrowIfFailed(hr, "Failed while creating Pipeline State object for denoiser");
	m_pipelineStateObject->SetName(L"Denoiser PSO");

}
void AtrouWaveletFilter::Run(ID3D12GraphicsCommandList4* commandList, UINT filterStep, ID3D12DescriptorHeap* descriptorHeap)
{
	UNUSED((void)filterStep);
	auto resourceManager = g_theRendererDx12->GetResourceManager();
	auto outputResource = resourceManager->GetDenoiserOutputResource();
	auto varianceOutput = g_theRendererDx12->m_denoiser->m_varianceEstimator->m_varianceResource;
	//auto GetGBuffers = (resourceManager->m_GpuresourceBuffers);

	GpuBuffer& denoiserInput  = resourceManager->m_GpuresourceBuffers[(int)GBufferResources::DenoiserInput]; 

	auto resourceDesc = outputResource->GetResource()->GetDesc();
	Vec2 resourceDimensions = Vec2((float)g_theRendererDx12->m_dimensions.x, (float)g_theRendererDx12->m_dimensions.y);


	m_denoiserCB->textureDim = resourceDimensions;
	m_denoiserCB->invTextureDim = Vec2(1.f / resourceDimensions.x, 1.f / resourceDimensions.y);
	m_denoiserCB->kernelSize = (float)m_filterSize;
	m_denoiserCB->temporalFade = g_theRendererDx12->m_temporalFade;
	m_denoiserCB->atrousStepSize = m_atrousStepSize;
	m_instanceID = (m_instanceID + 1) % m_denoiserCB.NumInstances();
	m_denoiserCB.CopyStagingToGpu(m_instanceID);

	{
		resourceManager->TransitionResource(&resourceManager->m_GpuresourceBuffers[(int)GBufferResources::DenoiserInput], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	//Set all the resources for the Pipeline state object of Gaussian filter
	commandList->SetDescriptorHeaps(1, &descriptorHeap);
	commandList->SetComputeRootSignature(m_rootSignature.Get());
	commandList->SetComputeRootDescriptorTable((int)AtrousRootSignatures::AtrousOutput, outputResource->gpuWriteDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)AtrousRootSignatures::AtrousDenoiserInput, denoiserInput.gpuReadDescriptorHandle);
	//commandList->SetComputeRootDescriptorTable((int)AtrousRootSignatures::AtrousDenoiserInput, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::GI].gpuDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)AtrousRootSignatures::AtrousDIInput, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::DirectLight].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)AtrousRootSignatures::AtrousAlbedoInput, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexAlbedo].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)AtrousRootSignatures::AtrousNormalInput, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexNormal].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)AtrousRootSignatures::AtrousVertexPositionInput, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::VertexPosition].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)AtrousRootSignatures::AtrousDepthInput, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::Depth].gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)AtrousRootSignatures::AtrousVarianceInput, varianceOutput.gpuReadDescriptorHandle);
	commandList->SetComputeRootDescriptorTable((int)AtrousRootSignatures::AtrousPartialDepth, resourceManager->m_GpuresourceBuffers[(int)GBufferResources::PartialDerivates].gpuReadDescriptorHandle);
	commandList->SetComputeRootConstantBufferView((int)AtrousRootSignatures::AtrousConstantBuffers, m_denoiserCB.GpuVirtualAddress(m_instanceID));
	commandList->SetPipelineState(m_pipelineStateObject.Get());

	int dispatchx = (int)(resourceDimensions.x + m_dispatchDim.x - 1) / m_dispatchDim.x;
	int dispatchy = (int)(resourceDimensions.y + m_dispatchDim.y - 1) / m_dispatchDim.y;
	commandList->Dispatch(dispatchx, dispatchy, 1);
}
