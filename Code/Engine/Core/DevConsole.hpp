#pragma once
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EventSytem.hpp"
#include <string>
#include <vector>
#include <mutex>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Renderer;
class Camera;
class BitmapFont;
class Stopwatch;
struct AABB2;
class DevConsole;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
extern DevConsole* g_theDevConsole;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct DevConsoleLine
{
	Rgba8 m_color;
	std::string m_text;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct DevConsoleConfig
{
	Renderer* m_renderer = nullptr;
	Camera* m_camera = nullptr;
	std::string m_fontName = "SquirrelFixedFont";
	float m_fontAspect = 0.8f;
	int m_linesOnScreen = 40;
	int m_maxCommandHistory = 128;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//class for a dev console that allows entering text and executing commands. Can be toggled with
//tilde (~) and renders within a transparent box with configurable bounds. Other features
//include specific coloring for a different lines of text and a blinking caret.
class DevConsole
{
public:
	DevConsole(DevConsoleConfig const& config);
	~DevConsole();

	//Subscribe to any events needed, prints an initial line of text, and starts the caret timer.
	void				Startup();
	void				Shutdown();
	void				BeginFrame();
	void				EndFrame();

	//Parses the current input line and executes it using the event system. Commands and arguments
	//are delimited from each other with space (' ') and argument names and values are delimited with
	//equals ('='). Echoes the command to the dev console as well as any command input.
	void				Execute(std::string const& consoleCommandText);

	//Adds a line of text to the current list of lines being shown. Individual lines are delimited
	//with the newline ('\n') character.
	void				AddLine(Rgba8 const& color, std::string const& text);

	//Renders just visible text lines within the bounds specified. Bounds are in terms of the camera
	//being used to render. the current input renders at the bottom with all the other lines rendered
	//above it, with the most recent lines at the bottom.
	void				Render(AABB2 const& bounds);

	//Toggles between open and closed.
	void				ToggleOpen();

	//Handle key input.
	static bool			Event_KeyPressed(EventArgs& args);

	//handle char input by appending valid characters to our current input line.
	static bool			Event_CharInput(EventArgs& args);

	//Clear all lines of text.
	static bool			Command_Clear(EventArgs& args);

	//Display all currently registered commands in the event system.
	static bool			Command_Help(EventArgs& args);

	void				SetToClipboard(const std::string& input);
	std::string			GetFromClipboard();
	std::string const	GetPreviousCommand();
	std::string const	GetNextCommand();
	void				ResetCaret();
	void				SetCaretPosition(int position);
	void				AddToCommandHistory(std::string input);
	void				RemoveCharacterOnLine(int direction);
	void				ClearInput();
	void				ExecuteXmlCommandScriptNode(tinyxml2::XMLElement const& commandScriptXmlElement);
	void				ExecuteXmlCommandScriptFile(std::string& filePath);


	DevConsoleConfig			m_config;

	//True if the dev console is currently visible and accepting input.
	bool						m_isOpen = false;

	//All lines added to the dev console since the last time it was cleared.
	std::vector<DevConsoleLine> m_lines;

	//Our current line of input text.
	std::string					m_inputLine;

	//Index of the caret is currently in the visible phase of blinking.
	int							m_caretPosition = 0;

	//True if our caret is currently in the visible phase of blinking.
	bool						m_caretVisible = false;
	
	//Stopwatch for controlling caret visibility.
	Stopwatch*					m_caretStopwatch = nullptr;

	//History of all commands executed.
	std::vector<std::string>	m_commandHistory;

	//Our current index in our history of commands as we are scrolling.
	int							m_historyIndex = -1;

	int							m_commandHistorySize = 0;

	static const Rgba8 ERROR_COLOR;
	static const Rgba8 WARNING;
	static const Rgba8 INFO_MAJOR;
	static const Rgba8 INFO_MINOR;
	static const Rgba8 COMMAND_ECHO;
	static const Rgba8 INPUT_TEXT;
	static const Rgba8 INPUT_CARET;

};
//--------------------------------------------------------------------------------------------------------------------------------------------------------