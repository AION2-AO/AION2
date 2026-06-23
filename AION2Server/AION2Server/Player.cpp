#include "pch.h"
#include "Player.h"

Player::Player(int32 playerClass, int32 exp, int32 gold, int32 hp)
	: _class(playerClass), _exp(exp), _gold(gold), _hp(hp)
{
}

void Player::SetPlayerInfo(int32 playerClass, int32 exp, int32 gold, int32 hp)
{
	_class = playerClass;
	_exp = exp;
	_gold = gold;
	_hp = hp;
}
