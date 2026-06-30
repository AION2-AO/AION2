#include "pch.h"
#include "Dungeon.h"
#include "Player.h"
#include "PacketHandler.h"
#include "Protocol.pb.h"
#include "GameSession.h"
#include "DedicatedSession.h"
#include "DediSessionManager.h"
#include "GameSessionManager.h"
#include <algorithm>
#include <iostream>

DungeonWaitingRoomRef GDungeonWaitingRoom = std::make_shared<DungeonWaitingRoom>();

// ==============
// Dungeon
// ==============

Dungeon::Dungeon(int32 dungeonId, PlayerRef leader)
	: _dungeonId(dungeonId), _leader(leader), _status(Protocol::RoomStatus::RECRUITING)
{
}

bool Dungeon::AddMember(PlayerRef player)
{
	if (IsFull()) return false;
	_members.push_back(player);
	return true;
}

bool Dungeon::RemoveMember(PlayerRef player)
{
	auto it = std::find(_members.begin(), _members.end(), player);
	if (it != _members.end())
	{
		_members.erase(it);
		return true;
	}
	return false;
}

bool Dungeon::IsFull() const
{
	return _members.size() >= MAX_MEMBERS;
}

int32 Dungeon::GetFreeIndex() const
{
	return static_cast<int32>(_members.size() + 1);
}


void Dungeon::Broadcast(SendBufferRef sendBuffer)
{
	if (_leader)
	{
		if (auto session = _leader->_ownerSession.lock())
		{
			session->Send(sendBuffer);
		}
	}

	for (auto& member : _members)
	{
		if (member)
		{
			if (auto session = member->_ownerSession.lock())
			{
				session->Send(sendBuffer);
			}
		}
	}
}

Protocol::DungeonInfo Dungeon::ToProto()
{
	Protocol::DungeonInfo info;
	info.set_dungeonid(_dungeonId);
	info.set_status(_status);

	if (_leader)
	{
		Protocol::DungeonPlayerInfo* leaderProto = info.mutable_leaderinfo();
		leaderProto->set_memberid(_leader->GetId());
		leaderProto->set_membername(_leader->GetName());
		leaderProto->set_memberclass(_leader->GetClass());
		leaderProto->set_isready(true);
		leaderProto->set_index(0);
	}

	for (size_t i = 0; i < _members.size(); ++i)
	{
		if (PlayerRef member = _members[i])
		{
			Protocol::DungeonPlayerInfo* memberProto = info.add_members();
			memberProto->set_memberid(member->GetId());
			memberProto->set_membername(member->GetName());
			memberProto->set_memberclass(member->GetClass());
			memberProto->set_isready(false);
			memberProto->set_index(static_cast<int32>(i + 1));
		}
	}
	return info;
}

// =====================
// DungeonWaitingRoom
// =====================

DungeonWaitingRoom::DungeonWaitingRoom()
{
}

DungeonWaitingRoom::~DungeonWaitingRoom()
{
}

void DungeonWaitingRoom::HandleWaitingRoom(PlayerRef player)
{
	_waitingPlayers[player->GetId()] = player;

	Protocol::S_DungeonWaitingRoomEnterPacket waitPkt;
	for (const auto& [dungeonId, dungeon] : _dungeons)
	{
		Protocol::DungeonInfo* dungeonInfo = waitPkt.add_dungeoninfos();
		dungeonInfo->CopyFrom(dungeon->ToProto());
	}

	SendBufferRef dungeonBuffer = PacketHandler::MakeSendBuffer(waitPkt);
	if (auto session = player->_ownerSession.lock())
		session->Send(dungeonBuffer);
}

void DungeonWaitingRoom::HandleLeaveWaitingRoom(PlayerRef player)
{
	if (player == nullptr) return;
	_waitingPlayers.erase(player->GetId());
}

void DungeonWaitingRoom::HandleCreateDungeon(PlayerRef player)
{
	if (_dungeons.size() >= MAX_DUNGEON) return;

	DedicatedSessionRef dedi = GDediSessionManager.GetFreeDediSession();
	if (dedi == nullptr)
	{
		std::cout << "Full Dedi Servers!" << std::endl;
		return;
	}

	int32 dungeonId = GetFreeDungeonId();
	DungeonRef dungeon = std::make_shared<Dungeon>(dungeonId, player);
	_dungeons[dungeonId] = dungeon;

	dedi->SetUsing(true);
	dungeon->SetDediSession(dedi);

	Protocol::S_DungeonCreatePacket createPkt;
	createPkt.mutable_dungeoninfo()->CopyFrom(dungeon->ToProto());

	SendBufferRef createdungeonBuffer = PacketHandler::MakeSendBuffer(createPkt);
	WaitingRoomBroadcast(createdungeonBuffer);
}

