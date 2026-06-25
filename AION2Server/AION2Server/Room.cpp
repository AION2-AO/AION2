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
		std::cout << "New Player: " << player->GetId() << "\n";

		Protocol::S_SpawnPacket spawnPkt;
		Protocol::PlayerState* playerState = spawnPkt.add_playerstates();

		playerState->set_playerid(player->_playerId);
		playerState->set_playerclass(static_cast<Protocol::ClassType>(player->_class));

		Protocol::Vector3* pos = playerState->mutable_playerlocation();
		pos->set_x(player->GetPos().x());
		pos->set_y(player->GetPos().y());
		pos->set_z(player->GetPos().z());

		Protocol::Rotator3* rot = playerState->mutable_playerrotation();
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
			std::cout << "Ex Player: " << userInfo.first << "To New Player" << player->GetId() << "\n";

			PlayerRef user = userInfo.second;			
			Protocol::PlayerState* playerState = spawnPkt.add_playerstates();
			playerState->set_playerid(user->_playerId);
			playerState->set_playerclass(static_cast<Protocol::ClassType>(user->_class));

			Protocol::Vector3* pos = playerState->mutable_playerlocation();
			pos->set_x(user->GetPos().x());
			pos->set_y(user->GetPos().y());
			pos->set_z(user->GetPos().z());

			Protocol::Rotator3* rot = playerState->mutable_playerrotation();
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

void Room::HandleMove(Protocol::C_MovePacket pkt, PlayerRef player)
{
	const uint64 playerId = pkt.playerid();
	const Protocol::Vector3& targetPos = pkt.playerlocation();
	const Protocol::Rotator3& targetRot = pkt.playerrotation();

	Protocol::Vector3 startPos = player->GetPos();

	player->SetPos(targetPos);
	player->SetRot(targetRot);

	Protocol::S_MovePacket movePkt;

	movePkt.set_playerid(playerId);
	std::cout << "HandleMove - Move Player: " << playerId << "\n";

	Protocol::Vector3* loc = movePkt.mutable_playerlocation();
	loc->set_x(targetPos.x());
	loc->set_y(targetPos.y());
	loc->set_z(targetPos.z());

	Protocol::Vector3* vel = movePkt.mutable_playervelocity();
	vel->set_x(pkt.playervelocity().x());
	vel->set_y(pkt.playervelocity().y());
	vel->set_z(pkt.playervelocity().z());

	Protocol::Rotator3* rot = movePkt.mutable_playerrotation();
	rot->set_pitch(targetRot.pitch());
	rot->set_yaw(targetRot.yaw());
	rot->set_roll(targetRot.roll());

	SendBufferRef sendBuffer = PacketHandler::MakeSendBuffer(movePkt);
	Broadcast(sendBuffer, player->_playerId);
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
