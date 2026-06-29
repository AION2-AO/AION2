#include "pch.h"
#include "Player.h"

Player::Player(Protocol::ClassType playerClass, int32 exp, int32 gold, int32 hp)
	: _class(playerClass), _exp(exp), _gold(gold), _hp(hp)
{
}

void Player::SetPlayerInfo(Protocol::ClassType playerClass, int32 exp, int32 gold, int32 hp)
{
	_class = playerClass;
	_exp = exp;
	_gold = gold;
	_hp = hp;

	_playerPos.set_x(100.f);
	_playerPos.set_y(200.f);
	_playerPos.set_z(300.f);

	_playerRot.set_pitch(0);
	_playerRot.set_yaw(0);
	_playerRot.set_roll(0);
}
