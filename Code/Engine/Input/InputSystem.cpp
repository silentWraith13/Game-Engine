#include "Engine/Input/InputSystem.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSytem.hpp"
#include "Engine/Core/NamedProperties.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const unsigned char KEYCODE_F1 = VK_F1;
const unsigned char KEYCODE_F2 = VK_F2;
const unsigned char KEYCODE_F3 = VK_F3;
const unsigned char KEYCODE_F4 = VK_F4;
const unsigned char KEYCODE_F5 = VK_F5;
const unsigned char KEYCODE_F6 = VK_F6;
const unsigned char KEYCODE_F7 = VK_F7;
const unsigned char KEYCODE_F8 = VK_F8;
const unsigned char KEYCODE_F9 = VK_F9;
const unsigned char KEYCODE_F10 = VK_F10;
const unsigned char KEYCODE_F11 = VK_F11;
const unsigned char KEYCODE_ESC = VK_ESCAPE;
const unsigned char KEYCODE_SPACE = VK_SPACE;
const unsigned char KEYCODE_ENTER = VK_RETURN;
const unsigned char KEYCODE_UPARROW = VK_UP;
const unsigned char KEYCODE_DOWNARROW = VK_DOWN;
const unsigned char KEYCODE_LEFTARROW = VK_LEFT;
const unsigned char KEYCODE_RIGHTARROW = VK_RIGHT;
const unsigned char KEYCODE_LEFT_CTRL = VK_CONTROL;
const unsigned char KEYCODE_TILDE = 0xC0;
const unsigned char KEYCODE_BACKSPACE = VK_BACK;
const unsigned char KEYCODE_INSERT = VK_INSERT;
const unsigned char KEYCODE_DELETE = VK_DELETE;
const unsigned char KEYCODE_HOME = VK_HOME;
const unsigned char KEYCODE_END = VK_END;
const unsigned char KEYCODE_SHIFT = VK_SHIFT;
const unsigned char KEYCODE_LEFT_MOUSE = VK_LBUTTON;
const unsigned char KEYCODE_RIGHT_MOUSE = VK_RBUTTON;
const unsigned char KEYCODE_LEFT_BRACKET = 0xDB;
const unsigned char KEYCODE_RIGHT_BRACKET = 0xDD;
const unsigned char KEYCODE_COMMA = VK_OEM_COMMA;
const unsigned char KEYCODE_PERIOD = VK_OEM_PERIOD;
const unsigned char KEYCODE_SEMICOLON = VK_OEM_1;
const unsigned char KEYCODE_SINGLEQUOTE = VK_OEM_7;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
InputSystem::InputSystem(InputSystemConfig const& config) : m_config(config)
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
InputSystem::~InputSystem()
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void InputSystem::Startup()
{
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		m_controllers[controllerIndex].m_id = controllerIndex;
	}
	
	g_theEventSystem->SubscribeEventCallbackFunction("KeyPressed", InputSystem::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("KeyReleased", InputSystem::Event_KeyReleased);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void InputSystem::Shutdown()
{

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void InputSystem::BeginFrame()
{
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		m_controllers[controllerIndex].Update();
	}

	if (m_mouseStateConfig.m_currentHidden != m_mouseStateConfig.m_desiredHidden)
	{
		m_mouseStateConfig.m_currentHidden = m_mouseStateConfig.m_desiredHidden;

		if (m_mouseStateConfig.m_currentHidden)
		{
			while (ShowCursor(FALSE) >= 0); // hide cursor
		}
		else
		{
			while (ShowCursor(TRUE) < 0); // show cursor
		}
	}

	if (m_mouseStateConfig.m_currentRelative != m_mouseStateConfig.m_desiredRelative) {
		m_mouseStateConfig.m_currentRelative = m_mouseStateConfig.m_desiredRelative;
		if (m_mouseStateConfig.m_currentRelative) {
			// Center the mouse when entering relative mode
			m_mouseStateConfig.m_cursorClientPosition = g_theWindow->GetClientDimensions() / 2;
			SetCursorPos(m_mouseStateConfig.m_cursorClientPosition.x, m_mouseStateConfig.m_cursorClientPosition.y);
		}
	}

	// Update the mouse position and calculate the delta
	POINT currentCursorPosition;
	GetCursorPos(&currentCursorPosition);
	ScreenToClient(GetActiveWindow(), &currentCursorPosition);
	IntVec2 newCursorPosition =  IntVec2(currentCursorPosition.x, currentCursorPosition.y);

	if (m_mouseStateConfig.m_currentRelative) 
	{
		// Calculate the mouse delta
		m_mouseStateConfig.m_cursorClientDelta = newCursorPosition - m_mouseStateConfig.m_cursorClientPosition;

		// Re-center the mouse cursor to avoid continuous drift
		m_mouseStateConfig.m_cursorClientPosition = g_theWindow->GetClientDimensions() / 2;
		SetCursorPos(m_mouseStateConfig.m_cursorClientPosition.x, m_mouseStateConfig.m_cursorClientPosition.y);
	}
	else {
		// In non-relative mode, update the position and set delta to zero
		m_mouseStateConfig.m_cursorClientPosition = newCursorPosition;
		m_mouseStateConfig.m_cursorClientDelta = IntVec2(0, 0);
	}


	// Store current cursor client position again
	GetCursorPos(&currentCursorPosition);
	ScreenToClient(GetActiveWindow(), &currentCursorPosition);
	m_mouseStateConfig.m_cursorClientPosition.x = currentCursorPosition.x;
	m_mouseStateConfig.m_cursorClientPosition.y = currentCursorPosition.y;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void InputSystem::EndFrame()
{
	for (int keyIndex = 0; keyIndex < NUM_KEYCODES; keyIndex++)
	{
		m_keyStates[keyIndex].UpdateKeyDownLastFrame();
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
	return m_keyStates[keyCode].WasKeyJustPressed();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool InputSystem::WasKeyJustReleased(unsigned char keyCode)
{
	return m_keyStates[keyCode].WasKeyJustReleased();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	return m_keyStates[keyCode].IsKeyDown();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool InputSystem::IsCtrlHeld() 
{
	return IsKeyDown(KEYCODE_LEFT_CTRL);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool InputSystem::Event_KeyPressed(EventArgs& args)
{
	if (!g_theInput)
	{
		return false;
	}
	std::string keyCodeStr = args.GetValue("KeyCode", std::string("-1"));
	unsigned char keyCode = static_cast<unsigned char>(std::stoi(keyCodeStr));
	g_theInput->HandleKeyPressed(keyCode);
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool InputSystem::Event_KeyReleased(EventArgs& args)
{
	if (!g_theInput)
	{
		return false;
	}
	std::string keyCodeStr = args.GetValue("KeyCode", std::string("-1"));
	unsigned char keyCode = static_cast<unsigned char>(std::stoi(keyCodeStr));
	g_theInput->HandleKeyReleased(keyCode);
	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].m_keyDownThisFrame = true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].m_keyDownThisFrame = false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
XboxController const& InputSystem::GetController(int controllerID)
{
	return m_controllers[controllerID];
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
InputSystemConfig const& InputSystem::GetConfig() const
{
	return m_config;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void InputSystem::SetCursorMode(bool hidden, bool relative)
{
	m_mouseStateConfig.m_desiredHidden = hidden;
	m_mouseStateConfig.m_desiredRelative = relative;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 InputSystem::GetCursorClientDelta() const
{
	return Vec2((float)m_mouseStateConfig.m_cursorClientDelta.x, (float)m_mouseStateConfig.m_cursorClientDelta.y);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 InputSystem::GetCursorClientPosition() const
{
	return Vec2((float)m_mouseStateConfig.m_cursorClientPosition.x, (float)m_mouseStateConfig.m_cursorClientPosition.y);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 InputSystem::GetCursorNormalizedPosition() const
{
	RECT clientRect;
	HWND windowHandle = GetActiveWindow();
	POINT cursorCoords;

	::GetCursorPos(&cursorCoords);
	::ScreenToClient(windowHandle, &cursorCoords);
	::GetClientRect(windowHandle, &clientRect);
	float cursorX = float(cursorCoords.x) / float(clientRect.right);
	float cursorY = float(cursorCoords.y) / float(clientRect.bottom);
	return Vec2(cursorX, 1.f - cursorY);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------