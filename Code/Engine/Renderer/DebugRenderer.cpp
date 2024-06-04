#include "DebugRenderer.hpp"
#include "Engine/Math/VertexUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <mutex>

const Rgba8 Error = Rgba8::RED;
const Rgba8 Warning = Rgba8(255, 255, 0, 255);
const Rgba8 INFO_MAJOR = Rgba8(255, 255, 255, 255);
const Rgba8 INFO_MINOR = Rgba8(180, 180, 180, 255);
const Rgba8 InputTextColor = Rgba8::WHITE;

static std::mutex			m_debugRendererMutex;
struct DebugRendererText
{
	std::string text;
	Rgba8 color;
	Vec2 alignment;
	Vec2 position;
	Vec3 position3D;
	Rgba8 startColor;
	Rgba8 endColor;
	float size;
	float duration;
	float textHeight;
	DebugRenderMode renderMode;
	Stopwatch watch;
	std::vector<Vertex_PCU> verts;
	Mat44 modelMatrix;
};

struct DebugRenderObject
{
	std::vector<Vertex_PCU> verts;
	Vec3 position;
	Rgba8 startColor;
	Rgba8 endColor;
	float duration;
	FillMode fillMode;
	DebugRenderMode renderMode;
	Stopwatch watch;
};

enum class Visibility
{
	Hidden,
	Visible
};
class DebugRenderEntity
{
public:
	DebugRenderEntity(DebugRenderConfig config)
	{
		m_debugRenderConfig = config;
		if (config.m_startHidden)
		{
			m_visibility = Visibility::Hidden;
		}
		else
			m_visibility = Visibility::Visible;
	}
	~DebugRenderEntity()
	{

	}
	void Startup()
	{
		m_clock = new Clock();
	}

	void BeginFrame()
	{
		Update();
	}

	void EndFrame()
	{
	}

	void ShutDown()
	{	
		delete this;
	}

	void Update()
	{

	}

	void SetVisible()
	{
		m_visibility = Visibility::Visible;
	}

	void SetHidden()
	{
		m_visibility = Visibility::Hidden;
	}

	void RenderClear()
	{
		m_debugRenderObjects.clear();
		m_debugScreenMessages.clear();
		m_billBoardTexts.clear();
		m_worldTexts.clear();
	}

	void SetParentClock(Clock& parent)
	{
		m_clock->SetParent(parent);
	}

	DebugRenderConfig GetDebugRenderConfig()
	{
		return m_debugRenderConfig;
	}

	Visibility GetDebugRendererVisibility()
	{
		return m_visibility;
	}

	void SetCamera(Camera const& camera)
	{
		m_currentRenderCamera = camera;
	}

	void RenderScreen(Camera const& camera)
	{
		if (m_visibility == Visibility::Visible)
		{
			m_debugRenderConfig.m_renderer->BeginCamera(camera);
			m_debugRenderConfig.m_renderer->BindShader(m_debugRenderConfig.m_renderer->GetDefaultShader());
			m_debugRenderConfig.m_renderer->BindTexture(nullptr);
			m_debugRenderConfig.m_renderer->SetBlendMode(BlendMode::ALPHA);
			m_debugRenderConfig.m_renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
			m_debugRenderConfig.m_renderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
			m_debugRenderConfig.m_renderer->SetSamplerMode(SamplerMode::POINTCLAMP);
			std::vector<Vertex_PCU> debugRendererTextVerts;
			AABB2 bounds = camera.GetCameraBounds();
			AABB2 debugRendererInputBounds;
			debugRendererInputBounds.m_mins = bounds.m_mins;
			debugRendererInputBounds.m_maxs.x = bounds.m_maxs.x;
			debugRendererInputBounds.m_maxs.y = bounds.m_mins.y + m_debugRendererTextHeight;
			BitmapFont* font = m_debugRenderConfig.m_bitmapFont;
			for (int i = 0; i < m_debugRendererTexts.size(); i++)
			{
				font->AddVertsForTextInBox2D(debugRendererTextVerts, bounds, m_debugRendererTextHeight, m_debugRendererTexts[i].text, m_debugRendererTexts[i].startColor,
					0.6f, m_debugRendererTexts[i].alignment, TextBoxMode::SHRINK_TO_FIT, 999999, 5.0f);
			}

			for (int i = 0; i < (int)m_debugScreenMessages.size() ; i++)
			{
				if (!m_debugScreenMessages[i].watch.HasDurationElapsed() || m_debugScreenMessages[i].duration == -1.0f)
				{
					bounds.m_maxs.y -= m_debugScreenMessagesHeight;
					font->AddVertsForTextInBox2D(debugRendererTextVerts, bounds, m_debugScreenMessagesHeight, m_debugScreenMessages[i].text, m_debugScreenMessages[i].color,
						0.6f, Vec2(0.0f, 1.0f), TextBoxMode::SHRINK_TO_FIT, 999999, 5.0f);
				}
			}
			m_debugRenderConfig.m_renderer->BindTexture(&font->GetTexture());
			m_debugRenderConfig.m_renderer->DrawVertexArray(int(debugRendererTextVerts.size()), debugRendererTextVerts.data());
			m_debugRenderConfig.m_renderer->BindTexture(nullptr);
			font = nullptr;
			m_debugRendererTexts.clear();
			debugRendererTextVerts.clear();
			m_debugRenderConfig.m_renderer->EndCamera(camera);
		}
		
	}

