#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "Protocol.pb.h"
#include "PacketHandler.h"
#include "GameSession.h"

RoomRef GRoom = std::make_shared<Room>();

// Village
Room::Room()
{

}

Room::~Room()
{

}

void Room::EnterRoom(PlayerRef player)
{
	// 새로 들어온 플레이어의 위치를 기존 플레이어들에게 전송
	{
		Protocol::S_SpawnPacket spawnPkt;
		Protocol::PlayerState* playerState = spawnPkt.add_playerstates();

		Protocol::PlayerInfo* playerInfo = playerState->mutable_playerinfo();
		playerInfo->set_playerid(player->_playerId);
		playerInfo->set_playerclass(static_cast<Protocol::ClassType>(player->_class));

		Protocol::Vector3* pos = playerState->mutable_playerpos();
		pos->set_x(100.0f);
		pos->set_y(200.0f);
		pos->set_z(300.0f);

		Protocol::Rotator3* rot = playerState->mutable_playerrot();
		rot->set_pitch(player->_playerRot.pitch());
		rot->set_yaw(player->_playerRot.yaw());
		rot->set_roll(player->_playerRot.roll());

		SendBufferRef spawnBuffer =  PacketHandler::MakeSendBuffer(spawnPkt);

		Broadcast(spawnBuffer, player->_playerId);
	}

	// 기존 플레이어들의 위치를 새로 들어온 플레이어에게 전송
	{
		Protocol::S_SpawnPacket spawnPkt;
		for (auto& userInfo : _players)
		{
			PlayerRef user = userInfo.second;
			Protocol::PlayerState* playerState = spawnPkt.add_playerstates();
			Protocol::PlayerInfo* playerInfo = playerState->mutable_playerinfo();
			playerInfo->set_playerid(user->_playerId);
			playerInfo->set_playerclass(static_cast<Protocol::ClassType>(user->_class));

			Protocol::Vector3* pos = playerState->mutable_playerpos();
			pos->set_x(100.0f);
			pos->set_y(200.0f);
			pos->set_z(300.0f);

			Protocol::Rotator3* rot = playerState->mutable_playerrot();
			rot->set_pitch(user->_playerRot.pitch());
			rot->set_yaw(user->_playerRot.yaw());
			rot->set_roll(user->_playerRot.roll());
		}
			SendBufferRef spawnBuffer = PacketHandler::MakeSendBuffer(spawnPkt);

			if (auto session = player->_ownerSession.lock())
				session->Send(spawnBuffer);
	}
}

void Room::LeaveRoom(PlayerRef player)
{
}

bool Room::HandleEnterPlayer(PlayerRef player)
{
	EnterRoom(player);
	return true;
}

bool Room::HandleLeavePlayer(PlayerRef player)
{
	return true;
}

void Room::HandleMove()
{
}

void Room::AddPlayer(PlayerRef player)
{
	_players.insert(std::make_pair(player->_playerId, player));

}
void Room::Broadcast(SendBufferRef sendBuffer, uint64 exceptId)
{
	for (auto& item : _players)
	{
		PlayerRef player = std::dynamic_pointer_cast<Player>(item.second);
		if (player == nullptr)
			continue;
		if (player->_playerId == exceptId)
			continue;
		if (GameSessionRef session = player->_ownerSession.lock())
		{
			session->Send(sendBuffer);
		}
	}
}
