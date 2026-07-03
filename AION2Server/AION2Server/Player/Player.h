#pragma once

#include "Protocol.pb.h"

class Player
{
public:
	Player() = default;
	Player(Protocol::ClassType playerClass, int32 exp, int32 gold, int32 hp);

	void SetPlayerInfo(uint64 playerId, Protocol::ClassType playerClass, int32 exp, int32 gold, int32 hp);

	uint64 GetId() { return _playerId; }
	std::string GetName() { return _name; }
	Protocol::ClassType GetClass() { return _class; }

	int32 GetGold() { return _gold; }
	int32 GetHp() { return _hp; }
	int32 GetExp() { return _exp; }

	void SetName(std::string playerName) { _name = playerName; }
	void SetPos(Protocol::Vector3 inPos) { _playerPos = inPos; }
	void SetRot(Protocol::Rotator3 inRot) { _playerRot = inRot; }
	void SetReady(bool ready) { _isReady = ready; }
	void SetHp(int32 NewHp) { _hp = NewHp; }

	Protocol::Vector3 GetPos() { return _playerPos; }
	Protocol::Rotator3 GetRot() { return _playerRot; }

	bool GetReady() { return _isReady; }

public:
	uint64 _playerId = 0;
	std::string _name;
	std::weak_ptr<class GameSession>_ownerSession;

	std::atomic<std::weak_ptr<Room>> room;

public:
	Protocol::ClassType _class;
	int32 _gold;
	int32 _hp;
	int32 _exp;

	bool _isReady = false;
	
	Protocol::Vector3 _playerPos;
	Protocol::Rotator3 _playerRot;
};
