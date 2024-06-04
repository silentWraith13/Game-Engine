#pragma once

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/RaytracingHelpers.hpp"
#include "Engine/Renderer/Denoiser.hpp"

struct CompositionData
{
	IntVec2 textureDim;
	Vec2    invTextureDim;
	bool	denoiserOn;
	float	renderOutput;
};

enum CompositionRootSignatures 
{
	FinalOutput = 0,
	GI = 1,
	DI = 2,
	Albedo = 3,
	Normals,
	Position,
	MotionVectorOutput,
	Depth,
	TemporalGBuffer,
	DenoiserOutput,
	MomentsBuffer,
	HistoryBuffer,
	PartialDerivatesBuffer,
	ConstantBuffer,
	CompositorOuput,
	VarianceEstimation,
	IndirectAlbedo,
	CountCompositor
};

class Composition
{
	public:
		Composition() {};
		~Composition() { 
		m_rootSignature.Reset();
		m_pipelineStateObject.Reset();
		m_compositionCB.ResetResources();
		}
		void Initialize(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
		void Run(ID3D12GraphicsCommandList4* commandList, ID3D12DescriptorHeap* descriptorHeap, bool denoiserOn);

	public:
		Microsoft::WRL::ComPtr<ID3D12RootSignature>         m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>         m_pipelineStateObject;
		ConstantBufferD12<CompositionData>					m_compositionCB;
		UINT												m_instanceID = 0;
		IntVec2												m_dispatchDim = IntVec2(8, 8);
		int													m_renderOutput = 0;

};
