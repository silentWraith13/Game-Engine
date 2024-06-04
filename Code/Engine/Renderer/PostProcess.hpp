#pragma once
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Renderer/ResourceManager.hpp"
#include "Engine/Renderer/RaytracingHelpers.hpp"
#include <dxcapi.h>
#include <Game/GameCommon.hpp>
#include "Engine/Math/Mat44.hpp"

struct PostProcessCB
{
	Vec2 textureDim;
	Vec2    invTextureDim;
	Vec4	lightPosition;
	Vec4	GIColor;
	//Vec4	cameraPosition;
	Mat44	viewMatrix;
	Mat44	projectionMatrix;
	//Mat44	inversedViewMatrix;
	//Mat44	inversedProjectionMatrix;
};
class PostProcess
{
	public:
		~PostProcess() {
			m_postProcessCB.ResetResources();
			m_rootSignature.Reset();
			m_pipelineStateObject.Reset();
		}
		void InitializeGodRays(ID3D12Device5* device, UINT frameCount, UINT numCallsPerFrame = 1);
		void RunGodRays(ID3D12GraphicsCommandList4* commandList,ID3D12DescriptorHeap* descriptorHeap);
	public:
		ConstantBufferD12<PostProcessCB>						m_postProcessCB;
		Microsoft::WRL::ComPtr<ID3D12RootSignature>				m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>				m_pipelineStateObject;
		UINT													m_instanceID = 0;
		IntVec2													m_dispatchDim = IntVec2(8, 8);
		int														m_filterSize = 0;
};