	void RenderWorld(Camera const& camera)
	{
		m_camera = camera;
		if (m_visibility == Visibility::Visible)
		{
			m_debugRenderConfig.m_renderer->BeginCamera(camera);
			m_debugRenderConfig.m_renderer->BindShader(m_debugRenderConfig.m_renderer->GetDefaultShader());
			for(int i  = 0;  i< m_debugRenderObjects.size(); i++)
			{
				if (!m_debugRenderObjects[i].watch.HasDurationElapsed() || m_debugRenderObjects[i].duration == -1.0f)
				{
 					Rgba8 color;
					Rgba8 startColor = m_debugRenderObjects[i].startColor;
					Rgba8 endColor = m_debugRenderObjects[i].endColor;
					FillMode mode = m_debugRenderObjects[i].fillMode;
					if (m_debugRenderObjects[i].duration != -1.0f)
					{
						for (int j = 0; j < m_debugRenderObjects[i].verts.size(); j++)
						{
							float fraction = m_debugRenderObjects[i].watch.GetElapsedFraction();
							color.r = (unsigned char)RangeMap(fraction, (unsigned char)0.0f, (unsigned char)1.0f, startColor.r, endColor.r);
							color.g = (unsigned char)RangeMap(fraction, (unsigned char)0.0f, (unsigned char)1.0f, startColor.g, endColor.g);
							color.b = (unsigned char)RangeMap(fraction, (unsigned char)0.0f, (unsigned char)1.0f, startColor.b, endColor.b);
		
							m_debugRenderObjects[i].verts[j].m_color = color;
						}

					}
					m_debugRenderConfig.m_renderer->SetBlendMode(BlendMode::Opaque);
					m_debugRenderConfig.m_renderer->SetRasterizerState(CullMode::NONE, mode, WindingOrder::COUNTERCLOCKWISE);
					m_debugRenderConfig.m_renderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
					m_debugRenderConfig.m_renderer->SetSamplerMode(SamplerMode::POINTCLAMP);
					m_debugRenderConfig.m_renderer->DrawVertexArray((int)m_debugRenderObjects[i].verts.size(), m_debugRenderObjects[i].verts);

					if (m_debugRenderObjects[i].renderMode == DebugRenderMode::XRAY)
					{
						m_debugRenderConfig.m_renderer->SetBlendMode(BlendMode::Opaque);
						m_debugRenderConfig.m_renderer->SetRasterizerState(CullMode::NONE, mode, WindingOrder::COUNTERCLOCKWISE);
						m_debugRenderConfig.m_renderer->SetDepthStencilState(DepthTest::LESSEQUAL, false);
						m_debugRenderConfig.m_renderer->SetSamplerMode(SamplerMode::BILINEARWRAP);
						float modelColor[4];
						Rgba8 startColors = m_debugRenderObjects[i].startColor;
						Rgba8 halfAlpha = Rgba8((unsigned char)startColors.r, (unsigned char)startColors.g, (unsigned char)startColors.b, (unsigned char)(startColors.a * 0.5f));
						halfAlpha.GetAsFloats(modelColor);
						m_debugRenderConfig.m_renderer->SetModalColor(modelColor);
						m_debugRenderConfig.m_renderer->DrawVertexArray((int)m_debugRenderObjects[i].verts.size(), m_debugRenderObjects[i].verts);
						float modelColor2[4];
						Rgba8::WHITE.GetAsFloats(modelColor2);
						m_debugRenderConfig.m_renderer->SetModalColor(modelColor2);
					}
				}
			}
			m_debugRenderConfig.m_renderer->BindTexture(&m_debugRenderConfig.m_bitmapFont->GetTexture());
			for (int i = 0; i < m_billBoardTexts.size(); i++)
			{
				Vec3 normalizedAlignment = Vec3(0.5f, 0.5f, 0.0f);
				AABB3 bounds = GetVertexBounds(m_billBoardTexts[i].verts);
				Vec3 alignmentVec(-normalizedAlignment.x * bounds.GetDimensions().x, -normalizedAlignment.y * bounds.GetDimensions().y, 0.0f);
				Mat44 alignmentMatrix = Mat44::CreateTranslation3D(alignmentVec);
				Mat44 facingMatrix = Mat44(Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f),Vec3());

				const Camera cameras = m_debugRenderConfig.m_renderer->GetCurrentCamera();
				Mat44 cameraMatrix = cameras.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
				Mat44 billboardMatrix = Mat44(-cameraMatrix.GetIBasis3D(), -cameraMatrix.GetJBasis3D(), cameraMatrix.GetKBasis3D(), Vec3());

				Mat44 modelMatrix = m_billBoardTexts[i].modelMatrix;
				modelMatrix.Append(billboardMatrix);
				modelMatrix.Append(facingMatrix);
				modelMatrix.Append(alignmentMatrix);

				if (!m_billBoardTexts[i].watch.HasDurationElapsed() || m_billBoardTexts[i].duration == -1.0f)
				{
					Rgba8 color;
					Rgba8 startColor = m_billBoardTexts[i].startColor;
					Rgba8 endColor = m_billBoardTexts[i].endColor;
					for (int j = 0; j < m_billBoardTexts[i].verts.size(); j++)
					{
						float fraction = m_billBoardTexts[i].watch.GetElapsedFraction();
						color.r = (unsigned char)RangeMap(fraction, (unsigned char)0.0f, (unsigned char)1.0f, startColor.r, endColor.r);
						color.g = (unsigned char)RangeMap(fraction, (unsigned char)0.0f, (unsigned char)1.0f, startColor.g, endColor.g);
						color.b = (unsigned char)RangeMap(fraction, (unsigned char)0.0f, (unsigned char)1.0f, startColor.b, endColor.b);
						m_billBoardTexts[i].verts[j].m_color = color;
					}
					m_debugRenderConfig.m_renderer->SetModalMatrix(modelMatrix);
					m_debugRenderConfig.m_renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
					m_debugRenderConfig.m_renderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
					m_debugRenderConfig.m_renderer->SetSamplerMode(SamplerMode::POINTCLAMP);
					m_debugRenderConfig.m_renderer->DrawVertexArray((int)m_billBoardTexts[i].verts.size(), m_billBoardTexts[i].verts.data());
					m_debugRenderConfig.m_renderer->SetModalMatrix(Mat44());
				}
			
			}
			for (int i = 0; i < m_worldTexts.size(); i++)
			{
				Mat44 tranlationMatrix = Mat44::CreateTranslation3D(m_debugRenderConfig.m_renderer->GetModelMatrix().GetTranslation3D());
				Mat44 modelMatrix = m_worldTexts[i].modelMatrix;
				modelMatrix.Append(tranlationMatrix);
				if (!m_worldTexts[i].watch.HasDurationElapsed() || m_worldTexts[i].duration == -1.0f)
				{
					Rgba8 color;
					Rgba8 startColor = m_worldTexts[i].startColor;
					Rgba8 endColor = m_worldTexts[i].endColor;
					m_debugRenderConfig.m_renderer->SetModalMatrix(modelMatrix);
					m_debugRenderConfig.m_renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
					m_debugRenderConfig.m_renderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
					m_debugRenderConfig.m_renderer->SetSamplerMode(SamplerMode::POINTCLAMP);
					m_debugRenderConfig.m_renderer->DrawVertexArray((int)m_worldTexts[i].verts.size(), m_worldTexts[i].verts.data());
				}

			}
			m_debugRenderConfig.m_renderer->BindTexture(nullptr);
			m_debugRenderConfig.m_renderer->EndCamera(camera);
		}
	}

	void AddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		UNUSED((void)radius);
		DebugRenderObject debugObject;
		std::vector<Vertex_PCU> verts;

		debugObject.verts = verts;
		debugObject.position = pos;
		debugObject.duration = duration;
		debugObject.fillMode = FillMode::WIREFRAME;
		debugObject.startColor = startColor;
		debugObject.endColor = endColor;
		debugObject.renderMode = mode;
		debugObject.watch.Start(m_clock, duration);
		m_debugRenderObjects.push_back(debugObject);
	}

	void AddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		DebugRenderObject object;
		std::vector<Vertex_PCU> verts;
		Vec3 worldxBasis = Vec3(1.0f, 0.0f, 0.0f);
		Vec3 worldyBasis = Vec3(0.0f, 1.0f, 0.0f);

		Vec3 kBasis = (start - end).GetNormalized();
		Vec3 iBasis, jBasis;
		if (abs(DotProduct3D(kBasis, worldxBasis)) < 1)
		{
			jBasis = CrossProduct3D(kBasis, worldxBasis).GetNormalized();
			iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
		}
		else
		{
			jBasis = CrossProduct3D(kBasis, worldyBasis).GetNormalized();
			iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
		}

		float height = (end - start).GetLength();
		Mat44 transforMatrix = Mat44(iBasis, jBasis, kBasis, Vec3());
		//transforMatrix.AppendTranslation3D(start);
		AddVertsForZCylinder(verts, height, radius, AABB2::ZERO_TO_ONE, startColor);
		TransformVertexArrayUsingMatrix3D((int)verts.size(), verts, transforMatrix);
		TransformVertexArray3D((int)verts.size(), verts, 1.0f, 0.0f, end);
		object.verts = verts;
		object.duration = duration;
		object.fillMode = FillMode::SOLID;
		object.startColor = startColor;
		object.endColor = endColor;
		object.renderMode = mode;
		object.watch.Start(m_clock, duration);
		m_debugRenderObjects.push_back(object);
	}

	void AddWorldPlayerLine(const Mat44& basis, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		UNUSED((void)startColor);
		UNUSED((void)endColor);
		UNUSED((void)mode);
		Vec3 translation = basis.GetTranslation3D();
		Vec3 IBasis = translation + basis.GetIBasis3D() * 0.1f;
		Vec3 JBasis = translation + basis.GetJBasis3D() * 0.1f;
		Vec3 KBasis = translation + basis.GetKBasis3D() * 0.1f;

		DebugAddWorldArrow(translation, IBasis, 0.001f, duration, Rgba8::RED, Rgba8::RED, Rgba8::RED, DebugRenderMode::USEDEPTH);
		DebugAddWorldArrow(translation, JBasis, 0.001f, duration, Rgba8::GREEN, Rgba8::GREEN, Rgba8::RED, DebugRenderMode::USEDEPTH);
		DebugAddWorldArrow(translation, KBasis, 0.001f, duration, Rgba8::BLUE, Rgba8::BLUE, Rgba8::RED, DebugRenderMode::USEDEPTH);
	}
	void AddWorldWireCylinder(const Vec3& base, const Vec3& top,const Vec3& position, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		DebugRenderObject debugRenderCylinder;
		std::vector<Vertex_PCU> verts;
		float height = (top - base).GetLength();
		AddVertsForZCylinder(verts, height,radius, AABB2::ZERO_TO_ONE, startColor);
		TransformVertexArray3D((int)verts.size(), verts, 1.0f, 0.0f, position);
		debugRenderCylinder.verts = verts;
		debugRenderCylinder.position = position;
		debugRenderCylinder.duration = duration;
		debugRenderCylinder.fillMode = FillMode::WIREFRAME;
		debugRenderCylinder.startColor = startColor;
		debugRenderCylinder.endColor = endColor;
		debugRenderCylinder.renderMode = mode;
		debugRenderCylinder.watch.Start(m_clock, duration);
		m_debugRenderObjects.push_back(debugRenderCylinder);
	}

	void AddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		DebugRenderObject debugRenderSphere;
		std::vector<Vertex_PCU> verts;
		AddVertsForSphere3D(verts, radius, Vec3(), AABB2::ZERO_TO_ONE, startColor);
		TransformVertexArray3D((int)verts.size(),verts,1.0f,0.0f, center);
		debugRenderSphere.verts = verts;
		debugRenderSphere.position = center;
		debugRenderSphere.duration = duration;
		debugRenderSphere.fillMode = FillMode::WIREFRAME;
		debugRenderSphere.startColor = startColor;
		debugRenderSphere.endColor = endColor;
		debugRenderSphere.renderMode = mode;
		debugRenderSphere.watch.Start(m_clock, duration);
		m_debugRenderObjects.push_back(debugRenderSphere);
	}

	void AddWorldSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		DebugRenderObject debugRenderSphere;
		std::vector<Vertex_PCU> verts;
		AddVertsForSphere3D(verts, radius, Vec3(), AABB2::ZERO_TO_ONE, startColor);
		TransformVertexArray3D((int)verts.size(), verts, 1.0f, 0.0f, center);
		debugRenderSphere.verts = verts;
		debugRenderSphere.position = center;
		debugRenderSphere.duration = duration;
		debugRenderSphere.fillMode = FillMode::SOLID;
		debugRenderSphere.startColor = startColor;
		debugRenderSphere.endColor = endColor;
		debugRenderSphere.renderMode = mode;
		debugRenderSphere.watch.Start(m_clock, duration);
		m_debugRenderObjects.push_back(debugRenderSphere);
	}

	void AddWorldWireArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& baseColor, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		UNUSED((void)endColor);
		UNUSED((void)baseColor);
		DebugRenderObject debugRenderArrow;
		std::vector<Vertex_PCU> cylinderVerts;
		std::vector<Vertex_PCU> arrowVerts;
		Vec3 worldxBasis = Vec3(1.0f, 0.0f, 0.0f);
		Vec3 worldyBasis = Vec3(0.0f, 1.0f, 0.0f);

		Vec3 kBasis = (end - start).GetNormalized();
		Vec3 iBasis, jBasis;
		float dot = DotProduct3D(kBasis, worldxBasis);
		dot = fabsf(dot);
		if (dot < 0.99f)
		{
			jBasis = CrossProduct3D(kBasis, worldxBasis).GetNormalized();
			iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
		}
		else
		{
			jBasis = CrossProduct3D(kBasis, worldyBasis).GetNormalized();
			iBasis = CrossProduct3D(jBasis, kBasis).GetNormalized();
		}
		float height = (end - start).GetLength();
		Mat44 transforMatrix = Mat44(iBasis, jBasis, kBasis, Vec3());
		AddVertsForZCylinder(cylinderVerts, height, radius, AABB2::ZERO_TO_ONE, startColor);
		AddVertsForCone3D(arrowVerts, height, radius*2.0f, AABB2::ZERO_TO_ONE, startColor);
		TransformVertexArrayUsingMatrix3D((int)cylinderVerts.size(), cylinderVerts, transforMatrix);
		TransformVertexArrayUsingMatrix3D((int)arrowVerts.size(), arrowVerts, transforMatrix);
		TransformVertexArray3D((int)cylinderVerts.size(), cylinderVerts, 1.0f, 0.0f, start);
		TransformVertexArray3D((int)arrowVerts.size(), arrowVerts, 1.0f, 0.0f, end);
		
		for (int i = 0; i < arrowVerts.size(); i++)
		{
			cylinderVerts.push_back(arrowVerts[i]);
		}
		debugRenderArrow.verts = cylinderVerts;
		debugRenderArrow.duration = duration;
		debugRenderArrow.fillMode = FillMode::SOLID;
		debugRenderArrow.startColor = startColor;
		debugRenderArrow.endColor = endColor;
		debugRenderArrow.renderMode = mode;
		debugRenderArrow.watch.Start(m_clock, duration);
		m_debugRenderObjects.push_back(debugRenderArrow);
		cylinderVerts.clear();
		arrowVerts.clear();
	}

	void AddWorldBox(const AABB3& bounds, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		DebugRenderObject debugRenderSphere;
		std::vector<Vertex_PCU> verts;
		debugRenderSphere.position = bounds.GetCenterOfAABB3();
		AddVertsForAABB3D(verts,bounds,startColor);
		TransformVertexArray3D((int)verts.size(), verts, 1.0f, 0.0f, debugRenderSphere.position);
		debugRenderSphere.verts = verts;

		debugRenderSphere.duration = duration;
		debugRenderSphere.fillMode = FillMode::WIREFRAME;
		debugRenderSphere.startColor = startColor;
		debugRenderSphere.endColor = endColor;
		debugRenderSphere.renderMode = mode;

		m_debugRenderObjects.push_back(debugRenderSphere);
	}

	void AddWorldBasis(const Mat44& basis, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		UNUSED((void)startColor);
		UNUSED((void)endColor);
		UNUSED((void)mode);
		UNUSED((void)duration);
		Vec3 translation = basis.GetTranslation3D();
		Vec3 IBasis = translation + basis.GetIBasis3D() * 0.3f;
		Vec3 JBasis = translation + basis.GetJBasis3D() * 0.3f;
		Vec3 KBasis = translation + basis.GetKBasis3D() * 0.3f;
	
		DebugAddWorldArrow(translation, IBasis, 0.03f, -1.0f, Rgba8::RED, Rgba8::RED, Rgba8::RED, DebugRenderMode::USEDEPTH);
		DebugAddWorldArrow(translation, JBasis, 0.03f, -1.0f, Rgba8::GREEN, Rgba8::GREEN, Rgba8::RED, DebugRenderMode::USEDEPTH);
		DebugAddWorldArrow(translation, KBasis, 0.03f, -1.0f, Rgba8::BLUE, Rgba8::BLUE, Rgba8::RED, DebugRenderMode::USEDEPTH);
	}

	void AddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		BitmapFont* font = m_debugRenderConfig.m_bitmapFont;
		DebugRendererText line;
		line.text = text;
		line.alignment = alignment;
		line.duration = duration;
		line.startColor = startColor;
		line.endColor = endColor;
		line.renderMode = mode;
		line.textHeight = textHeight;
		line.modelMatrix = transform;
		line.watch.Start(m_clock, duration);
		font->AddVertsForText2D(line.verts, line.position, line.textHeight, line.text, line.startColor,0.6f,1.0f);
		m_worldTexts.push_back(line);
		font = nullptr;
	}

	void AddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
		BitmapFont* font = m_debugRenderConfig.m_bitmapFont;
		DebugRendererText line;
		line.text = text;
		line.position3D = origin;
		line.alignment = alignment;
		line.duration = duration;
		line.startColor = startColor;
		line.endColor = endColor;
		line.renderMode = mode;
		line.textHeight = textHeight;
		line.modelMatrix = Mat44::CreateTranslation3D(origin);
		line.watch.Start(m_clock, duration);
		font->AddVertsForText2D(line.verts, line.position, line.textHeight, line.text, line.color, 0.6f,1.f);
		m_billBoardTexts.push_back(line); 
		font = nullptr;
	}

	void AddScreenText(const std::string& text, const Vec2& position, float duration, const Vec2& alignment, float size, const Rgba8& startColor, const Rgba8& endColor)
	{
		UNUSED((void)size);
		DebugRendererText line;
		line.text = text;
		line.position = position;
		line.alignment = alignment;
		line.startColor = startColor;
		line.duration = duration;
		line.endColor = endColor;
		m_debugRendererTexts.push_back(line);
	}

	void AddMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor)
	{
		DebugRendererText line;
		line.text = text;
		line.startColor = startColor;
		line.endColor = endColor;
		line.watch.Start(m_clock, duration);
		line.duration = duration;
		line.color = startColor;
		m_debugScreenMessages.push_back(line);
	}
	void SetTimeDilation(float deltaValue)
	{
		deltaValue = Clamp(deltaValue, 0.1f, 10.0f);
		m_clock->SetTimeDilation(deltaValue);
	}
	Clock* GetClock()
	{
		return m_clock;
	}


	bool ToggleVisibility()
	{
		if (m_visibility == Visibility::Visible)
		{
			m_visibility = Visibility::Hidden;
		}
		else
		{
			m_visibility = Visibility::Visible;
		}
		return false;
	}
	float							m_debugRendererTextHeight = 13.0f;
	DebugRenderConfig				m_debugRenderConfig;
	Clock*							m_clock;
	Visibility						m_visibility;
	Camera							m_currentRenderCamera;
	std::vector<DebugRendererText>	m_debugRendererTexts;
	std::vector<DebugRenderObject>	m_debugRenderObjects;
	std::vector<DebugRendererText>	m_debugScreenMessages;
	std::vector<DebugRendererText>	m_billBoardTexts;
	std::vector<DebugRendererText>	m_worldTexts;
	float							m_debugScreenMessagesHeight = 13.0f;
	Camera							m_camera;
};

