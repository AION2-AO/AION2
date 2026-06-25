#pragma once

#include "Protocol.pb.h"

class Player
{
public:
	Player() = default;
	Player(int32 playerClass, int32 exp, int32 gold, int32 hp);

	void SetPlayerInfo(int32 playerClass, int32 exp, int32 gold, int32 hp);

	uint64 GetId() { return _playerId; }
	int32 GetGold() { return _gold; }
	int32 GetHp() { return _hp; }
	int32 GetExp() { return _exp; }

	void SetPos(Protocol::Vector3 inPos) { _playerPos = inPos; }
	void SetRot(Protocol::Rotator3 inRot) { _playerRot = inRot; }

	Protocol::Vector3 GetPos() { return _playerPos; }
	Protocol::Rotator3 GetRot() { return _playerRot; }

public:
	uint64 _playerId = 0;
	std::string _name;
	std::weak_ptr<class GameSession>_ownerSession;

	std::atomic<std::weak_ptr<Room>> room;

public:
	int32 _class;
	int32 _gold;
	int32 _hp;
	int32 _exp;

	Protocol::Vector3 _playerPos;
	Protocol::Rotator3 _playerRot;
};
