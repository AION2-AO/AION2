#pragma once
#include "Session.h"

class GameSession : public PacketSession
{
public:
	~GameSession() {}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	const std::string GetToken();
	std::string GetMyToken() { return _token; }

public:
	std::shared_ptr<Player> _player;
	std::string _token;
};