DebugRenderEntity* m_debugRenderEntity = nullptr;

void DebugRenderStartup(const DebugRenderConfig& config)
{
	m_debugRenderEntity = new DebugRenderEntity(config);
	m_debugRenderEntity->Startup();
	g_theEventSystem->SubscribeEventCallbackFunction("debugrendertoggle", DebugRenderToggle);
	g_theEventSystem->SubscribeEventCallbackFunction("showdebugrender", DebugRenderToggle);
	g_theEventSystem->SubscribeEventCallbackFunction("hidedebugrender", DebugRenderToggle);
}

void DebugRenderBeginFrame()
{
	m_debugRenderEntity->BeginFrame();
}

void DebugRenderEndFrame()
{
	m_debugRenderEntity->EndFrame();
}

void DebugRenderShutDown()
{
	m_debugRenderEntity->ShutDown();
}

void DebugRenderSetVisible()
{
	m_debugRenderEntity->SetVisible();
}

void DebugRenderSetHidden()
{
	m_debugRenderEntity->SetHidden();
}

void DebugRenderClear()
{
	m_debugRenderEntity->RenderClear();
}

void DebugRenderSetParentClock(Clock& parent)
{
	m_debugRenderEntity->SetParentClock(parent);
}

DebugRenderConfig GetDebugRenderConfig()
{
	return m_debugRenderEntity->m_debugRenderConfig;
}

