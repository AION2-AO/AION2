#include "pch.h"
#include "SocketUtils.h"

LPFN_CONNECTEX SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX SocketUtils::AcceptEx = nullptr;

bool SocketUtils::Init()
{
	::WSADATA wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	SOCKET dummySocket = CreateSocket();
	bool result = true;
	result &= BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx));
	result &= BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx));
	result &= BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx));
	Close(dummySocket);

	return result;
}

void SocketUtils::Clear()
{
	::WSACleanup();
}

bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), &bytes, NULL, NULL);
}

SOCKET SocketUtils::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
	{
		::closesocket(socket);
		socket = INVALID_SOCKET;
	}
}

bool SocketUtils::SetLinger(SOCKET socket, uint16 onoff, uint16 linger)
{
	LINGER opt;
	opt.l_onoff = onoff;
	opt.l_linger = linger;
	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, opt);
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	int32 value = (flag ? 1 : 0);
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, value);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	int32 value = (flag ? 1 : 0);
	return SetSockOpt(socket, IPPROTO_TCP, TCP_NODELAY, value);
}

bool SocketUtils::SetUpdateAcceptContext(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::Bind(SOCKET socket, NetAddress address)
{
	return ::bind(socket, reinterpret_cast<const SOCKADDR*>(&address.GetSockAddr()), sizeof(SOCKADDR_IN)) != SOCKET_ERROR;
}

bool SocketUtils::Listen(SOCKET socket, int32 backlog)
{
	return ::listen(socket, backlog) != SOCKET_ERROR;
}
