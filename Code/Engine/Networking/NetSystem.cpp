#include "Engine/Networking/NetSystem.hpp"
#ifdef ENABLE_NETWORKING
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#pragma comment(lib, "Ws2_32.lib")
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedProperties.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
NetSystem* g_theNetSystem = nullptr;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
NetSystem::NetSystem(const NetSystemConfig& config)
	: m_config(config)
{
	m_sendBuffer = new char[m_config.m_sendBufferSize];
	m_recvBuffer = new char[m_config.m_recvBufferSize];
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
NetSystem::~NetSystem()
{
	delete[] m_sendBuffer;
	delete[] m_recvBuffer;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NetSystem::Startup()
{
	g_theEventSystem->SubscribeEventCallbackFunction("BurstTest", NetSystem::Event_BurstTest);
	g_theEventSystem->SubscribeEventCallbackFunction("Echo", NetSystem::Event_Echo);
	g_theEventSystem->SubscribeEventCallbackFunction("RemoteCommand", NetSystem::Event_RemoteCommand);
	
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		ERROR_AND_DIE("Failed to initialize WinSock");
	}
	
	if (m_config.m_modeString == "CLIENT" || m_config.m_modeString == "Client" || m_config.m_modeString == "client")
	{
		ClientStartup();
	}

	if (m_config.m_modeString == "SERVER" || m_config.m_modeString == "Server" || m_config.m_modeString == "server")
	{
		ServerStartup();
	}

	else if (m_config.m_modeString == "NONE" || m_config.m_modeString == "None" || m_config.m_modeString == "none")
	{
		m_mode = Mode::NONE;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NetSystem::BeginFrame()
{
	if (m_mode == Mode::CLIENT && m_clientSocket != INVALID_SOCKET)
	{
		ClientBeginFrame();
	}

	else if (m_mode == Mode::SERVER && m_listenSocket != INVALID_SOCKET)
	{
		ServerBeginFrame();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NetSystem::EndFrame()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NetSystem::ServerStartup()
{
	m_mode = Mode::SERVER;
	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listenSocket == INVALID_SOCKET)
	{
		ERROR_AND_DIE("Failed to create listening socket");
	}

	unsigned long blockingMode = 1;
	if (ioctlsocket(m_listenSocket, FIONBIO, &blockingMode) == SOCKET_ERROR)
	{
		ERROR_AND_DIE("Failed to set non-blocking mode for listening socket");
	}

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	Strings hostParts = SplitStringOnDelimiter(m_config.m_hostAddressString, ':');
	if (hostParts.size() != 2)
	{
		ERROR_AND_DIE("Invalid IP address and port format");
	}
	DebuggerPrintf("Server IP Address: %s \n", hostParts[0].c_str());

	int portValue = std::stoi(hostParts[1]);
	if (portValue < 0 || portValue > 65535) 
	{
		ERROR_AND_DIE("The port number is out of range for unsigned short.");
	}
	else 
	{
		m_hostPort = static_cast<unsigned short>(portValue);
		addr.sin_port = htons(m_hostPort);
	}
	DebuggerPrintf("Server Port: %d \n", portValue);
	if (bind(m_listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		ERROR_AND_DIE("Failed to bind listening socket");
	}

	if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		ERROR_AND_DIE("Failed to put listening socket into listening mode");
	}

	m_serverState = ServerState::LISTENING;
	DebuggerPrintf("Server is in listening state and waiting for a client \n");
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NetSystem::ServerBeginFrame()
{
	if (m_serverState == ServerState::LISTENING) {

		uintptr_t socket = accept(m_listenSocket, NULL, NULL);

		//m_clientSocket = socket;
		if (socket != INVALID_SOCKET) {
			m_clientSocket = socket;
			unsigned long blockingMode = 1;
			int result = ioctlsocket(m_clientSocket, FIONBIO, &blockingMode);
			if (result == SOCKET_ERROR) {
				ERROR_AND_DIE(Stringf("Server listening set blocking mode error: %d\n", WSAGetLastError()));
			}
			else {
				m_serverState = ServerState::CONNECTED;
			}

		}
		else {
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				ERROR_AND_DIE(Stringf("Server accept error: %d\n", WSAGetLastError()));
			}
			else
			{
				DebuggerPrintf("No connection for Server to accept\n");
				return;
			}
		}
	}

	if (m_serverState == ServerState::CONNECTED) 
	{

		int result = recv(m_clientSocket, m_recvBuffer, sizeof(m_recvBuffer), 0);
		if (result == SOCKET_ERROR) 
		{
			int error = WSAGetLastError();
			if (error == WSAECONNRESET) 
			{
				m_serverState = ServerState::LISTENING;
				Shutdown();
				Startup();
				return;
			}
			else if (error != WSAEWOULDBLOCK) 
			{
				ERROR_AND_DIE(Stringf("Server receiving error: %d\n", WSAGetLastError()));
			}
		}
		else if (result == 0) {
			m_serverState = ServerState::LISTENING;
			Shutdown();
			Startup();
			return;
		}

		// receive msg too long
		if (result != SOCKET_ERROR && result > m_config.m_recvBufferSize) {
			ERROR_AND_DIE("Server receive message size is larger than receive buffer");
		}

		// receive message
		ReceiveMessage(m_recvBuffer, result);

		// send message
		if ((int)m_sendQueue.size() > 0) {
			AddQueueToSendBuffer();
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NetSystem::ClientStartup()
{
	m_mode = Mode::CLIENT;
	m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_clientSocket == INVALID_SOCKET)
	{
		ERROR_AND_DIE("Failed to create client socket");
	}

	unsigned long blockingMode = 1;
	if (ioctlsocket(m_clientSocket, FIONBIO, &blockingMode) == SOCKET_ERROR)
	{
		ERROR_AND_DIE("Failed to set non-blocking mode for client socket");
	}
	IN_ADDR addr;

	Strings hostParts = SplitStringOnDelimiter(m_config.m_hostAddressString, ':');
	if (hostParts.size() != 2)
	{
		ERROR_AND_DIE("Invalid IP address and port format");
	}
	DebuggerPrintf("Client IP Address: %s \n", hostParts[0].c_str());
	if (inet_pton(AF_INET, hostParts[0].c_str(), &addr) != 1)
	{
		ERROR_AND_DIE("Failed to convert IP address string to binary");
	}
	else
	{
		m_hostAddress = ntohl(addr.s_addr);  
	}

	int portValue = std::stoi(hostParts[1]);
	if (portValue < 0 || portValue > 65535) 
	{
		// Handle error: The port number is out of range for unsigned short.
	}
	else 
	{
		m_hostPort = static_cast<unsigned short>(portValue);
	}
	DebuggerPrintf("Client Port: %d \n", portValue);
	m_clientState = ClientState::READY_TO_CONNECT;
	DebuggerPrintf("Client is ready to connect \n");
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NetSystem::ClientBeginFrame()
{
	if (m_clientState == ClientState::READY_TO_CONNECT)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl(m_hostAddress);
		addr.sin_port = htons(m_hostPort);
		int result = connect(m_clientSocket, (sockaddr*)(&addr), (int)sizeof(addr));

		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				ERROR_AND_DIE(Stringf("Client connect error: %d\n", WSAGetLastError()));
			}
			else
			{
				m_clientState = ClientState::CONNECTING;
				return;
			}
		}
	}

	if (m_clientState ==ClientState::CONNECTING) 
	{
		//Check if our connection attempt failed.
		fd_set failedSockets;
		FD_ZERO(&failedSockets);
		FD_SET(m_clientSocket, &failedSockets);
		timeval failedWaitTime = { };
		int failedResult = select(0, NULL, NULL, &failedSockets, &failedWaitTime);

		if (failedResult == SOCKET_ERROR) {
			ERROR_AND_DIE(Stringf("Client connection attempt failed error: %d\n", WSAGetLastError()));
		}
		if (failedResult > 0 && FD_ISSET(m_clientSocket, &failedSockets)) {
			m_clientState = ClientState::READY_TO_CONNECT;
			return;
		}

		//Check if our connection attempt completed.
		fd_set successSockets;
		FD_ZERO(&successSockets);
		FD_SET(m_clientSocket, &successSockets);
		timeval successWaitTime = { };
		int successResult = select(0, NULL, &successSockets, NULL, &successWaitTime);
		if (successResult == SOCKET_ERROR) {
			ERROR_AND_DIE(Stringf("Client connection attempt completed error: %d\n", WSAGetLastError()));
		}
		//We are connected if the following is true.
		if (successResult > 0 && FD_ISSET(m_clientSocket, &successSockets)) {
			m_clientState = ClientState::CONNECTED;
		}
	}

	if (m_clientState == ClientState::CONNECTED)
	{
		int result = recv(m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0);
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAECONNRESET)
			{
				m_clientState = ClientState::READY_TO_CONNECT;
				Shutdown();
				Startup();
				return;
			}
			else if (error != WSAEWOULDBLOCK)
			{
				ERROR_AND_DIE(Stringf("Client receiving error: %d\n", WSAGetLastError()));
			}
		}
		else if (result == 0)
		{
			m_clientState =ClientState::READY_TO_CONNECT;
			Shutdown();
			Startup();
			return;
		}

		// receive msg too long
		if (result != SOCKET_ERROR && result > m_config.m_recvBufferSize)
		{
			ERROR_AND_DIE("Client receive message size is larger than receive buffer");
		}

		// receive message
		ReceiveMessage(m_recvBuffer, result);
		// send message
		if ((int)m_sendQueue.size() > 0) {
			AddQueueToSendBuffer();
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool NetSystem::Event_BurstTest(EventArgs& args)
{
	(void)args;
	if (g_theNetSystem->m_mode == Mode::NONE)
	{
		return true;
	}

	for (int i = 1; i <= 20; ++i) 
	{
		std::string message = "Echo Message=" + std::to_string(i);
		g_theNetSystem->m_sendQueue.push_back(message);
	}
	
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool NetSystem::Event_Echo(EventArgs& args)
{
	std:: string val = args.GetValue("Message", "INVALID");
	if (val == "INVALID")
	{
		return false;
	}
	g_theDevConsole->AddLine(Rgba8(255, 0, 255), val);
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool NetSystem::Event_RemoteCommand(EventArgs& args)
{
	std::string val = args.GetValue("Command", "INVALID");
	if (val == "INVALID")
	{
		return false;
	}
	TrimString(val, '"');
	g_theNetSystem->m_sendQueue.push_back(val);
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NetSystem::AddQueueToSendBuffer()
{
	int size = 0;
	for (int i = 0; i < (int)m_sendQueue.size(); ++i)
	{

		if (!strncpy_s(m_sendBuffer, m_config.m_sendBufferSize, m_sendQueue[i].c_str(), strlen(m_sendQueue[i].c_str()) + 1) == 0)
		{
			ERROR_AND_DIE("This copy action is failed");
		}
		size = (int)m_sendQueue[i].size() + 1;
		if (size > m_config.m_sendBufferSize)
		{
			ERROR_AND_DIE("the size of send buffer is too big");
		}
		int sendResult = send(m_clientSocket, m_sendBuffer, (int)strlen(m_sendBuffer) + 1, 0);
		if (sendResult == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				ERROR_AND_DIE("Error happened at send()\n");
			}
		}
	}

	m_sendQueue.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool NetSystem::ReceiveMessage(char*& messageBuffer, int length)
{
	for (int i = 0; i < length; i++) 
	{
		char const& temp = messageBuffer[i];
		if (temp == -51) 
		{
			break;
		}
		if (temp == '\0') 
		{
			if (!m_recvRemaining.empty()) 
			{
				g_theDevConsole->Execute(m_recvRemaining);
				m_recvRemaining.clear();
			}
		}
		else 
		{
			m_recvRemaining += temp;
		}
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NetSystem::AddSendMessage(std::string message)
{
	m_sendQueue.push_back(message);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void NetSystem::Shutdown()
{
	if (m_mode == Mode::CLIENT)
	{
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
	}
	if (m_mode == Mode::SERVER)
	{
		closesocket(m_clientSocket);
		closesocket(m_listenSocket);
		m_clientSocket = INVALID_SOCKET;
	}

	WSACleanup();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
#endif // ENABLE_NETWORKING