Visibility GetDebugRendererVisibility()
{
	return m_debugRenderEntity->m_visibility;
}

void SetDebugRendererCamera(Camera const& camera)
{
	m_debugRenderEntity->SetCamera(camera);
}

void DebugRenderScreen(Camera const& camera)
{
	m_debugRenderEntity->RenderScreen(camera);
}

void DebugRenderWorld(Camera const& camera)
{
	m_debugRenderEntity->RenderWorld(camera);
}

void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldPoint(pos, radius, duration, startColor, endColor, mode);
}

void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldLine(start, end, radius, duration, startColor, endColor, mode);
}

void DebugAddWorldPlayerLine(const Mat44& basis, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldPlayerLine(basis, duration,startColor, endColor, mode);
}
void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top,const Vec3& position, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldWireCylinder(base, top, position, radius, duration, startColor, endColor, mode);
}

void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldWireSphere(center, radius, duration, startColor, endColor, mode);
}
void DebugAddWorldSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldSphere(center, radius, duration, startColor, endColor, mode);
}
void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& baseColor, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldWireArrow(start, end, radius, duration, baseColor, startColor, endColor, mode);
}

void DebugAddWorldBox(const AABB3& bounds, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldBox(bounds, duration, startColor, endColor, mode);
}

