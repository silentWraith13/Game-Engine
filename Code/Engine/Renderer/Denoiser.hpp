#pragma once

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/RaytracingHelpers.hpp"

struct DenoiserData
{
	Vec2	textureDim;
	Vec2    invTextureDim;
	float	kernelSize = 0.0f;
	int		atrousStepSize;
	float	temporalFade;
};

enum DenoiserRootSignatures 
{
	Output = 0,
	GIInput = 1,
	DIInput = 2,
	AlbedoInput = 3,
	NormalInput = 4,
	_GIInput = 5,
	_NormalInput = 6,
	ConstantBuffers = 7,
	MotionVector = 8,
	DepthInput = 9, 
	_DepthInput = 10,
	VertexPositionInput = 11,
	TemporalOutput = 12,
	_TemporalOutput = 13,
	HistoryLength = 14,
	_HistoryLength = 15,
	Moments = 16,
	_Moments = 17,
	PartialDerivates = 18,
	Count
};


enum AtrousRootSignatures
{
	AtrousOutput = 0,
	AtrousDenoiserInput = 1,
	AtrousDIInput = 2,
	AtrousAlbedoInput = 3,
	AtrousNormalInput = 4,
	AtrousConstantBuffers = 5,
	AtrousDepthInput = 6,
	AtrousVertexPositionInput = 7,
	AtrousVarianceInput = 8,
	AtrousPartialDepth = 9,
	AtrousCount
};

enum class FilterSize
{
	Filter3X3,
	Filter7X7,
	Filter9X9
};

enum class DenoiserType
{
	GaussianFilter,
	BilateralFilter,
	AtrousBilateral
};
class DenoiserStages
{
	public:
		DenoiserStages() {};
		DenoiserStages(int type);
		~DenoiserStages();
	public:
		Microsoft::WRL::ComPtr<ID3D12RootSignature>         m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature>         m_partialDerivativesRootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>         m_pipelineStateObject;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>         m_partialDerivatesPipelineStateObject;
		int													m_filterSize;
		ConstantBufferD12<DenoiserData>						m_denoiserCB;

		UINT												m_instanceID = 0;
		IntVec2												m_dispatchDim = IntVec2(8, 8);
};

class TemporalSampler : public DenoiserStages
{
	public:
		TemporalSampler() {} ;
		~TemporalSampler() { m_temporalOutput.resource.Reset();
		m_partialDerivativesRootSignature.Reset();
		m_partialDerivatesPipelineStateObject.Reset();
		} ;
		void Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
		void Run(ID3D12GraphicsCommandList4* commandList, UINT filterStep, ID3D12DescriptorHeap* descriptorHeap);
	public:
		GpuBuffer											m_temporalOutput;
		GpuBuffer											m_previousTemporalOutput;
};

class VarianceEstimator : public DenoiserStages
{
public:
	VarianceEstimator() {};
	~VarianceEstimator() { 
		m_varianceResource.ResetResource();
		m_momentResource.ResetResource();
		m_rootSignature.Reset();
		m_pipelineStateObject.Reset();
		m_partialDerivativesRootSignature.Reset();
		m_partialDerivatesPipelineStateObject.Reset();
	};
	void Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
	void Run(ID3D12GraphicsCommandList4* commandList, UINT filterStep, GpuBuffer* inputBuffer);

	void InitializeParitalDerivatives(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
	void RunPartialDerivatives(ID3D12GraphicsCommandList4* commandList, UINT filterStep);
public:
	GpuBuffer											m_varianceResource;
	GpuBuffer											m_momentResource;
};

class GaussianFilter : public DenoiserStages
{
	public:	
		GaussianFilter() {};
		~GaussianFilter() {
			m_rootSignature.Reset();
			m_pipelineStateObject.Reset();
			m_partialDerivativesRootSignature.Reset();
			m_partialDerivatesPipelineStateObject.Reset();
		};
		void Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
		void Run(ID3D12GraphicsCommandList4* commandList,UINT filterStep,ID3D12DescriptorHeap* descriptorHeap);
};

class AtrouWaveletFilter : public DenoiserStages
{
	public:
		AtrouWaveletFilter() {};
		~AtrouWaveletFilter() {
			m_rootSignature.Reset();
			m_pipelineStateObject.Reset();
			m_partialDerivativesRootSignature.Reset();
			m_partialDerivatesPipelineStateObject.Reset();
		} ;
		void Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
		void Run(ID3D12GraphicsCommandList4* commandList, UINT filterStep, ID3D12DescriptorHeap* descriptorHeap);
	public:
		int														m_atrousStepSize = 1;

};

class Denoiser
{
	public:
		Denoiser();
		void Run(ID3D12GraphicsCommandList4* commandList, UINT filterStep, ID3D12DescriptorHeap* descriptorHeap, bool temporalSamplerOn, bool denoiserOn);
		void SetFilterSize(int size);
		void CopyDenoiserOutputToInputForNextStage();
		~Denoiser();

	public:
		void InitializeDenoiser(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
	public:
		GaussianFilter*						m_gaussianFilter = nullptr;	
		AtrouWaveletFilter*					m_atrousWaveletFilter = nullptr;
		TemporalSampler*					m_temporalSampler = nullptr;
		VarianceEstimator*					m_varianceEstimator = nullptr;
		DenoiserType						m_denoiserType;
		bool								m_temporalSamplerOn = true;
		int									m_filterSize;
		int									m_totalAtrousSteps = 0;
		bool								m_varianceFilteringOn = true;
};