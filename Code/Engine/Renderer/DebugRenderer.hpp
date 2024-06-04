#pragma once
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/AABB3.hpp"

enum class DebugRenderMode
{
	ALWAYS,
	USEDEPTH,
	XRAY,
};

//------------------------------------------------------------------------
struct DebugRenderConfig
{
	Renderer*   m_renderer = nullptr;
	bool		m_startHidden = false;
	BitmapFont* m_bitmapFont= nullptr;
};


void							DebugRenderStartup(const DebugRenderConfig& config);
void							DebugRenderBeginFrame();
void							DebugRenderEndFrame();
void							DebugRenderShutDown();

//-----------------------------Control--------------------------------------------------
void							DebugRenderSetVisible();
void							DebugRenderSetHidden();
void							DebugRenderClear();
void							DebugRenderSetParentClock(Clock& parent);
//------------------------------Output--------------------------------------------------
void							DebugRenderScreen(Camera const& camera);
void							DebugRenderWorld(Camera const& camera);
//----------------------------- Geometry-----------------------------------------------------
void							DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddWorldPlayerLine(const Mat44& basis, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top,const Vec3& position, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddWorldSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& baseColor, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddWorldBox(const AABB3& bounds, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddWorldBasis(const Mat44& basis, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void							DebugAddScreenText(const std::string& text, const Vec2& position, float duration, const Vec2& alignment, float size, const Rgba8& startColor, const Rgba8& endColor);
void							DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor);
static	bool					DebugRenderToggle(EventArgs& args);
Clock*							DebugRenderGetClock();
void							DebugRenderClockSetTimeDilation(float deltaValue);
