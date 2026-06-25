#pragma once
#include "JobQueue.h"
#include "Protocol.pb.h"

#include <map>

// ´øÀü
class Room : public JobQueue
{
public:
	Room();
	virtual ~Room();

public:
	void EnterRoom(PlayerRef player);
	void LeaveRoom(PlayerRef player);

	bool HandleEnterPlayer(PlayerRef player);
	bool HandleLeavePlayer(PlayerRef player);
	void HandleMove(Protocol::C_MovePacket pkt, PlayerRef player);

	void AddPlayer(PlayerRef player);

	void Broadcast(SendBufferRef sendBuffer, uint64 exceptId);

private:
	std::map< uint64, PlayerRef> _players;
};

extern RoomRef GRoom;