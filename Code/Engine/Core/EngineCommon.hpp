#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSytem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Vertex_PCU;
struct Vertex_PNCU;
struct Vertex_PCUTBN;
class NamedStrings;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x)
#pragma warning( disable : 26812)
//--------------------------------------------------------------------------------------------------------------------------------------------------------
extern NamedStrings g_gameConfigBlackboard;
extern InputSystem* g_theInput;
extern Window*		g_theWindow;
extern Renderer*	g_theRenderer;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
typedef std::vector<Vertex_PCU> VertexArray;
typedef std::vector<Vertex_PNCU> VertexNormalArray;
typedef std::vector<Vertex_PCUTBN> VertexNormalTangentArray;
typedef std::vector<unsigned int> IndexArray;
//--------------------------------------------------------------------------------------------------------------------------------------------------------