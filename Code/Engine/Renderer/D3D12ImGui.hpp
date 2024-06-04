#pragma once
#include "Engine/Renderer/RendererD12.hpp"
#include "Engine/Core/Rgba8.hpp"


struct ImColor;
struct ImVec4;
struct ImGuiContext;
struct RaytracingThesisVariables
{
	Vec4		m_samplingData; //x - Reflections s , y - Samples, z - Color Bleeding
	Vec4		m_color;
	Vec4        m_lightBools; // x - shadows , y - GI , z - Sky Light , w - normal maps
	float		m_renderOutput;
	bool		m_temporalSamplerOn;
	bool		m_godRaysOn;
	bool		m_denoiserOn;
	int			m_denoiserKernelSize = 0; // 0 --> 3X3 1 --> 7X7;
	int			m_denoiserType = 0 ; // 0 --> Gaussian 1 --> Atrous
	float		m_sunRadius;
	int 	    m_currentScene;
	int			m_atrousStepSize;
	bool		m_varianceFiltering;
	float		m_temporalFade;
	float		m_lightFallof;
	float		m_ambientIntensity;
	float		m_samplingMode = 1.0f;
	int			m_varianceEstimationLevel;
	Vec4		m_textureMappings;
};

class D3D12ImGui
{
	public:
		D3D12ImGui() {};
	public:
		void				InitializeImGui();
		void				RenderImGui();
		void				UpdateImGui();
		void				ShutdownImGui();
		ImColor*			m_color = nullptr;
	public:
		bool				ImGuiColorPicker(const char* label, ImColor* color);
	public:
		int					m_bufferCounts = 3;
		std::string			m_playerInfoText;
		std::string			m_gameInfoText;
		RaytracingThesisVariables m_thesisVariables;
		ImVec4*				m_headingColor;
		ImVec4*				m_subheadingColor;
		ImGuiContext*		m_context = nullptr;
		float				m_relativeScale = 1.0f;
		IntVec2				m_windowdim;
	//------------------RAYTRACING THESIS VARIABLES-------------------------
};