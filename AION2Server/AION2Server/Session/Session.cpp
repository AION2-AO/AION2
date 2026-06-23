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
		std::cout << "IsConnected failed\n";
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
		std::cout << "registerSend true\n";

		RegisterSend();
	}
}

bool Session::Connect()
{
	return false;
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	// TODO: Logout logic
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
	return false;
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
	std::cout << "registerSend start\n";

	_sendEvent.Init();
	_sendEvent.owner = shared_from_this();

	std::vector<WSABUF> wsaBufs;
	{
		std::lock_guard<std::mutex> lock(_sendLock);
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
			_sendEvent.owner = nullptr; // RELEASE_REF
			_sendEvent.sendBuffers.clear(); // RELEASE_REF
			_sendRegistered.store(false);
		}
	}
}

void Session::ProcessConnect()
{
//	_connectEvent.owner = nullptr;
	_connected.store(true);

	GetService()->AddSession(GetSessionRef());

	// TODO: OnConnected override logic
	std::cout << "Client Connected!" << std::endl;

	OnConnected();
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	GetService()->ReleaseSession(GetSessionRef());
	// TODO: OnDisconnected override logic
	std::cout << "Client Disconnected!" << std::endl;
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
	// TODO: Packet processing logic
	// For now, just echo or print
	std::cout << "Received " << numBytes << " bytes" << std::endl;
	//_recvBuffer.OnRead(dataSize);

	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	RegisterRecv(); 
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnReadError");
		return;
	}
}

void Session::ProcessSend(int32 numBytes)
{
	if (numBytes == 0)
	{
		Disconnect(L"SendZero");
		return;
	}

	_sendEvent.sendBuffers.clear();

	std::lock_guard<std::mutex> lock(_sendLock);
	if (_sendQueue.empty())
	{
		_sendRegistered.store(false);
	}
	else
	{
		RegisterSend();
	}
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
		std::cout << "Parsing Packet: ID=" << static_cast<uint16>(header.id) << ", Size=" << header.size << std::endl;
		if (dataSize < header.size)
		{
			break;
		}

		OnRecvPacket(&buffer[processLen], header.size);
		processLen += header.size;
	}

	return processLen;
}
