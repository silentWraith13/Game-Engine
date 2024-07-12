#pragma once
#define ENABLE_NETWORKING

#ifdef ENABLE_NETWORKING

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <string>

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class NetSystem;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
extern NetSystem* g_theNetSystem;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct NetSystemConfig
{
	std::string m_modeString;
	std::string m_hostAddressString;
	int			m_sendBufferSize = 2048;
	int			m_recvBufferSize = 2048;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class Mode
{
	NONE = 0,
	CLIENT,
	SERVER,
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class ClientState
{
	INVALID,
	READY_TO_CONNECT,
	CONNECTING,
	CONNECTED,
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
enum class ServerState
{
	INVALID,
	LISTENING,
	CONNECTED,
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class NetSystem
{
public:
	NetSystem(const NetSystemConfig& config);
	~NetSystem();

	void		Startup();
	void		Shutdown();
	void		BeginFrame();
	void		EndFrame();
	
	void		ServerStartup();
	void		ServerBeginFrame();

	void		ClientStartup();
	void		ClientBeginFrame();

	static bool Event_BurstTest(EventArgs& args);
	static bool Event_Echo(EventArgs& args);
	static bool Event_RemoteCommand(EventArgs& args);
	
	void		AddQueueToSendBuffer();
	bool		ReceiveMessage(char*& messageBuffer, int length);
	void		AddSendMessage(std::string message);

	NetSystemConfig				m_config;
	Mode						m_mode = Mode::NONE;

	ClientState					m_clientState = ClientState::INVALID;
	ServerState					m_serverState = ServerState::INVALID;

	uintptr_t					m_clientSocket = ~0ull;
	uintptr_t					m_listenSocket = ~0ull;

	unsigned long				m_hostAddress = 0;
	unsigned short				m_hostPort = 0;

	char						*m_sendBuffer = nullptr;
	char						*m_recvBuffer = nullptr;

	std::vector<std::string>	m_sendQueue;
	std::string					m_recvRemaining;
	int							m_frameCounter = 0;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------

#endif // ENABLE_NETWORKING