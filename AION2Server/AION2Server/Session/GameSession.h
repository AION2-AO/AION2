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

public:
	shared_ptr<class Player> _currenyPlayer;
};

