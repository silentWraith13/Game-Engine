#include "D3D12ImGui.hpp"
//#include "ThirdParty/ImGui/imgui.h"

#include "ThirdParty/ImGui/imgui_impl_dx12.h"
#include "ThirdParty/ImGui/imgui_impl_win32.h"
#include "ThirdParty/ImGui/imgui_internal.h"

extern RendererD12* g_theRendererDx12;

void D3D12ImGui::InitializeImGui()
{
	auto device = g_theRendererDx12->GetDevice();
	auto imguiHeap = g_theRendererDx12->GetIMGUIDescriptorHeap();
	IMGUI_CHECKVERSION();
	m_context = ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(g_theRendererDx12->GetRenderConfig().m_window->GetHwnd());
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = imguiHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = imguiHeap->GetGPUDescriptorHandleForHeapStart();
	m_color = new ImColor(255, 255, 255);
	ImGui_ImplDX12_Init(device, 3, DXGI_FORMAT_R8G8B8A8_UNORM, imguiHeap,
		cpuHandle, gpuHandle);
	ImGui_ImplDX12_CreateDeviceObjects();
	m_windowdim = g_theRendererDx12->GetRenderConfig().m_window->GetClientDimensions();
	ImGui::GetIO().ImeWindowHandle = g_theRendererDx12->GetRenderConfig().m_window->GetHwnd();

	m_relativeScale = g_theRendererDx12->m_dimensions.x / static_cast<float>(m_windowdim.x);
	//ImGui::GetIO().DisplaySize =  ImVec2(windowdim.x, windowdim.y);
	/*ImGui::GetCurrentWindow();*/
	m_headingColor = new ImVec4(0.0f,1.0f,1.0f,1.0f);
	m_subheadingColor = new ImVec4(0.0f,0.5f,0.5f,1.0f);
	
	//ImGui::GetStyle().ScaleAllSizes(m_relativeScale);
	//ImGui::GetIO().FontGlobalScale = m_relativeScale;
}

