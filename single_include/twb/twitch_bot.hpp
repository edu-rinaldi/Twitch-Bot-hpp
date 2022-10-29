/*
	MIT License

	Copyright (c) 2022 Eduardo Rinaldi

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef _TWITCH_BOT_HPP_

#ifdef _WIN64
#define USE_WINSOCK2
#else
#undef USE_WINSOCK2
#endif

#ifdef USE_WINSOCK2
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

typedef uint64_t SOCKET;
const int INVALID_SOCKET = 0;
const int SOCKET_ERROR	 = -1;

#define closesocket(socket) close(socket)
#endif

#include <assert.h>
#include <functional>
#include <regex>
#include <sstream>
#include <string>

#ifdef _DEBUG
#define TWB_LOG_ENABLED
#endif

#ifdef TWB_LOG_ENABLED
#include <iostream>

#define Log(x) std::cout << "[LOG] " << x << std::endl
#define LogError(x, err)                                                                                               \
	{                                                                                                                  \
		std::cerr << "[ERROR] " << x << std::endl;                                                                     \
		exit(err);                                                                                                     \
	}

#define CheckAndLogStatus(statusFn)                                                                                    \
	{                                                                                                                  \
		auto status = statusFn;                                                                                        \
		if (!status.hasSucceeded) { LogError(status.errorMessage, 1); }                                                \
	}
#else
#define Log(x)
#define LogError(x, err)
#define CheckAndLogStatus(statusFn) statusFn
#endif
namespace twb
{

constexpr static inline size_t RECEIVE_BUFFER_MAX_SIZE = 4096;

constexpr static inline const char* HOST = "irc.chat.twitch.tv";
constexpr static inline const char* PORT = "6667";

class TCPSocket
{
  public:
	TCPSocket(const std::string& host, const std::string& port);

	struct TCPOperationResult
	{
		bool hasSucceeded		 = false;
		std::string errorMessage = "";
	};

	TCPOperationResult Connect();
	TCPOperationResult Disconnect();

	TCPOperationResult Send(const std::string& message) const;
	bool ReceiveValue(std::string& outReceived) const;

  private:
	std::string m_Host;
	std::string m_Port;

/* Windows WinSock2 impl. */
#ifdef USE_WINSOCK2
	WSAData m_Data;
	WORD m_Version;
#endif
	SOCKET m_Socket;
};

/*
Supported IRC messages. Ref:
https://dev.twitch.tv/docs/irc#supported-irc-messages
*/
struct IRCMessage
{
	enum struct IRCMessageType
	{
		JOIN,
		NICK,
		PASS,
		PING,
		PONG,
		PRIVMSG,
		// Not implemented
		// NOTICE,
		// PART,
		///* Twitch specific IRC messages */
		// CLEARCHAT,
		// CLEARMSG,
		// GLOBALUSERSTATE,
		// HOSTTARGET,
		// RECONNECT,
		// ROOMSTATE,
		// USERNOTICE,
		// USERSTATE,
		// WHISPER
	} type;

	std::string message;

	static inline IRCMessage BuildJoin(const std::string& channelName)
	{
		std::stringstream messageBuilder;
		messageBuilder << "JOIN #" << channelName << "\r\n";
		return IRCMessage{.type = IRCMessageType::JOIN, .message = messageBuilder.str()};
	}

	static inline IRCMessage BuildNick(const std::string& nick)
	{
		std::stringstream messageBuilder;
		messageBuilder << "NICK " << nick << "\r\n";
		return IRCMessage{.type = IRCMessageType::NICK, .message = messageBuilder.str()};
	}

	static inline IRCMessage BuildPass(const std::string& pass)
	{
		std::stringstream messageBuilder;
		messageBuilder << "PASS " << pass << "\r\n";
		return IRCMessage{.type = IRCMessageType::PASS, .message = messageBuilder.str()};
	}

	static inline IRCMessage BuildPing()
	{
		return IRCMessage{.type = IRCMessageType::PING, .message = "PING :tmi.twitch.tv"};
	}

	static inline IRCMessage BuildPong()
	{
		return IRCMessage{.type = IRCMessageType::PONG, .message = "PONG :tmi.twitch.tv"};
	}

	static inline IRCMessage BuildPrivMsg(const std::string& channelName, const std::string& message)
	{
		// PRIVMSG #<channel name> :This is a sample message
		std::stringstream messageBuilder;
		messageBuilder << "PRIVMSG #" << channelName << " :" << message << "\r\n";
		return IRCMessage{.type = IRCMessageType::PRIVMSG, .message = messageBuilder.str()};
	}
};

using OnReceiveFunction = std::function<void(const std::string& senderUsername, const std::string& message)>;
using OnJoinChannelChatFunction = std::function<void()>;

class Bot
{
  public:
	Bot(const std::string& username, const std::string& password);
	~Bot();

	void ConnectTo(const std::string& channelToConnect);
	void Disconnect();
	void Run();
	void Message(const std::string& message) const;
	void Message(const std::string& destinationChannel, const std::string& message) const;

	void BindOnReceiveMessageCallback(OnReceiveFunction fn) { m_OnReceiveMessageCallback = fn; }
	void BindOnJoinChannelChatCallback(OnJoinChannelChatFunction fn) { m_OnJoinChannelChatCallBack = fn; }

  private:
	inline void HandlePing() const { m_Socket.Send(IRCMessage::BuildPong().message); }

  private:
	std::string m_BotUsername;
	std::string m_BotPassword;
	std::string m_ChannelConnected;

