#pragma once
#include "Session.h"

class DedicatedSession : public PacketSession
{
public:
	~DedicatedSession() {}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

	std::string GetToken()const;

	void SetAddrInfo(const std::string& ip, const int32 port);
	void SetUsing(bool use) { _isUsing = use; }

	const std::string GetIP(){ return _ip; }
	const int32 GetPort(){ return _port; }

	const bool IsUsing() { return _isUsing; }
private:
	std::string _ip;
	int32 _port;

	bool _isUsing;
};

