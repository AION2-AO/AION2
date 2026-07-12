#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"
#include "SendBuffer.h"

Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
	{
		//std::cout << "IsConnected failed\n";
		return;
	}

	bool registerSend = false;
	{
		std::lock_guard<std::mutex> lock(_sendLock);
		_sendQueue.push(sendBuffer);
		if (_sendRegistered.exchange(true) == false)
		{
			registerSend = true;
		}
	}

	if (registerSend)
	{
		RegisterSend();
	}
}

bool Session::Connect()
{
	if (IsConnected())
		return false;

	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;

	// Bind to ANY address first for ConnectEx
	NetAddress anyAddress(L"0.0.0.0", 0);
	if (SocketUtils::Bind(_socket, anyAddress) == false)
	{
		::closesocket(_socket);
		_socket = INVALID_SOCKET;
		return false;
	}

	// Register socket to IOCP Core
	if (GetService()->GetIocpCore()->Register(shared_from_this()) == false)
	{
		::closesocket(_socket);
		_socket = INVALID_SOCKET;
		return false;
	}

	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	std::wcout << "Disconnected: " << cause << "\n";
	OnDisconnected();;
	RegisterDisConnect();
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numBytes)
{
	switch (iocpEvent->_type)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Disconnect:
		ProcessDisconnect();
		break;
	case EventType::Recv:
		ProcessRecv(numBytes);
		break;
	case EventType::Send:
		ProcessSend(numBytes);
		break;
	}
}

bool Session::RegisterConnect()
{
	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();
	_connectEvent._type = EventType::Connect;

	DWORD bytesSent = 0;
	SOCKADDR_IN addr = GetAddress().GetSockAddr();

	bool pending = SocketUtils::ConnectEx(
		_socket,
		reinterpret_cast<const SOCKADDR*>(&addr),
		sizeof(addr),
		nullptr,
		0,
		&bytesSent,
		&_connectEvent
	);

	if (pending == false)
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_connectEvent.owner = nullptr; // Reset ownership
			return false;
		}
	}

	return true;
}

void Session::RegisterDisConnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this();

	if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			ProcessDisconnect();
		}
	}
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	DWORD bytesReceived = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, &bytesReceived, &flags, &_recvEvent, NULL))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			Disconnect(L"RecvError");
		}
	}
}

void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	_sendEvent.Init();
	_sendEvent.owner = shared_from_this();

	std::vector<WSABUF> wsaBufs;
	{
		std::lock_guard<std::mutex> lock(_sendLock);
		if (_sendQueue.empty())
		{
			_sendRegistered.store(false);
			_sendEvent.owner = nullptr;
			return;
		}

		int32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();
			writeSize += sendBuffer->WriteSize();

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (SendBufferRef sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, 0, &_sendEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_sendEvent.owner = nullptr;
			_sendEvent.sendBuffers.clear(); 
			_sendRegistered.store(false);
		}
	}
}

void Session::ProcessConnect()
{
	//	_connectEvent.owner = nullptr;
	_connected.store(true);

	GetService()->AddSession(GetSessionRef());
	if (GetService()->GetServiceType() == ServiceType::MMOServer)
	{
		std::wcout << L"Client Connected! IP: " << GetService()->GetNetAddress().GetIpAddress()
			<< L", Port: " << GetService()->GetNetAddress().GetPort() 
			<< L" | Active Sessions: " << GetService()->GetCurrentSessionCount() << std::endl;
	}

	else
	{
		std::wcout << L"Dedi Connected! IP: " << GetService()->GetNetAddress().GetIpAddress()
			<< L", Port: " << GetService()->GetNetAddress().GetPort() 
			<< L" | Active Sessions: " << GetService()->GetCurrentSessionCount() << std::endl;
	}

	OnConnected();
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	GetService()->ReleaseSession(GetSessionRef());

	std::cout << "Client Disconnected! Active Sessions: " << GetService()->GetCurrentSessionCount() << std::endl;
}

void Session::ProcessRecv(int32 numBytes)
{
	if (numBytes == 0)
	{
		Disconnect(L"RecvZero");
		return;
	}

	if (_recvBuffer.OnWrite(numBytes) == false)
	{
		Disconnect(L"RecvBufferOverflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize();

	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnReadError");
		return;
	}

	_recvBuffer.Clean();

	RegisterRecv();
}

void Session::ProcessSend(int32 numBytes)
{
	_sendEvent.owner = nullptr;
	_sendEvent.sendBuffers.clear();

	if (numBytes == 0)
	{
		Disconnect(L"SendZero");
		return;
	}

	RegisterSend();
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;
	default:
		// TODO : Log
		std::cout << "Handle Error : " << errorCode << std::endl;
		break;
	}
}

PacketSession::PacketSession()
{

}

PacketSession::~PacketSession()
{

}

int32 PacketSession::OnRecv(BYTE* buffer, int32 len)
{
	int32 processLen = 0;
	while (true)
	{
		int32 dataSize = len - processLen;
		if (dataSize < sizeof(PacketHeader))
		{
			break;
		}
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
		if (dataSize < header.size)
		{
			break;
		}

		OnRecvPacket(&buffer[processLen], header.size);
		processLen += header.size;
	}

	return processLen;
}