void DebugAddWorldBasis(const Mat44& basis, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldBasis(basis, duration, startColor, endColor, mode);
}

void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldText(text, transform, textHeight, alignment, duration, startColor, endColor, mode);
}

void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	m_debugRenderEntity->AddWorldBillboardText(text, origin, textHeight, alignment, duration, startColor, endColor, mode);
}

void DebugAddScreenText(const std::string& text, const Vec2& position, float duration, const Vec2& alignment, float size, const Rgba8& startColor, const Rgba8& endColor)
{
	m_debugRenderEntity->AddScreenText(text, position, duration, alignment, size, startColor, endColor);
}

void DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	m_debugRenderEntity->AddMessage(text, duration, startColor, endColor);
}

bool DebugRenderToggle(EventArgs& args)
{
	UNUSED((void)args);
	m_debugRenderEntity->ToggleVisibility();
	return false;
}

Clock* DebugRenderGetClock()
{
	return m_debugRenderEntity->GetClock();
}

void DebugRenderClockSetTimeDilation(float deltaValue)
{
	m_debugRenderEntity->SetTimeDilation(deltaValue);
}

bool DebugRenderHide(EventArgs& args)
{
	UNUSED((void)args);
	m_debugRenderEntity->SetHidden();
	return false;
}

bool DebugRenderShow(EventArgs& args)
{
	UNUSED((void)args);
	m_debugRenderEntity->SetVisible();
	return false;
}


