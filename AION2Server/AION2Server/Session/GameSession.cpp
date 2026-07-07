#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "PacketHandler.h"
#include "Room.h"
#include "Dungeon.h"

#include "uuids.h"
#include <sstream>
#include <random>
#include <iomanip>

void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));

	if (_player)
	{
		std::cout << "Disconnected Player" << std::endl;
		GRoom->DoAsync(&Room::HandleLeavePlayer, _player);
		GDungeonWaitingRoom->DoAsync(&DungeonWaitingRoom::HandleLeaveWaitingRoom, _player);
	}
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{
}

const std::string GameSession::GetToken()
{
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<uint64> dis;

	std::stringstream ss;
	ss << std::hex << std::setfill('0') << std::setw(16) << dis(gen) << std::setw(16) << dis(gen);
	_token = ss.str();
	
	return ss.str();

}