	bool m_ShouldRun;
	OnReceiveFunction m_OnReceiveMessageCallback = nullptr;
	OnJoinChannelChatFunction m_OnJoinChannelChatCallBack = nullptr;

	TCPSocket m_Socket;
};
} // namespace twb

/* Implementation */
namespace twb
{

/* TCPSocket implementation */
twb::TCPSocket::TCPSocket(const std::string& host, const std::string& port)
	: m_Host(host), m_Port(port), m_Socket()
#ifdef USE_WINSOCK2
	  ,
	  m_Data(), m_Version(MAKEWORD(2, 2))
{
	int wsResult = WSAStartup(m_Version, &m_Data);
	assert(wsResult == 0 && "Startup failed");
}
#else
{
}
#endif

inline TCPSocket::TCPOperationResult twb::TCPSocket::Connect()
{
	TCPOperationResult result = {.hasSucceeded = true};

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family	  = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(m_Host.c_str(), m_Port.c_str(), &hints, &res);

	// Create socket
	m_Socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (m_Socket == INVALID_SOCKET)
	{
		result.hasSucceeded = false;
#ifdef USE_WINSOCK2
		result.errorMessage = WSAGetLastError();
		WSACleanup();
#endif
		return result;
	}

	// Connect to server
	int connectResult = connect(m_Socket, res->ai_addr, res->ai_addrlen);
	if (connectResult == SOCKET_ERROR)
	{
		result.hasSucceeded = false;
#ifdef USE_WINSOCK2
		result.errorMessage = WSAGetLastError();
		WSACleanup();
#endif
		closesocket(m_Socket);
		return result;
	}

	return result;
}

inline TCPSocket::TCPOperationResult twb::TCPSocket::Disconnect()
{
	TCPOperationResult result = {.hasSucceeded = true};

	int closeSocketResult = closesocket(m_Socket);
	if (closeSocketResult == SOCKET_ERROR)
	{
#ifdef USE_WINSOCK2
		WSACleanup();
		result.errorMessage = WSAGetLastError();
#endif
		result.hasSucceeded = false;
	}

	return result;
}

inline TCPSocket::TCPOperationResult twb::TCPSocket::Send(const std::string& message) const
{
	TCPOperationResult result = {.hasSucceeded = true};

	int sendResult = send(m_Socket, message.c_str(), message.size() + 1, 0);
	if (sendResult == SOCKET_ERROR)
	{
		result.hasSucceeded = false;
#ifdef USE_WINSOCK2
		result.errorMessage = WSAGetLastError();
		WSACleanup();
#endif
		closesocket(m_Socket);
	}

	return result;
}
inline bool twb::TCPSocket::ReceiveValue(std::string& outReceived) const
{
	char tmp[RECEIVE_BUFFER_MAX_SIZE];

	int resSize = recv(m_Socket, tmp, RECEIVE_BUFFER_MAX_SIZE, 0);
	if (resSize > 0) { outReceived = std::string(tmp, 0, resSize); }
	return resSize > 0;
}

/* Bot implementation */
Bot::Bot(const std::string& username, const std::string& password)
	: m_BotUsername(username), m_BotPassword(password), m_ShouldRun(true), m_Socket(HOST, PORT)
{
}

inline twb::Bot::~Bot() { Disconnect(); }

inline void Bot::ConnectTo(const std::string& channelToConnect)
{
	CheckAndLogStatus(m_Socket.Connect());

	CheckAndLogStatus(m_Socket.Send(twb::IRCMessage::BuildPass(m_BotPassword).message));
	CheckAndLogStatus(m_Socket.Send(twb::IRCMessage::BuildNick(m_BotUsername).message));
	CheckAndLogStatus(m_Socket.Send(twb::IRCMessage::BuildJoin(channelToConnect).message));
	m_ChannelConnected = channelToConnect;

	this->Run();
}

inline void twb::Bot::Disconnect()
{
	m_ShouldRun = false;
	m_Socket.Disconnect();
}

inline void twb::Bot::Run()
{
	std::string received;
	while (m_ShouldRun)
	{
		// Fetch message from socket
		if (m_Socket.ReceiveValue(received))
		{
			if (received.find("PING") != std::string::npos) { HandlePing(); }
			else if (received.find(":tmi.twitch.tv") != std::string::npos)
			{
				Log("Received: " << received);
				/* a.t.m. we don't handle other IRC message types */
				continue;
			}
			else if (received.find(".tmi.twitch.tv JOIN") != std::string::npos)
			{
				Log("Received: " << received);
				if (m_OnJoinChannelChatCallBack != nullptr) { m_OnJoinChannelChatCallBack(); }
			}
			else
			{
				// Process message obtaining user and message
				std::smatch sm;
				std::regex_search(received, sm, std::regex("\\w+"));

				auto message = std::regex_replace(
					received, std::regex("^:\\w+!\\w+@\\w+\\.tmi\\.twitch\\.tv PRIVMSG #\\w+ :"), "");
				if (m_OnReceiveMessageCallback != nullptr) { m_OnReceiveMessageCallback(sm[0].str(), message); }
			}
		}
	}
}

inline void twb::Bot::Message(const std::string& destinationChannel, const std::string& message) const
{
	m_Socket.Send(IRCMessage::BuildPrivMsg(destinationChannel, message).message);
}

inline void twb::Bot::Message(const std::string& message) const { Message(m_ChannelConnected, message); }

} // namespace twb

#endif // !_TWITCH_BOT_HPP_