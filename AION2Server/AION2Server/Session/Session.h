#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

class Session : public IocpObject
{
	friend class Listener;
	friend class IocpCore;
	friend class Service;

	enum
	{
		BUFFER_SIZE = 0x1000, // 64KB
	};

public:
	Session();
	virtual ~Session();

public:
	void Send(SendBufferRef sendBuffer);
	bool Connect();
	void Disconnect(const WCHAR* cause);

	void SetService(ServiceRef service) { _service = service; }
	void SetNetAddress(NetAddress address) { _netAddress = address; }

	shared_ptr<Service>	GetService() { return _service.lock(); }
	NetAddress GetAddress() { return _netAddress; }
	SOCKET GetSocket() { return _socket; }
	SessionRef GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

	bool IsConnected() { return _connected; }

private:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numBytes = 0) override;

private:
	void RegisterConnect();
	void RegisterDisConnect();
	void RegisterRecv();
	void RegisterSend();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 numBytes);
	void ProcessSend(int32 numBytes);

	void HandleError(int32 errorCode);

protected:
	virtual void OnConnected() {}
	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void OnSend(int32 len) {}
	virtual void OnDisconnected() {}

private:
	SOCKET _socket;
	NetAddress _netAddress;
	weak_ptr<Service> _service;
	atomic<bool>		_connected = false;

	RecvBuffer	_recvBuffer;

	queue<SendBufferRef>	_sendQueue;
	atomic<bool>			_sendRegistered = false;

	std::mutex _sendLock;
private:
	ConnectEvent		_connectEvent;
	DisconnectEvent		_disconnectEvent;
	RecvEvent			_recvEvent;
	SendEvent			_sendEvent;
};

#include "../../Common/Protocol.h"

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

	PacketSessionRef GetPacketSessionRef() { return static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	virtual int32 OnRecv(BYTE* buffer, int32 len) sealed;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) abstract;
};
