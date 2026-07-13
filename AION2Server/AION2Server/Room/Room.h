#pragma once
#include "JobQueue.h"
#include "Protocol.pb.h"

#include <map>


struct FPvpStateConfig
{
	Protocol::EPvpState State;
	int32 Duration; // 지속 시간 (초 단위)
};

static const FPvpStateConfig PvpStateSequence[] = {
	{ Protocol::PVP_STATE_INACTIVE,   60 },
	{ Protocol::PVP_STATE_WARN_START, 5   },
	{ Protocol::PVP_STATE_ACTIVE,     600 },
	{ Protocol::PVP_STATE_WARN_END,   5   } 
};
static const int32 NumPvpStates = sizeof(PvpStateSequence) / sizeof(FPvpStateConfig);

class Room : public JobQueue
{
public:
	Room();
	virtual ~Room();

public:
	void EnterRoom(PlayerRef player);
	void LeaveRoom(PlayerRef player);

	void UpdatePvpTimer();
	void BroadcastPvpState();

	bool HandleEnterPlayer(PlayerRef player);
	bool HandleLeavePlayer(PlayerRef player);
	void HandleMove(Protocol::C_MovePacket pkt, PlayerRef player);
	void HandlePlayerDash(Protocol::C_DashPacket pkt, PlayerRef player);
	void HandlePlayerJump(Protocol::C_JumpPacket pkt, PlayerRef player);
	void HandleAttack(Protocol::C_AttackPacket pkt, PlayerRef player);
	void ClearDeathPenalty(uint64 playerId);
	void HandleSavePlayerHp();
	void HandleChat(Protocol::S_ChatPacket pkt);

	void AddPlayer(PlayerRef player);

	void Broadcast(SendBufferRef sendBuffer, uint64 exceptId);

private:
	std::map<uint64, PlayerRef> _players;

private:
	int32 CurrentStateIndex = 1;   
	int32 PvpTimerSeconds = 600;  

	Protocol::EPvpState _currentPvpState = Protocol::PVP_STATE_INACTIVE;
	uint64 _currentStateStartTime = 0;
	int32 _currentStateDuration = 0;
};

extern RoomRef GRoom;
