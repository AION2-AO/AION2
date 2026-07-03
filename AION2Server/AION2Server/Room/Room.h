#pragma once
#include "JobQueue.h"
#include "Protocol.pb.h"

#include <map>

// ·Îºñ
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
	void HandleSavePlayerHp();
	void HandleChat(Protocol::S_ChatPacket pkt);

	void AddPlayer(PlayerRef player);

	void Broadcast(SendBufferRef sendBuffer, uint64 exceptId);

private:
	std::map<uint64, PlayerRef> _players;
};

extern RoomRef GRoom;