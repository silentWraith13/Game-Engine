#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EventSytem.hpp"
#include <string>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Renderer;
struct Mat44;
struct Vec3;
struct Vec2;
class Camera;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class DebugRenderMode
{
	ALWAYS,
	USE_DEPTH,
	X_RAY,
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct DebugRenderConfig
{
	Renderer* m_renderer = nullptr;
	bool m_startHidden = false;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------

//Setup
void DebugRenderSystemStartup(const DebugRenderConfig& config);
void DebugRenderSystemShutdown();
 
//Control
void DebugRenderSetVisible();
void DebugRenderSetHidden();
void DebugRenderClear();
 
//Output
void DebugRenderBeginFrame();
void DebugRenderWorld(const Camera& camera);
void DebugRenderScreen(const Camera& camera);
void DebugRenderEndFrame();
 
//Geometry
void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor = Rgba8::WHITE, 
	const Rgba8& endColor = Rgba8::WHITE, DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor = Rgba8::WHITE, 
	const Rgba8& endColor = Rgba8::WHITE, DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor = Rgba8::WHITE, 
	const Rgba8& endColor = Rgba8::WHITE, DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE, DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor = Rgba8::WHITE,
	const Rgba8& endColor = Rgba8::WHITE, DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration,
	const Rgba8& startColor = Rgba8::WHITE, const Rgba8& endColor = Rgba8::WHITE, DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration,
	const Rgba8& startColor = Rgba8::WHITE, const Rgba8& endColor = Rgba8::WHITE, DebugRenderMode mode = DebugRenderMode::USE_DEPTH);
void DebugAddScreenText(const std::string& text, const Vec2& position, float size, const Vec2& alignment, float duration,
	const Rgba8& startColor = Rgba8::WHITE, const Rgba8& endColor = Rgba8::WHITE);
void DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor = Rgba8::WHITE, const Rgba8& endColor = Rgba8::WHITE);
 
//ConsoleCommands
bool Command_DebugRenderClear(EventArgs& args);
bool Command_DebugRenderToggle(EventArgs& args);
//--------------------------------------------------------------------------------------------------------------------------------------------------------