#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/vertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#define NOMINMAX
#include <windows.h>

 //--------------------------------------------------------------------------------------------------------------------------------------------------------
DevConsole* g_theDevConsole = nullptr;
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
const Rgba8 DevConsole::ERROR_COLOR = Rgba8(255, 0, 0);
const Rgba8 DevConsole::WARNING = Rgba8(255, 255, 0);
const Rgba8 DevConsole::INFO_MAJOR = Rgba8(0, 255, 0);
const Rgba8 DevConsole::INFO_MINOR = Rgba8(0, 128, 128);
const Rgba8 DevConsole::COMMAND_ECHO = Rgba8(255, 128, 0);
const Rgba8 DevConsole::INPUT_TEXT = Rgba8(255, 0, 255);
const Rgba8 DevConsole::INPUT_CARET = Rgba8(255, 255, 255);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
DevConsole::DevConsole(DevConsoleConfig const& config)
	:m_config(config)
{
	m_caretStopwatch = new Stopwatch(0.5f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
DevConsole::~DevConsole()
{
	delete m_caretStopwatch;
	m_caretStopwatch = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DevConsole::Startup()
{
	//Subscribe to all events.
	g_theEventSystem->SubscribeEventCallbackFunction("KeyPressed", DevConsole::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("CharInput",  DevConsole::Event_CharInput);
	g_theEventSystem->SubscribeEventCallbackFunction("Clear",	   DevConsole::Command_Clear);
	g_theEventSystem->SubscribeEventCallbackFunction("Help",       DevConsole::Command_Help);

	AddLine(INFO_MINOR, "Type Help for a list of commands");
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DevConsole::Shutdown()
{
	// Unsubscribe from all events
	g_theEventSystem->UnsubscribeEventCallbackFunction("KeyPressed", DevConsole::Event_KeyPressed);
 	g_theEventSystem->UnsubscribeEventCallbackFunction("CharInput",	 DevConsole::Event_CharInput);
 	g_theEventSystem->UnsubscribeEventCallbackFunction("Clear",      DevConsole::Command_Clear);
 	g_theEventSystem->UnsubscribeEventCallbackFunction("Help",       DevConsole::Command_Help);

	m_caretStopwatch->Stop();

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DevConsole::BeginFrame()
{
	if (m_isOpen)
	{
		if (m_caretStopwatch->DecrementDurationIfElapsed())
		{
			m_caretVisible = !m_caretVisible;
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DevConsole::EndFrame()
{
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//Parses the current input line and executes it using the event system. Commands and arguments
//are delimited from each other with space (' ') and argument names and values are delimited with
//equals ('='). Echoes the command to the dev console as well as any command input.
void DevConsole::Execute(std::string const& consoleCommandText)
{
	EventArgs args;
	std::string command;
	
	//The function first splits the given console command text into separate parts using the "SplitStringOnDelimiter" function.
	auto parts = SplitStringWithDelimeterWithQuotes(consoleCommandText, ' ');			//auto =  std::vector<std::string>.
	
																			//If the number of parts is one, it sets the command as the only part.
	if (parts.size() == 1)
	{
		command = parts[0];
	}
	else
	{
		//If there are multiple parts, it sets the first part as the command, and processes the rest of the parts as arguments.
		command = parts[0];
		for (unsigned int i = 1; i < parts.size(); i++)
		{
			// Split the argument into name and value using the "SplitStringOnDelimiter" function with the delimiter '='.
			auto argParts = SplitStringWithDelimeterWithQuotes(parts[i], '=');
			if (argParts.size() == 2)
			{
				// If the argument has a name and value, set the argument in the EventArgs struct.
				args.SetValue(argParts[0], argParts[1]);
			}
		}
	}
	// Echo the command to the dev console and then the executed command is echoed to the dev console by creating a "DevConsoleLine" object and adding it to the "m_lines" vector.
	DevConsoleLine line;
	line.m_text = consoleCommandText;
	line.m_color = DevConsole::INPUT_TEXT;

	if (command != "Echo")
	{
		m_lines.push_back(line);
	}
	
	FireEvent(command, args);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DevConsole::AddLine(Rgba8 const& color, std::string const& text)
{
	DevConsoleLine line;
	line.m_color = color;
	line.m_text = text;
	m_lines.push_back(line);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DevConsole::Render(AABB2 const& bounds)
{
	if (!m_isOpen)
		return;

	//Begin camera
	m_config.m_renderer->BeginCamera(*m_config.m_camera);

	//Render a transparent background4
	std::vector<Vertex_PCU> backGroundVerts;
	AddVertsForAABB2D(backGroundVerts, bounds, Rgba8(80, 80, 80, 150));
	m_config.m_renderer->BindTexture(nullptr);
	m_config.m_renderer->DrawVertexArray((int)backGroundVerts.size(), backGroundVerts.data());

	//Use the font in the config for all text rendering.
	BitmapFont* textFont = m_config.m_renderer->CreateOrGetBitmapFontFromFile(("Data/Fonts/" + m_config.m_fontName).c_str());

	//Calculate line height and number of visible lines.
	float lineHeight = bounds.GetDimensions().y / static_cast<float>(m_config.m_linesOnScreen);
	int numLinesToRender = std::min(static_cast<int>(m_lines.size()), m_config.m_linesOnScreen);

	//Only render visible lines. Do not render any that are not visible on screen.
	std::vector<Vertex_PCU> visibleLinesVerts;
	for (int i = 0; i < numLinesToRender; i++)
	{
		DevConsoleLine line = m_lines[m_lines.size() - 1 - i];
		AABB2 lineBounds = AABB2(bounds.m_mins + Vec2(0.f, i * lineHeight + lineHeight), Vec2(bounds.m_maxs.x, bounds.m_mins.y + (i + 1) * lineHeight + lineHeight));
		textFont->AddVertsForTextInBox2D(visibleLinesVerts, lineBounds, lineHeight, line.m_text, line.m_color, m_config.m_fontAspect, Vec2(0.f, 1.f), TextDrawMode::SHRINK_TO_FIT);
	}
	m_config.m_renderer->BindTexture(&textFont->GetTexture());
	m_config.m_renderer->DrawVertexArray((int)visibleLinesVerts.size(), visibleLinesVerts.data());
	m_config.m_renderer->BindTexture(nullptr);

	//Render the current input line in a distinctive color.
	AABB2 inputLineBounds = AABB2(bounds.m_mins, Vec2(bounds.m_maxs.x, bounds.m_mins.y + lineHeight));
	std::vector<Vertex_PCU> inputLineVerts;
	textFont->AddVertsForTextInBox2D(inputLineVerts, inputLineBounds, lineHeight, m_inputLine, DevConsole::INPUT_TEXT, m_config.m_fontAspect, Vec2(0.f, 1.f), TextDrawMode::SHRINK_TO_FIT);
	m_config.m_renderer->BindTexture(&textFont->GetTexture());
	m_config.m_renderer->DrawVertexArray((int)inputLineVerts.size(), inputLineVerts.data());
	m_config.m_renderer->BindTexture(nullptr);

	//Use the stopwatch to determine if the blinking caret is currently visible and if so render it.
	if (m_caretVisible)
	{
		std::string inputTextUpToCaret = m_inputLine.substr(0, m_caretPosition);
		float inputTextWidth = textFont->GetTextWidth(lineHeight, inputTextUpToCaret.c_str(), m_config.m_fontAspect);

		float caretWidth = lineHeight * 0.1f;

		// Position the caret right after the input text up to the caret position
		Vec2 caretPos = inputLineBounds.m_mins + Vec2(inputTextWidth, 0.f);
		AABB2 caretLineBounds = AABB2(caretPos, caretPos + Vec2(caretWidth, lineHeight));
		std::vector<Vertex_PCU> caretLineVerts;

		AddVertsForAABB2D(caretLineVerts, caretLineBounds, Rgba8(255, 255, 255));
		m_config.m_renderer->BindTexture(nullptr);
		m_config.m_renderer->DrawVertexArray((int)caretLineVerts.size(), caretLineVerts.data());
	}

	//End camera
	m_config.m_renderer->EndCamera(*m_config.m_camera);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void DevConsole::ToggleOpen()
{
    m_isOpen = !m_isOpen;
	m_caretStopwatch->Start();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool DevConsole::Event_KeyPressed(EventArgs& args)
{
	std::string keyCodeStr = args.GetValue("KeyCode", std::string("-1"));

	unsigned int keyCode = (unsigned int)std::stoi(keyCodeStr);

	if (keyCode == KEYCODE_TILDE)
	{
		g_theDevConsole->ToggleOpen();
		return  true;
	}
	 
	if (g_theDevConsole->m_isOpen)
	{
		g_theDevConsole->m_caretStopwatch->Restart();

		if (keyCode == KEYCODE_ESC)
		{
			if (g_theDevConsole->m_inputLine.empty())
			{
				g_theDevConsole->m_isOpen = false;
			}
			else
			{
				g_theDevConsole->ClearInput();
			}
		}

		if (keyCode == KEYCODE_ENTER)
		{
			if (!g_theDevConsole->m_inputLine.empty())
			{
				g_theDevConsole->Execute(g_theDevConsole->m_inputLine);
				g_theDevConsole->AddToCommandHistory(g_theDevConsole->m_inputLine);
				g_theDevConsole->ClearInput();
				g_theDevConsole->m_caretPosition = 0;
			}
			else
			{
				g_theDevConsole->m_isOpen = false;
			}
			
		}
		
		if (keyCode == KEYCODE_UPARROW)
		{
			g_theDevConsole->m_inputLine = g_theDevConsole->GetPreviousCommand();
			g_theDevConsole->SetCaretPosition((int)(g_theDevConsole->m_inputLine).size());
		}
		
		if (keyCode == KEYCODE_DOWNARROW)
		{
			g_theDevConsole->m_inputLine = g_theDevConsole->GetNextCommand();
			g_theDevConsole->SetCaretPosition((int)(g_theDevConsole->m_inputLine).size());
		}

		if (keyCode == KEYCODE_LEFTARROW)
		{
			g_theDevConsole->SetCaretPosition(g_theDevConsole->m_caretPosition - 1);
		}

		if (keyCode == KEYCODE_RIGHTARROW)
		{
			g_theDevConsole->SetCaretPosition(g_theDevConsole->m_caretPosition + 1);
		}

		if (keyCode == KEYCODE_HOME)
		{
			g_theDevConsole->SetCaretPosition(0);
		}

		if (keyCode == KEYCODE_END)
		{
			g_theDevConsole->SetCaretPosition((int)(g_theDevConsole->m_inputLine).size());
		}

		if (keyCode == KEYCODE_DELETE)
		{
			g_theDevConsole->RemoveCharacterOnLine(1);
		}

		if (keyCode == KEYCODE_BACKSPACE)
		{
			g_theDevConsole->RemoveCharacterOnLine(-1);
		}

		if (g_theInput->IsCtrlHeld())
		{
			if (g_theInput->WasKeyJustPressed('C'))
			{
				// Copy current input to clipboard
				g_theDevConsole->SetToClipboard(g_theDevConsole->m_inputLine);
				return true; // Consumes the CTRL+C event
			}
			else if (g_theInput->WasKeyJustPressed('V'))
			{
				// Paste from clipboard to current input
				g_theDevConsole->m_inputLine += g_theDevConsole->GetFromClipboard();
				g_theDevConsole->m_caretPosition = (int)g_theDevConsole->m_inputLine.length(); // Set the caret at the end of the input text
				return true; // Consumes the CTRL+V event
			}
		}


		return true;
	}
	else // !m_isOpen 
	{
		return false; // dev console is not open, do not consume any keypress event
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool DevConsole::Event_CharInput(EventArgs& args)
{
	std::string keyCodeStr = args.GetValue("KeyCode", std::string("-1"));

	int charCode = std::stoi(keyCodeStr);
	if (g_theDevConsole->m_isOpen)
	{
		if (charCode >= 32 && charCode <= 126 && charCode != '~' && charCode != '`')
		{
			g_theDevConsole->m_inputLine.insert(g_theDevConsole->m_caretPosition, 1, (char)charCode);
			g_theDevConsole->m_caretPosition++;
		}
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool DevConsole::Command_Clear(EventArgs& args)
{
	(void)args;
	g_theDevConsole->m_lines.clear(); 
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool DevConsole::Command_Help(EventArgs& args)
{
	(void)args;
	g_theEventSystem->GetNamesOfAllEvents(g_theDevConsole->m_commandHistory);
			
	g_theDevConsole->AddLine(INFO_MAJOR, "List of all registered commands");
	
	if (g_theDevConsole->m_commandHistory.empty())
	{
		DevConsoleLine line;
		line.m_color = DevConsole::COMMAND_ECHO;
		line.m_text = "No commands are registered.";
		g_theDevConsole->AddLine(line.m_color, line.m_text);
	}
	else
	{
		for (unsigned int i = 0; i < g_theDevConsole->m_commandHistory.size(); i++)
		{
			DevConsoleLine line;
			line.m_color = DevConsole::COMMAND_ECHO;
			line.m_text = g_theDevConsole->m_commandHistory[i];
			g_theDevConsole->m_lines.push_back(line);
		}
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 bool DevConsole::Command_LoadModel(EventArgs& args)
 {
	 std::string path = args.GetValue("File", "INVALID_PATH");
	 if (path == "INVALID_PATH") 
	 {
		 g_theDevConsole->AddLine(ERROR_COLOR, "Invalid path provided for model loading.");
		 return false;
	 }

	 g_theEventSystem->FireEvent("OnModelLoadCommand", args);
	 return true;
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void DevConsole::SetToClipboard(const std::string& input)
 {
	 if (OpenClipboard(NULL))
	 {
		 EmptyClipboard();

		 HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, input.size() + 1);
		 if (!hGlob)
		 {
			 CloseClipboard();
			 return;
		 }

		 memcpy(GlobalLock(hGlob), input.c_str(), input.size() + 1);
		 GlobalUnlock(hGlob);
		 SetClipboardData(CF_TEXT, hGlob);

		 CloseClipboard();
		 GlobalFree(hGlob);
	 }
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 std::string DevConsole::GetFromClipboard()
 {
	 std::string clipboardContent;

	 if (OpenClipboard(NULL))
	 {
		 HANDLE hClip = GetClipboardData(CF_TEXT);
		 if (hClip)
		 {
			 char* clipboardData = static_cast<char*>(GlobalLock(hClip));
			 if (clipboardData)
			 {
				 clipboardContent = clipboardData;
				 GlobalUnlock(hClip);
			 }
		 }
		 CloseClipboard();
	 }

	 return clipboardContent;
 }

//--------------------------------------------------------------------------------------------------------------------------------------------------------
   
 void DevConsole::AddToCommandHistory(std::string input)
 {
	 m_commandHistory.insert(m_commandHistory.begin(), input);

	 if ((int)m_commandHistory.size() > m_config.m_maxCommandHistory)
	 {
		 m_commandHistory.pop_back();
	 }
 }


 void DevConsole::SetCaretPosition(int position)
 {
	 m_caretPosition = position;

	 if (m_caretPosition < 0)
	 {
		 m_caretPosition = 0;
	 }

	 if (m_caretPosition > (int)m_inputLine.size())
	 {
		 m_caretPosition = (int)m_inputLine.size();
	 }

	 ResetCaret();
 }


 void DevConsole::RemoveCharacterOnLine(int direction)
 {
	 if (direction < 0)
	 {
		 if (m_caretPosition > 0)
		 {
			 m_inputLine.erase(m_caretPosition - 1, 1);
			 SetCaretPosition(m_caretPosition - 1);
		 }
	 }
	 else if (direction > 0)
	 {
		 if (m_caretPosition <= (int)m_inputLine.size() - 1)
		 {
			 m_inputLine.erase(m_caretPosition, 1);
			 SetCaretPosition(m_caretPosition);
		 }
	 }
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void DevConsole::ResetCaret()
 {
	 m_caretStopwatch->Restart();
	 m_caretVisible = true;
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 std::string const DevConsole::GetPreviousCommand()
 {
	 if (m_historyIndex < (int)m_commandHistory.size() - 1)
	 {

		 m_historyIndex++;
		 return m_commandHistory[m_historyIndex];
	 }

	 if ((int)m_commandHistory.size() == 0)
	 {
		 return "";
	 }

	 return m_commandHistory[(int)m_commandHistory.size() - 1];
 }
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 std::string const DevConsole::GetNextCommand()
 {
	 if (m_historyIndex > 0)
	 {
		 m_historyIndex--;
		 return m_commandHistory[m_historyIndex];
	 }
	 m_historyIndex = -1;
	 return "";
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void DevConsole::ClearInput()
 {
	 m_inputLine = "";
	 SetCaretPosition(0);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void DevConsole::ExecuteXmlCommandScriptNode(tinyxml2::XMLElement  const& commandScriptXmlElement)
 {
	 const tinyxml2::XMLElement* element = commandScriptXmlElement.FirstChildElement();
	 if (element == nullptr) 
	 {
		 ERROR_AND_DIE("Provided XML element does not contain child elements for commands.");
	 }

	 while (element != nullptr)
	 {
		 std::string functionName = element->Name();
		 const tinyxml2::XMLAttribute* attributeElements = element->FirstAttribute();

		 if (attributeElements == nullptr) 
		 {
			 ERROR_AND_DIE(Stringf("XML command element '%s' has no attributes for command arguments.", functionName.c_str()));
		 }

		 EventArgs args;
		 while (attributeElements != nullptr)
		 {
			 std::string keyName = attributeElements->Name();
			 std::string keyValue = attributeElements->Value();
			 args.SetValue(keyName, keyValue);
			 attributeElements = attributeElements->Next();
		 }

		 g_theEventSystem->FireEvent(functionName, args);
		 element = element->NextSiblingElement();
	 }
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------
 void DevConsole::ExecuteXmlCommandScriptFile(std::string& filePath)
 {
	 tinyxml2::XMLDocument xml_doc;
	 tinyxml2::XMLError result = xml_doc.LoadFile(filePath.c_str());

	 if (result != tinyxml2::XML_SUCCESS) 
	 {
		 ERROR_AND_DIE("Failed to load XML file for command script.");
	 }

	 tinyxml2::XMLElement* rootElement = xml_doc.RootElement();
	 if (rootElement == nullptr)
	 {
		 ERROR_AND_DIE("XML file for command script does not have a valid root element.");
	 }

	 ExecuteXmlCommandScriptNode(*rootElement);
 }
 //--------------------------------------------------------------------------------------------------------------------------------------------------------