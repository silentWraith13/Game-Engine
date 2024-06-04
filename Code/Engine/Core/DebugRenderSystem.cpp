#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include <vector>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class DebugRenderObject
{
public:
	DebugRenderObject(float duration, const Rgba8& startColor = Rgba8::WHITE, const Rgba8& endColor = Rgba8::WHITE);
	void UpdateWorldObject();
	void UpdateScreenObjects();
	void RenderWorldObject(Camera const& camera, Renderer* renderer);
	void RenderScreenObject(Camera const& camera, Renderer* renderer);
	Rgba8 InterpolateColor(Rgba8 startColor, Rgba8 endColor, float duration);

	std::vector<Vertex_PCU> m_vertexes;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	Rgba8 m_lerpedColor = Rgba8::WHITE;
	float m_duration;
	Stopwatch* m_stopwatch;
	bool m_isWireFrame = false;
	DebugRenderMode m_debugRenderMode;
	BitmapFont* m_textFont2D;
	bool m_isGarbage = false;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////
//DebugRenderObjects class methods
/////////////////////////////////////////////////////////////////////
DebugRenderObject::DebugRenderObject(float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/)
	: m_duration(duration),
	m_startColor(startColor),
	m_endColor(endColor),
	m_stopwatch(nullptr)
{
	// Check if m_stopwatch is already initialized
	if (m_stopwatch == nullptr)
	{
		m_stopwatch = new Stopwatch(duration);
	}
	else
	{
		m_stopwatch->Restart();
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Rgba8 DebugRenderObject::InterpolateColor(Rgba8 startColor, Rgba8 endColor, float duration)
{
	float elapsedFraction = m_stopwatch->GetElapsedTime() / duration;
	elapsedFraction = GetClampedZeroToOne(elapsedFraction);

	Rgba8 result;
	result.r = static_cast<unsigned char>(Interpolate(static_cast<float>(startColor.r), static_cast<float>(endColor.r), elapsedFraction));
	result.g = static_cast<unsigned char>(Interpolate(static_cast<float>(startColor.g), static_cast<float>(endColor.g), elapsedFraction));
	result.b = static_cast<unsigned char>(Interpolate(static_cast<float>(startColor.b), static_cast<float>(endColor.b), elapsedFraction));
	result.a = static_cast<unsigned char>(Interpolate(static_cast<float>(startColor.a), static_cast<float>(endColor.a), elapsedFraction));

	return result;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class DebugRenderSystem
{
public:
	DebugRenderSystem(DebugRenderConfig const& debugRenderConfig);
	~DebugRenderSystem();

	void Update();
	void RenderWorld(Camera const& worldCamera);
	void RenderScreen(Camera const& screenCamera);

	std::vector<DebugRenderObject> m_worldObjectsList;
	std::vector<DebugRenderObject> m_screenObjectsList;
	DebugRenderConfig m_debugRenderConfig;
	Vec2 m_lastMessagePosition = Vec2(10.f, 760.0f);
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
static DebugRenderSystem* s_theDebugRenderSystem = nullptr;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////
//DebugRenderSystem class methods
/////////////////////////////////////////////////////////////////////
DebugRenderSystem::DebugRenderSystem(DebugRenderConfig const& debugRenderConfig)
{
	m_debugRenderConfig.m_renderer = debugRenderConfig.m_renderer;
	m_debugRenderConfig.m_startHidden = debugRenderConfig.m_startHidden;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
DebugRenderSystem::~DebugRenderSystem()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderSystem::Update()
{
	for (auto& renderObjects : m_worldObjectsList)
	{
		renderObjects.UpdateWorldObject();
	}
	for (auto& renderObjects : m_screenObjectsList)
	{
		renderObjects.UpdateScreenObjects();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderSystem::RenderWorld(Camera const& worldCamera)
{
	Renderer* renderer = m_debugRenderConfig.m_renderer;
	renderer->BeginCamera(worldCamera);
	for (auto& renderObjects : m_worldObjectsList) 
	{
		renderObjects.RenderWorldObject(worldCamera, renderer);
	}
	renderer->EndCamera(worldCamera);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderSystem::RenderScreen(Camera const& screenCamera)
{
	Renderer* renderer = m_debugRenderConfig.m_renderer;
	renderer->BeginCamera(screenCamera);

	// Render messages with infinite duration
	for (auto& renderObject : m_screenObjectsList)
	{
		if (renderObject.m_duration == -1.0f)
		{
			renderObject.RenderScreenObject(screenCamera, renderer);
		}
	}

	// Render messages with finite duration
	for (auto& renderObject : m_screenObjectsList)
	{
		if (renderObject.m_duration != -1.0f)
		{
			renderObject.RenderScreenObject(screenCamera, renderer);
		}
	}

	renderer->EndCamera(screenCamera);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////
//DebugRenderSystem global functions
/////////////////////////////////////////////////////////////////////
void DebugRenderSystemStartup(const DebugRenderConfig& config)
{
	if (s_theDebugRenderSystem == nullptr) 
	{
		s_theDebugRenderSystem = new DebugRenderSystem(config);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderSystemShutdown()
{
	if (s_theDebugRenderSystem != nullptr) 
	{
		delete s_theDebugRenderSystem;
		s_theDebugRenderSystem = nullptr;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderSetVisible()
{
	if (s_theDebugRenderSystem != nullptr) 
	{
		s_theDebugRenderSystem->m_debugRenderConfig.m_startHidden = false;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderSetHidden()
{
	if (s_theDebugRenderSystem != nullptr) 
	{
		s_theDebugRenderSystem->m_debugRenderConfig.m_startHidden = true;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderClear()
{
	if (s_theDebugRenderSystem != nullptr) 
	{
		s_theDebugRenderSystem->m_worldObjectsList.clear();
		s_theDebugRenderSystem->m_screenObjectsList.clear();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderBeginFrame()
{
	if (s_theDebugRenderSystem != nullptr) 
	{
		s_theDebugRenderSystem->Update();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderWorld(const Camera& camera)
{
	if (s_theDebugRenderSystem != nullptr) 
	{
		s_theDebugRenderSystem->RenderWorld(camera);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderScreen(const Camera& camera)
{
	if (s_theDebugRenderSystem != nullptr) 
	{
		s_theDebugRenderSystem->RenderScreen(camera);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderEndFrame()
{
	if (s_theDebugRenderSystem != nullptr) 
	{
		for (auto iterator = s_theDebugRenderSystem->m_worldObjectsList.begin(); iterator != s_theDebugRenderSystem->m_worldObjectsList.end(); )
		{
			if (iterator->m_isGarbage)
			{
				iterator = s_theDebugRenderSystem->m_worldObjectsList.erase(iterator);
			}
			else
			{
				iterator++;
			}
		}
		for (auto iterator = s_theDebugRenderSystem->m_screenObjectsList.begin(); iterator != s_theDebugRenderSystem->m_screenObjectsList.end(); )
		{
			if (iterator->m_isGarbage)
			{
				iterator = s_theDebugRenderSystem->m_screenObjectsList.erase(iterator);
			}
			else
			{
				iterator++;
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{	
	DebugRenderObject line = DebugRenderObject(duration, startColor, endColor);
	line.m_debugRenderMode = mode;
	AddVertsForLineSegment3D(line.m_vertexes, start, end, radius);
	line.m_stopwatch->Start();
	s_theDebugRenderSystem->m_worldObjectsList.push_back(line);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	DebugRenderObject cylinder = DebugRenderObject(duration, startColor, endColor);
	cylinder.m_isWireFrame = true;
	cylinder.m_debugRenderMode = mode;
	AddvertsForCylinderZ3D(cylinder.m_vertexes, base, top, radius);
	cylinder.m_stopwatch->Start();
	s_theDebugRenderSystem->m_worldObjectsList.push_back(cylinder);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	DebugRenderObject sphere = DebugRenderObject(duration, startColor, endColor);
	sphere.m_debugRenderMode = mode;
	AddVertsForSphere3D(sphere.m_vertexes, pos, radius);
	sphere.m_stopwatch->Start();
	s_theDebugRenderSystem->m_worldObjectsList.push_back(sphere);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	DebugRenderObject sphere = DebugRenderObject(duration, startColor, endColor);
	sphere.m_debugRenderMode = mode;
	sphere.m_isWireFrame = true;
	AddVertsForSphere3D(sphere.m_vertexes, center, radius);
	sphere.m_stopwatch->Start();
	s_theDebugRenderSystem->m_worldObjectsList.push_back(sphere);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	DebugRenderObject arrow = DebugRenderObject(duration, startColor, endColor);
	arrow.m_debugRenderMode = mode;
	AddVertsForArrow3D(arrow.m_vertexes, start, end, radius);
	arrow.m_stopwatch->Start();
	s_theDebugRenderSystem->m_worldObjectsList.push_back(arrow);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugAddScreenText(const std::string& text, const Vec2& position, float size, const Vec2& alignment, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/)
{
	(void)alignment;
	(void)endColor;
	DebugRenderObject text2D = DebugRenderObject(duration);
	std::string fontName = "SquirrelFixedFont";
	text2D.m_textFont2D = s_theDebugRenderSystem->m_debugRenderConfig.m_renderer->CreateOrGetBitmapFontFromFile(("Data/Fonts/" + fontName).c_str());
	text2D.m_textFont2D->AddVertsForText2D(text2D.m_vertexes, position, size, text, startColor);
	text2D.m_stopwatch->Start();
	s_theDebugRenderSystem->m_screenObjectsList.push_back(text2D);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/)
{
	DebugRenderObject message = DebugRenderObject(duration);
	message.m_duration = duration;
	message.m_startColor = startColor;
	message.m_endColor = endColor;
	message.m_stopwatch->Start();

	// Calculate the position of the new message
	float messageSize = 10.0f; // You can adjust the size of the message text
	Vec2 position = s_theDebugRenderSystem->m_lastMessagePosition;
	position.y -= messageSize; // 1.2 is for spacing between messages
	s_theDebugRenderSystem->m_lastMessagePosition = position;

	DebugAddScreenText(text, position, messageSize, Vec2(0, 0), duration, startColor, endColor);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderObject::UpdateWorldObject()
{
	 //calculate the elapsed fraction of the duration
	float elapsedFraction = m_stopwatch->GetElapsedTime() / m_duration;
	elapsedFraction = GetClampedZeroToOne(elapsedFraction);

	// interpolate the start and end colors to get the current color
	m_lerpedColor = InterpolateColor(m_startColor, m_endColor, m_duration);

	// update the color of the vertices with the current color
	for (Vertex_PCU& vertex : m_vertexes)
	{
		vertex.m_color = m_lerpedColor;
	}

	if (m_duration == -1.f)
		return;

	if (m_stopwatch->HasDurationElapsed())
	{
		m_vertexes.clear();
		m_stopwatch->Stop();
		m_isGarbage = true;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderObject::UpdateScreenObjects()
{
	
	if (m_stopwatch->HasDurationElapsed())
	{
		m_vertexes.clear();
		m_stopwatch->Stop();
		m_isGarbage = true;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderObject::RenderWorldObject(Camera const& camera, Renderer* renderer)
{
	(void)camera;
	renderer->BindTexture(nullptr);
	renderer->SetBlendMode(BlendMode::ALPHA);

	/*if (m_debugRenderMode == DebugRenderMode::ALWAYS)
	{
		if (m_isWireFrame)
		{
			renderer->SetRasterizerModes(RasterizerMode::WIREFRAME_CULL_BACK);
		}
		else
		{
			renderer->SetRasterizerModes(RasterizerMode::SOLID_CULL_BACK);
		}
		renderer->SetDepthModes(DepthMode::DISABLED);
		renderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data());
	}*/
	if (m_debugRenderMode == DebugRenderMode::USE_DEPTH)
	{
		if (m_isWireFrame)
		{
			renderer->SetRasterizerModes(RasterizerMode::WIREFRAME_CULL_BACK);
		}
		else
		{
			renderer->SetRasterizerModes(RasterizerMode::SOLID_CULL_BACK);
		}
		renderer->SetDepthModes(DepthMode::ENABLED);
		renderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data());
	}
	if (m_debugRenderMode == DebugRenderMode::X_RAY)
	{
		if (m_isWireFrame)
		{
			renderer->SetRasterizerModes(RasterizerMode::WIREFRAME_CULL_BACK);
		}
		else
		{
			renderer->SetRasterizerModes(RasterizerMode::SOLID_CULL_BACK);
		}
		 //Pass 1 - draw with alpha and without depth test, using a lightened and slightly transparent color
		renderer->SetBlendMode(BlendMode::ALPHA);
 		renderer->SetDepthModes(DepthMode::DISABLED);
 		Rgba8 lightenedColor = Interpolate(m_startColor, m_endColor, 0.8f);
 		lightenedColor.a = 50;
 		renderer->SetModelConstants(Mat44(), lightenedColor);
 		renderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data());

		 //Pass 2 - draw with opaque and with depth test, using the current color
		renderer->SetBlendMode(BlendMode::OPAQUE);
		renderer->SetDepthModes(DepthMode::ENABLED);
		renderer->SetModelConstants(Mat44(), m_startColor);
		renderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data());
	}

	renderer->SetModelConstants();
	renderer->SetRasterizerModes(RasterizerMode::SOLID_CULL_BACK);
	renderer->SetDepthModes(DepthMode::ENABLED);
	renderer->SetBlendMode(BlendMode::ALPHA);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DebugRenderObject::RenderScreenObject(Camera const& camera, Renderer* renderer)
{
	(void)camera;
	renderer->BindTexture(&m_textFont2D->GetTexture());
	renderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
