#pragma once
#include "pch.h"
#include <map>
#include <vector>
#include <set>
#include "Room.h"

const int MAX_DUNGEON = 4;
const int MAX_MEMBERS = 3;

// Manages a single dungeon instance
class Dungeon : public std::enable_shared_from_this<Dungeon>
{
public:
	Dungeon(int32 dungeonId, PlayerRef leader);
	~Dungeon() = default;

	int32 GetId() const { return _dungeonId; }
	Protocol::RoomStatus GetStatus() const { return _status; }
	void SetStatus(Protocol::RoomStatus status) { _status = status; }

	PlayerRef GetLeader() const { return _leader; }
	const std::vector<PlayerRef>& GetMembers() const { return _members; }

	bool AddMember(PlayerRef player);
	bool RemoveMember(PlayerRef player);

	bool IsFull() const;
	int32 GetFreeIndex() const;

	void Broadcast(SendBufferRef sendBuffer);
	
	Protocol::DungeonInfo ToProto();

	void SetDediSession(DedicatedSessionRef session) { _dediSession = session; }
	DedicatedSessionRef GetDediSession() const { return _dediSession; }

private:
	int32 _dungeonId;
	Protocol::RoomStatus _status = Protocol::RoomStatus::RECRUITING;
	PlayerRef _leader;
	std::vector<PlayerRef> _members;
	DedicatedSessionRef _dediSession = nullptr;
};

// Manages the dungeon lobby and lists
class DungeonWaitingRoom : public Room
{
public:
	DungeonWaitingRoom();
	virtual ~DungeonWaitingRoom() override;

	void HandleWaitingRoom(PlayerRef player);
	void HandleLeaveWaitingRoom(PlayerRef player);
	void HandleCreateDungeon(PlayerRef player);
	void HandleEnterDungeon(PlayerRef player);
	void HandleDungeonStart(PlayerRef player, int32 dungeonId);

	void WaitingRoomBroadcast(SendBufferRef sendBuffer, uint64 exceptId = 0);

private:
	int32 GetFreeDungeonId();

private:
	std::map<uint64, PlayerRef> _waitingPlayers;
	std::map<int32, DungeonRef> _dungeons;
	std::set<int32> _freeDungeonIds;
	uint32_t _nextDungeonId = 1;
};

extern DungeonWaitingRoomRef GDungeonWaitingRoom;