void D3D12ImGui::RenderImGui()
{

	//ImGui::GetStyle().ScaleAllSizes(m_relativeScale);
	//ImGui::GetIO().DisplaySize = ImVec2(m_windowdim.x, m_windowdim.y);


	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	
	//for (int i = 0; i < m_context->Windows.Size; i++)
	//{
	//	ImGuiWindow* window = m_context->Windows[i];
	//	//ImGui::SetWindowPos(window, window.Pos * m_relativeScale);
	//	Vec2 windowSize = Vec2(window->Size.x, window->Size.y);
	//	windowSize *= m_relativeScale;
	//	ImGui::SetWindowSize(window, ImVec2(windowSize.x, windowSize.y));
	//}
	ImGui::NewFrame();
	ImGui::Begin("Raytraced Global Illumination ");
	//ImGui::SetWindowFontScale(m_relativeScale);
	ImGui::TextColored(*m_headingColor, "Player Info : ");
	ImGui::SameLine();
	ImGui::Text(m_playerInfoText.c_str());

	ImGui::TextColored(*m_headingColor, "Game Info : ");
	ImGui::SameLine();
	ImGui::Text(m_gameInfoText.c_str());
	ImGui::Text("F1-F7 Different Scenarios");
	ImGui::TextColored(*m_headingColor, "Controls : ");
	ImGui::SameLine();
	ImGui::Text("(SPACE) for Activate IMGUI (WASD) To Move Camera (IJKL, MN) to move Light");
	ImGui::InputFloat("Reflection Count", &m_thesisVariables.m_samplingData.x, 1.0f,0.0f, "%.0f");
	ImGui::InputFloat("Samples Count", &m_thesisVariables.m_samplingData.y, 1.0f,0.0f, "%.0f");
	float lightFalloff = m_thesisVariables.m_lightFallof;
	float ambientIntensity = m_thesisVariables.m_ambientIntensity;
	ImGui::SliderFloat("Light Falloff", &lightFalloff, 0.0f, 1.0, "%.2f", 0);
	m_thesisVariables.m_lightFallof = lightFalloff;
	ImGui::SliderFloat("Ambient Intensity", &ambientIntensity, 0.0f, 1.0, "%.2f", 0);
	m_thesisVariables.m_ambientIntensity = ambientIntensity;
	//ImGui::InputFloat("Sun Radius", &m_thesisVariables.m_sunRadius, 0.0f,0.0f, "%.0f");
	ImGui::TextColored(*m_headingColor, "Render Target G Buffer");
	ImGui::Text("0-Final Render 1-Depth, 2-Albedo, 3-DI , 4-GI");
	ImGui::Text("5-Motion Vector, 6-History Buffer, 7-Temporal output, 8-Variance, 9-Partial Depth,10-Normals");
	ImGui::InputFloat("Render Target", &m_thesisVariables.m_renderOutput, 0.0f,0.0f, "%.0f");

	if (m_thesisVariables.m_renderOutput > 11)
	{
		m_thesisVariables.m_renderOutput = 11;
	}
	if (m_thesisVariables.m_renderOutput < 0)
	{
		m_thesisVariables.m_renderOutput = 0;
	}
	bool Shadows = (m_thesisVariables.m_lightBools.x == 1);
	bool indirect = (m_thesisVariables.m_lightBools.y == 1);
	bool skyLight = (m_thesisVariables.m_lightBools.z == 1);
	bool directLight = (m_thesisVariables.m_lightBools.w == 1);
	bool colorBleeding = (m_thesisVariables.m_samplingData.z == 1);
	bool denoising = m_thesisVariables.m_denoiserOn;
	bool temporalSampler = m_thesisVariables.m_temporalSamplerOn;
	bool godRays = m_thesisVariables.m_godRaysOn;
	bool varianceFiltering = m_thesisVariables.m_varianceFiltering;
	float temporalFade = m_thesisVariables.m_temporalFade;
	

	bool normalMaps = m_thesisVariables.m_textureMappings.x;
	bool specularMaps = m_thesisVariables.m_textureMappings.y;

	bool waterReflections = m_thesisVariables.m_textureMappings.z;
	bool waterRefractions = m_thesisVariables.m_textureMappings.w;

	static int samplingMode = m_thesisVariables.m_samplingMode;
	ImGui::RadioButton("Random Sampling", &samplingMode, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Cosine Weighted Sampling", &samplingMode, 1);

	m_thesisVariables.m_samplingMode = (float)samplingMode;

	ImGui::Checkbox("Direct Light", &directLight);	ImGui::SameLine();
	ImGui::Checkbox("GlobalIllumination", &indirect);	ImGui::SameLine();
	ImGui::Checkbox("Shadows", &Shadows);	ImGui::SameLine();
	ImGui::Checkbox("SkyLight", &skyLight);
	ImGui::Checkbox("Color Bleeding", &colorBleeding);	ImGui::SameLine();
	ImGui::Checkbox("TemporalSampler", &temporalSampler);	ImGui::SameLine();
	ImGui::Checkbox("GodRays", &godRays);	
	ImGui::SliderFloat("Temporal Fade", &temporalFade, 0.0f,0.2f, "%.3f",0);
	m_thesisVariables.m_temporalFade = temporalFade;

	ImGui::Checkbox("Normal Maps", &normalMaps); ImGui::SameLine();
	ImGui::Checkbox("Specular Maps", &specularMaps);
	ImGui::Checkbox("Water Reflections", &waterReflections); ImGui::SameLine();
	ImGui::Checkbox("Water Refractions", &waterRefractions);
	ImGui::Checkbox("Denoiser", &denoising); ImGui::SameLine();
	ImGui::Checkbox("Variance filtering", &varianceFiltering);

	m_thesisVariables.m_lightBools.x = Shadows && directLight;
	m_thesisVariables.m_lightBools.y = indirect;
	m_thesisVariables.m_lightBools.w = directLight;
	m_thesisVariables.m_lightBools.z = skyLight;
	m_thesisVariables.m_samplingData.z = colorBleeding;
	m_thesisVariables.m_denoiserOn = denoising;
	m_thesisVariables.m_godRaysOn = godRays;
	m_thesisVariables.m_temporalSamplerOn = temporalSampler;
	m_thesisVariables.m_varianceFiltering = varianceFiltering;
	m_thesisVariables.m_textureMappings =Vec4(normalMaps, specularMaps , waterReflections, waterRefractions);

	static int denoiser = m_thesisVariables.m_denoiserType;
	static int scene = m_thesisVariables.m_currentScene;
	ImGui::Text("Denoiser Type");
	ImGui::SameLine();
	ImGui::RadioButton("Gaussian", &denoiser, 0); ImGui::SameLine();
	ImGui::RadioButton("Atrous", &denoiser, 1);
	//ImGui::RadioButton("Minecraft", &scene, 0); 
	//ImGui::SameLine();ImGui::RadioButton("Bunny", &scene, 1);
	m_thesisVariables.m_denoiserType = denoiser;
	m_thesisVariables.m_currentScene = scene;

	static int kernel = 3;
	if (denoiser == 0)
	{
		ImGui::Text("Denoiser Kernel Size");
		ImGui::SameLine();
		ImGui::RadioButton("3X3", &kernel, 3); ImGui::SameLine();
		ImGui::RadioButton("7X7", &kernel, 7);
	}
	else
	{
		ImGui::Text("Atrous Step Count");
		ImGui::SameLine();
		if (ImGui::Button("-"))
			m_thesisVariables.m_atrousStepSize -= 1;

		ImGui::SameLine();
		if (ImGui::Button("+"))
			m_thesisVariables.m_atrousStepSize += 1;

		int stepSize = m_thesisVariables.m_atrousStepSize;
		ImGui::SameLine();
		ImGui::Text(std::to_string(stepSize).c_str());

	}
	m_thesisVariables.m_atrousStepSize > 5 ? m_thesisVariables.m_atrousStepSize = 5 : m_thesisVariables.m_atrousStepSize;
	m_thesisVariables.m_denoiserKernelSize = kernel;



	float color[4] = { m_thesisVariables.m_color.x , m_thesisVariables.m_color.y, m_thesisVariables.m_color.z, m_thesisVariables.m_color.w};
	ImGui::ColorEdit4("Light Color   ", color);
	m_thesisVariables.m_color.x = color[0];
	m_thesisVariables.m_color.y = color[1];
	m_thesisVariables.m_color.z = color[2];
	m_thesisVariables.m_color.w = color[3];

	ImGui::End();
	ImGui::Render();
	auto heap = g_theRendererDx12->GetIMGUIDescriptorHeap();
	auto renderTarget = g_theRendererDx12->GetBackBufferCPUHandle();
	g_theRendererDx12->GetCommandList()->SetDescriptorHeaps(1, &heap);
	g_theRendererDx12->GetCommandList()->OMSetRenderTargets(1, renderTarget, FALSE, NULL);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_theRendererDx12->GetCommandList());
}