void DungeonWaitingRoom::HandleEnterDungeon(PlayerRef player)
{
	DungeonRef targetDungeon = nullptr;
	for (auto& [dungeonId, dungeon] : _dungeons)
	{
		if (dungeon->GetStatus() == Protocol::RoomStatus::RECRUITING && !dungeon->IsFull())
		{
			targetDungeon = dungeon;
			break;
		}
	}

	if (targetDungeon == nullptr) return;

	if (targetDungeon->AddMember(player))
	{
		Protocol::S_DungeonEnterPacket enterPacket;
		enterPacket.set_dungeonid(targetDungeon->GetId());

		Protocol::DungeonPlayerInfo* enterPlayerProto = enterPacket.mutable_enterplayer();
		enterPlayerProto->set_memberid(player->GetId());
		enterPlayerProto->set_membername(player->GetName());
		enterPlayerProto->set_memberclass(player->GetClass());
		enterPlayerProto->set_isready(true);
		enterPlayerProto->set_index(static_cast<int32>(targetDungeon->GetMembers().size()));

		SendBufferRef enterBuffer = PacketHandler::MakeSendBuffer(enterPacket);
		WaitingRoomBroadcast(enterBuffer);
	}
}

void DungeonWaitingRoom::HandleReadyPacket(PlayerRef player, int32 dungeonId)
{
	player->SetReady(true);

	auto it = _dungeons.find(dungeonId);
	if (it == _dungeons.end()) return;
	DungeonRef dungeon = it->second;

	Protocol::S_DungeonReadyPacket readyPacket;
	readyPacket.set_dungeonid(dungeonId);
	readyPacket.set_playerid(player->GetId());

	SendBufferRef readyBuffer = PacketHandler::MakeSendBuffer(readyPacket);
	dungeon->Broadcast(readyBuffer);
}

void DungeonWaitingRoom::HandleDungeonStart(PlayerRef player, int32 dungeonId)
{
	auto it = _dungeons.find(dungeonId);
	if (it == _dungeons.end()) return;

	DungeonRef dungeon = it->second;
	if (dungeon->GetLeader() != player) return;

	auto dedi = dungeon->GetDediSession();
	dungeon->SetStatus(Protocol::RoomStatus::IN_PROGRESS);

	std::string dediIp = dedi->GetIP();
	int32 port = dedi->GetPort();

	Protocol::S_DungeonStartPacket startPkt;
	startPkt.set_dungeonid(dungeonId);
	startPkt.set_dungeonip(dediIp);
	startPkt.set_port(port);

	SendBufferRef startBuffer = PacketHandler::MakeSendBuffer(startPkt);
	dungeon->Broadcast(startBuffer);

	for (auto& member : dungeon->GetMembers())
	{
		if (!member->GetReady())
		{
			// Todo, 스타트 실패 패킷 보내기
			return;
		}
	}

	HandleLeaveWaitingRoom(dungeon->GetLeader());
	for (auto& member : dungeon->GetMembers())
	{
		HandleLeaveWaitingRoom(member);
	}

	_dungeons.erase(dungeonId);
	_freeDungeonIds.insert(dungeonId);
}

void DungeonWaitingRoom::HandleDungeonExit(int32 dungeonId)
{
	auto it = _dungeons.find(dungeonId);
	if (it == _dungeons.end()) return;

	DungeonRef dungeon = it->second;
	if (!dungeon) return;

	auto dedi = dungeon->GetDediSession();
	dedi->SetUsing(false);
	dungeon->SetStatus(Protocol::RoomStatus::RECRUITING);

	for (auto member : dungeon->GetMembers())
	{
		member->SetReady(false);
		dungeon->RemoveMember(member);
	}
}

void DungeonWaitingRoom::WaitingRoomBroadcast(SendBufferRef sendBuffer, uint64 exceptId)
{
	for (auto& item : _waitingPlayers)
	{
		PlayerRef player = item.second;
		if (player == nullptr || player->GetId() == exceptId) continue;

		if (auto session = player->_ownerSession.lock())
		{
			session->Send(sendBuffer);
		}
	}
}

int32 DungeonWaitingRoom::GetFreeDungeonId()
{
	if (!_freeDungeonIds.empty())
	{
		auto it = _freeDungeonIds.begin();
		int32 id = *it;
		_freeDungeonIds.erase(it);
		return id;
	}
	return _nextDungeonId++;
}
