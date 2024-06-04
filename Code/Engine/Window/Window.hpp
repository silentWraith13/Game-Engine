#pragma once
#include <string>
#include "Engine/Math/IntVec2.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class InputSystem;
struct IntVec2;
class EventSystem;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum WindowMode
{
	DESKSTOP_BASED,
	FULLSCREEN,
	WINDOWED,
	NUM_MODES
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct WindowConfig
{
	float m_clientAspect = 2.f;
	InputSystem* m_inputSystem = nullptr;
    std::string   m_windowTitle = "Unnamed Application";
	IntVec2 m_windowSize = IntVec2(-1,-1);
	IntVec2 m_windowPosition = IntVec2(-1, -1);
	WindowMode m_windowMode = FULLSCREEN;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Window
{
public:	
	Window(WindowConfig const& config);
	~Window();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void CreateOSWindow();
	void RunMessagePump();
	void* GetHwnd() const;
	IntVec2 GetClientDimensions() const;
	static Window* GetWindow()            { return s_mainWindow; }
	WindowConfig const& GetConfig() const { return m_config; }
	WindowConfig        m_config;
	static Window*      s_mainWindow;
	
	void* m_hwnd = nullptr;
	IntVec2 m_clientDimensions;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------