void D3D12ImGui::UpdateImGui()
{
}

void D3D12ImGui::ShutdownImGui()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

bool D3D12ImGui::ImGuiColorPicker(const char* label, ImColor* color)
{
	static const float HUE_PICKER_WIDTH = 20.0f;
	static const float CROSSHAIR_SIZE = 7.0f;
	static const ImVec2 SV_PICKER_SIZE = ImVec2(200, 200);

	bool value_changed = false;

	//ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 picker_pos = ImGui::GetCursorScreenPos();

	/*ImColor colors[] = { ImColor(255, 0, 0),
		ImColor(255, 255, 0),
		ImColor(0, 255, 0),
		ImColor(0, 255, 255),
		ImColor(0, 0, 255),
		ImColor(255, 0, 255),
		ImColor(255, 0, 0) };*/

		//for (int i = 0; i < 6; ++i)
		//{
		//	draw_list->AddRectFilledMultiColor(
		//		ImVec2(picker_pos.x + SV_PICKER_SIZE.x + 10, picker_pos.y + i * (SV_PICKER_SIZE.y / 6)),
		//		ImVec2(picker_pos.x + SV_PICKER_SIZE.x + 10 + HUE_PICKER_WIDTH,
		//			picker_pos.y + (i + 1) * (SV_PICKER_SIZE.y / 6)),
		//		colors[i],
		//		colors[i],
		//		colors[i + 1],
		//		colors[i + 1]);
		//}

	float hue, saturation, value;
	ImGui::ColorConvertRGBtoHSV(
		color->Value.x, color->Value.y, color->Value.z, hue, saturation, value);
	auto hue_color = ImColor::HSV(hue, 1, 1);

	//draw_list->AddLine(
	//	ImVec2(picker_pos.x + SV_PICKER_SIZE.x + 8, picker_pos.y + hue * SV_PICKER_SIZE.y),
	//	ImVec2(picker_pos.x + SV_PICKER_SIZE.x + 12 + HUE_PICKER_WIDTH,
	//		picker_pos.y + hue * SV_PICKER_SIZE.y),
	//	ImColor(255, 255, 255));


	float x = saturation * value;
	ImVec2 p(picker_pos.x + x * SV_PICKER_SIZE.x, picker_pos.y /*+ value * SV_PICKER_SIZE.y*/);
	/*draw_list->AddLine(ImVec2(p.x - CROSSHAIR_SIZE, p.y), ImVec2(p.x - 2, p.y), ImColor(255, 255, 255));
	draw_list->AddLine(ImVec2(p.x + CROSSHAIR_SIZE, p.y), ImVec2(p.x + 2, p.y), ImColor(255, 255, 255));
	draw_list->AddLine(ImVec2(p.x, p.y + CROSSHAIR_SIZE), ImVec2(p.x, p.y + 2), ImColor(255, 255, 255));
	draw_list->AddLine(ImVec2(p.x, p.y - CROSSHAIR_SIZE), ImVec2(p.x, p.y - 2), ImColor(255, 255, 255));*/

	//ImGui::InvisibleButton("saturation_value_selector", SV_PICKER_SIZE);
	//if (ImGui::IsItemHovered())
	//{
	//	ImVec2 mouse_pos_in_canvas = ImVec2(
	//		ImGui::GetIO().MousePos.x - picker_pos.x, ImGui::GetIO().MousePos.y - picker_pos.y);
	//	if (ImGui::GetIO().MouseDown[0])
	//	{
	//		mouse_pos_in_canvas.x =
	//			ImMin(mouse_pos_in_canvas.x, mouse_pos_in_canvas.y);

	//		value = mouse_pos_in_canvas.y / SV_PICKER_SIZE.y;
	//		saturation = value == 0 ? 0 : (mouse_pos_in_canvas.x / SV_PICKER_SIZE.x) / value;
	//		value_changed = true;
	//	}
	//}

	ImGui::SetCursorScreenPos(ImVec2(picker_pos.x + SV_PICKER_SIZE.x + 10, picker_pos.y));
	ImGui::InvisibleButton("hue_selector", ImVec2(HUE_PICKER_WIDTH, SV_PICKER_SIZE.y));

	if (ImGui::IsItemHovered())
	{
		if (ImGui::GetIO().MouseDown[0])
		{
			hue = ((ImGui::GetIO().MousePos.y - picker_pos.y) / SV_PICKER_SIZE.y);
			value_changed = true;
		}
	}

	*color = ImColor::HSV(hue, saturation, value);
	return value_changed | ImGui::ColorEdit3(label, &color->Value.x);
}

