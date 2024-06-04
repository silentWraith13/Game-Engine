#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Engine/Window/Window.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "ThirdParty/ImGui/imgui_impl_win32.h"
#include "Engine/Core/NamedProperties.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Window*            Window::s_mainWindow = nullptr;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(windowHandle, wmMessageCode, wParam, lParam))
	{
		return true;
	}

	//Window class should have a pointer to inputSystem
	Window* window = Window::GetWindow();
	GUARANTEE_OR_DIE(window != nullptr, "window::GetWindow() returned null!");
	InputSystem* input = window->GetConfig().m_inputSystem;
	GUARANTEE_OR_DIE(input !=nullptr, "Window::GetConfig().m_inputSystem was NULL!")

	switch (wmMessageCode)
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			EventArgs args;
			FireEvent("Quit", args);
			return 0;
		}

		case WM_KEYDOWN:
		{
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
			FireEvent("KeyPressed", args);
			return 0;
		}
		
		case WM_LBUTTONUP:
		{
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", KEYCODE_LEFT_MOUSE));
			FireEvent("KeyReleased", args);
			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", KEYCODE_LEFT_MOUSE));
			FireEvent("KeyPressed", args);
			return 0;
		}
		case WM_RBUTTONUP:
		{
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", KEYCODE_RIGHT_MOUSE));
			FireEvent("KeyReleased", args);
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", KEYCODE_RIGHT_MOUSE));
			FireEvent("KeyPressed", args);
			return 0;
		}

		case WM_KEYUP:
		{
			EventArgs args;
			args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
			FireEvent("KeyReleased", args);
			return 0;
		}	
		
		case WM_CHAR:
		{
			EventArgs args;
			args.SetValue("Character", Stringf("%d", (unsigned char)wParam));
			FireEvent("CharInput", args);
			return 0;
		}

		case WM_SIZE:
		{
			int newWidth = LOWORD(lParam);
			int newHeight = HIWORD(lParam);
			g_theWindow->m_clientDimensions = IntVec2(newWidth, newHeight);
			g_theWindow->m_config.m_clientAspect = (float)newWidth / (float)newHeight;
			return 0;
		}
	}			

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Window::Window(WindowConfig const& config)
	:m_config(config)
{
	s_mainWindow = this;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Window::~Window()
{

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Window::Startup()
{
	CreateOSWindow();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Window::Shutdown()
{

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Window::BeginFrame()
{
	RunMessagePump();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Window::EndFrame()
{

}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Window::CreateOSWindow()
{
	float clientAspect = m_config.m_clientAspect;
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_OVERLAPPED;
	DWORD windowStyleExFlags = WS_EX_APPWINDOW;
	RECT clientRect;
	
	RECT windowRect;
	windowRect.left = 0;
	windowRect.top = 0;
	windowRect.right = 800; 
	windowRect.bottom = 600; 
	
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 1.f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	
	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	switch (m_config.m_windowMode)
	{
	case DESKSTOP_BASED:
		if (clientAspect > desktopAspect)
		{
			clientHeight = clientWidth / clientAspect;
			clientWidth = desktopWidth * maxClientFractionOfDesktop;
		}
		else
		{
			clientWidth = clientHeight * clientAspect;
			clientHeight = desktopHeight * maxClientFractionOfDesktop;
		}

		m_clientDimensions = IntVec2((int)clientWidth, (int)clientHeight);
		clientRect.right = clientRect.left + (int)clientWidth;
		clientRect.bottom = clientRect.top + (int)clientHeight;
		AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);
		windowRect = clientRect;
		break;
	
	case FULLSCREEN:
		
		windowStyleFlags = WS_POPUP;
		m_clientDimensions = IntVec2((int)desktopWidth, (int)desktopHeight);
		m_config.m_clientAspect = (desktopWidth / desktopHeight);
		windowRect = desktopRect;

		break;
	case WINDOWED:
		m_clientDimensions = m_config.m_windowSize;
		m_config.m_clientAspect = (float)(m_config.m_windowSize.x / m_config.m_windowSize.y);
		windowRect.left = m_config.m_windowPosition.x;
		windowRect.top = m_config.m_windowPosition.y;
		windowRect.right = windowRect.left + m_clientDimensions.x;
		windowRect.bottom = windowRect.top + m_clientDimensions.y;
		AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

		break;
	default:
		break;
	}
	
	
	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	HWND hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	m_hwnd = hWnd;

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Window::RunMessagePump()
{
	MSG queuedMessage;
	for ( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void* Window::GetHwnd() const
{
	return m_hwnd;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec2 Window::GetClientDimensions() const
{
	return m_clientDimensions;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
