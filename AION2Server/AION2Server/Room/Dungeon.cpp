#include "pch.h"
#include "Dungeon.h"
#include "Player.h"
#include "PacketHandler.h"
#include "Protocol.pb.h"
#include "GameSession.h"
#include "DedicatedSession.h"
#include "DediSessionManager.h"
#include "GameSessionManager.h"
#include "DB/DBConnectionPool.h"
#include "DB/DBBind.h"

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
	for (auto& m : _members)
	{
		if (m->GetId() == player->GetId())
			return false;
	}
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

void Dungeon::ResetDungeon()
{
	_members.clear();
	_leader = nullptr;
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
			memberProto->set_isready(member->GetReady());
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

	for (auto it = _dungeons.begin(); it != _dungeons.end(); )
	{
		DungeonRef dungeon = it->second;
		if (dungeon == nullptr)
		{
			it = _dungeons.erase(it);
			continue;
		}

		if (dungeon->GetStatus() == Protocol::RoomStatus::RECRUITING)
		{
			if (dungeon->GetLeader() == player)
			{
				DediSessionRef dedi = dungeon->GetDediSession();
				if (dedi)
				{
					dedi->SetUsing(false);
					dedi->_dungeon = nullptr;
				}

				for (auto member : dungeon->GetMembers())
				{
					if (member)
					{
						member->SetReady(false);
					}
				}

				Protocol::S_DungeonExitPacket exitPkt;
				exitPkt.set_playerid(player->GetId());
				exitPkt.mutable_dungeoninfo()->CopyFrom(dungeon->ToProto());
				SendBufferRef exitBuffer = PacketHandler::MakeSendBuffer(exitPkt);
				WaitingRoomBroadcast(exitBuffer, -1);

				int32 dungeonId = dungeon->GetId();
				_freeDungeonIds.insert(dungeonId);
				it = _dungeons.erase(it);
				continue;
			}
			else
			{
				if (dungeon->RemoveMember(player))
				{
					Protocol::S_DungeonExitPacket exitPkt;
					exitPkt.set_playerid(player->GetId());
					exitPkt.mutable_dungeoninfo()->CopyFrom(dungeon->ToProto());
					SendBufferRef exitBuffer = PacketHandler::MakeSendBuffer(exitPkt);
					WaitingRoomBroadcast(exitBuffer, -1);
				}
			}
		}
		++it;
	}
}

void DungeonWaitingRoom::HandleCreateDungeon(PlayerRef player)
{
	if (_dungeons.size() >= MAX_DUNGEON) return;

	DedicatedSessionRef dedi = GDediSessionManager.GetFreeDediSession();
	if (dedi == nullptr)
	{
		HandleFailDungeon(player, Protocol::DungeonFailReason::FullDungeon);
		std::cout << "Full Dedi Servers!" << std::endl;
		return;
	}

	int32 dungeonId = GetFreeDungeonId();
	DungeonRef dungeon = std::make_shared<Dungeon>(dungeonId, player);
	_dungeons[dungeonId] = dungeon;

	dedi->_dungeon = dungeon;
	dedi->SetUsing(true);
	dungeon->SetDediSession(dedi);

	Protocol::S_DungeonCreatePacket createPkt;
	createPkt.mutable_dungeoninfo()->CopyFrom(dungeon->ToProto());

	SendBufferRef createdungeonBuffer = PacketHandler::MakeSendBuffer(createPkt);
	dedi->Send(createdungeonBuffer);
	WaitingRoomBroadcast(createdungeonBuffer);
}

void DungeonWaitingRoom::HandleEnterDungeon(PlayerRef player, int32 inDungeonId)
{
	DungeonRef targetDungeon = nullptr;
	auto IsAvailable = [](const DungeonRef& dungeon) {
		return dungeon && dungeon->GetStatus() == Protocol::RoomStatus::RECRUITING && !dungeon->IsFull();
		};
	if (inDungeonId <= 0)
	{
		for (auto& [dungeonId, dungeon] : _dungeons)
		{
			if (IsAvailable(dungeon))
			{
				targetDungeon = dungeon;
				break;
			}
			else
			{
				HandleCreateDungeon(player);
				return;
			}
		}
	}

	else
	{
		auto it = _dungeons.find(inDungeonId);
		if (it != _dungeons.end() && IsAvailable(it->second))
		{
			targetDungeon = it->second;
		}
		else
		{
			HandleCreateDungeon(player);
			return;
		}
	}
	if (targetDungeon == nullptr) return;

	if (targetDungeon->AddMember(player))
	{
		Protocol::S_DungeonEnterPacket enterPacket;
		enterPacket.set_dungeonid(targetDungeon->GetId());
		if (CheckAlreadyIn(player->GetId(), targetDungeon)) return;
		enterPacket.mutable_dungeoninfo()->CopyFrom(targetDungeon->ToProto());

		SendBufferRef enterBuffer = PacketHandler::MakeSendBuffer(enterPacket);
		WaitingRoomBroadcast(enterBuffer);
	}
}

void DungeonWaitingRoom::HandleReadyPacket(PlayerRef player, int32 dungeonId)
{
	if (player->GetReady())
	{
		player->SetReady(false);
	}
	else
	{
		player->SetReady(true);
	}

	auto it = _dungeons.find(dungeonId);
	if (it == _dungeons.end()) return;
	DungeonRef dungeon = it->second;

	Protocol::S_DungeonReadyPacket readyPacket;
	readyPacket.set_dungeonid(dungeonId);
	readyPacket.set_playerid(player->GetId());
	readyPacket.set_isready(player->GetReady());
	SendBufferRef readyBuffer = PacketHandler::MakeSendBuffer(readyPacket);
	dungeon->Broadcast(readyBuffer);
}

void DungeonWaitingRoom::HandleExitPacket(PlayerRef player, int32 dungeonId)
{
	auto it = _dungeons.find(dungeonId);
	if (it == _dungeons.end()) return;
	DungeonRef dungeon = it->second;

	if (player == dungeon->GetLeader())
	{
		DediSessionRef dedi = dungeon->GetDediSession();
		if (dedi)
		{
			dedi->SetUsing(false);
			dedi->_dungeon = nullptr;
		}
		for (auto member : dungeon->GetMembers())
		{
			member->SetReady(false);
		}
		_dungeons.erase(dungeonId);
		_freeDungeonIds.insert(dungeonId);
	}
	else
	{
		dungeon->RemoveMember(player);
	}

	Protocol::S_DungeonExitPacket exitPkt;
	exitPkt.set_playerid(player->GetId());
	exitPkt.mutable_dungeoninfo()->CopyFrom(dungeon->ToProto());
	SendBufferRef exitBuffer = PacketHandler::MakeSendBuffer(exitPkt);
	WaitingRoomBroadcast(exitBuffer, -1);
}

void DungeonWaitingRoom::HandleFailDungeon(PlayerRef player, Protocol::DungeonFailReason reason)
{
	Protocol::S_DungeonFailPacket failPkt;
	failPkt.set_reason(reason);
	SendBufferRef failBuffer = PacketHandler::MakeSendBuffer(failPkt);
	auto session = player->_ownerSession;
	if (auto session = player->_ownerSession.lock())
	{
		session->Send(failBuffer);
	}
}

bool DungeonWaitingRoom::CheckMembersReady(DungeonRef dungeon)
{
	for (auto& member : dungeon->GetMembers())
	{
		if (!member->GetReady())
			return false;
	}
	return true;
}

bool DungeonWaitingRoom::CheckAlreadyIn(uint64 playerId, DungeonRef dungeon)
{
	for (auto& p : dungeon->GetMembers())
	{
		if (p->GetId() == playerId)
			return false;
	}
	return true;
}

void DungeonWaitingRoom::HandleDungeonToken(DungeonRef dungeon)
{
	Protocol::S_DungeonStartDediPacket pkt;
	Protocol::DediDungeonInfo* dediInfo;
	for (auto member : dungeon->GetMembers())
	{
		dediInfo = pkt.add_preplayersinfos();
		auto session = member->_ownerSession;
		if (auto session = member->_ownerSession.lock())
		{	
			int32 memberId = member->GetId();

			dediInfo->set_clienttoken(session->GetToken());
			dediInfo->set_clientid(memberId);
			dediInfo->set_clientname(member->GetName());
			dediInfo->set_clientclass(member->GetClass());
			dediInfo->set_clienthp(member->GetHp());		

			DBConnection* dbConnect = GDBConnectionPool->Pop();
			DBBind<1, 4> dbBind(*dbConnect, L"{CALL sp_GetItems(?)}");
			dbBind.BindParam(0, memberId);

			int32 itemInstanceId = 0;
			int32 itemTemplateId = 0;
			int32 slotIndex = 0;
			int32 itemCount = 0;

			std::wcout.imbue(std::locale("kor"));

			dbBind.BindCol(0, itemInstanceId);
			dbBind.BindCol(1, itemTemplateId);
			dbBind.BindCol(2, slotIndex);
			dbBind.BindCol(3, itemCount);

			if (dbBind.Execute())
			{
				if (dbBind.Fetch())
				{
					if (itemInstanceId != 0)
					{
						Protocol::ItemData* item = dediInfo->add_playeritems();

						item->set_iteminstancedid(itemInstanceId);
						item->set_itemtemplateid(itemTemplateId);
						item->set_slotindex(slotIndex);
						item->set_count(itemCount);
					}
				}
			}
			GDBConnectionPool->Push(dbConnect);

		}
	}

	{
		auto leader = dungeon->GetLeader();
		auto leaderSession = leader->_ownerSession;
		if (auto leaderSession = leader->_ownerSession.lock())
		{
			int32 leaderId = leader->GetId();
			dediInfo = pkt.add_preplayersinfos();
			dediInfo->set_clienttoken(leaderSession->GetToken());
			dediInfo->set_clientid(leaderId);
			dediInfo->set_clientname(leader->GetName());
			dediInfo->set_clientclass(leader->GetClass());

			DBConnection* dbConnect = GDBConnectionPool->Pop();
			if (dbConnect == nullptr) return;
			DBBind<1, 4> dbBind(*dbConnect, L"{CALL sp_GetItems(?)}");
			dbBind.BindParam(0, leaderId);

			int32 itemInstanceId = 0;
			int32 itemTemplateId = 0;
			int32 slotIndex = 0;
			int32 itemCount = 0;

			std::wcout.imbue(std::locale("kor"));

			dbBind.BindCol(0, itemInstanceId);
			dbBind.BindCol(1, itemTemplateId);
			dbBind.BindCol(2, slotIndex);
			dbBind.BindCol(3, itemCount);

			if (dbBind.Execute())
			{
				if (dbBind.Fetch())
				{
					if (itemInstanceId != 0)
					{
						Protocol::ItemData* item = dediInfo->add_playeritems();

						item->set_iteminstancedid(itemInstanceId);
						item->set_itemtemplateid(itemTemplateId);
						item->set_slotindex(slotIndex);
						item->set_count(itemCount);
					}
				}
			}
			GDBConnectionPool->Push(dbConnect);
		}
	}
	SendBufferRef buffer = PacketHandler::MakeSendBuffer(pkt);
	auto dedi = dungeon->GetDediSession();
	dedi->Send(buffer);
}

void DungeonWaitingRoom::HandleDungeonStart(PlayerRef player, int32 dungeonId)
{
	auto it = _dungeons.find(dungeonId);
	if (it == _dungeons.end()) return;

	DungeonRef dungeon = it->second;

	if (!CheckMembersReady(dungeon))
	{
		HandleFailDungeon(player, Protocol::DungeonFailReason::Ready);
		return;
	}

	HandleDungeonToken(dungeon);
	StartDungeonPacket(dungeon);

	HandleLeaveWaitingRoom(dungeon->GetLeader());
	for (auto& member : dungeon->GetMembers())
	{
		HandleLeaveWaitingRoom(member);
	}
}

void DungeonWaitingRoom::HandleDungeonExit(int32 dungeonId)
{
	auto it = _dungeons.find(dungeonId);
	if (it == _dungeons.end()) return;

	DungeonRef dungeon = it->second;
	if (!dungeon) return;

	auto dedi = dungeon->GetDediSession();
	if (dedi)
	{
		dedi->SetUsing(false);
		dedi->_dungeon = nullptr;
	}
	dungeon->SetStatus(Protocol::RoomStatus::RECRUITING);

	for (auto member : dungeon->GetMembers())
	{
		member->SetReady(false);
		dungeon->RemoveMember(member);
	}
}

void DungeonWaitingRoom::HandleDungeonExitByDedi(int32 dungeonId)
{
	auto it = _dungeons.find(dungeonId);
	if (it == _dungeons.end()) return;

	DungeonRef dungeon = it->second;
	if (!dungeon) return;

	if (auto leader = dungeon->GetLeader())
	{
		leader->SetReady(false);
	}
	for (auto member : dungeon->GetMembers())
	{
		if (member)
		{
			member->SetReady(false);
		}
	}

	Protocol::S_DungeonExitPacket exitPkt;
	if (auto leader = dungeon->GetLeader())
		exitPkt.set_playerid(leader->GetId());
	exitPkt.mutable_dungeoninfo()->CopyFrom(dungeon->ToProto());
	SendBufferRef exitBuffer = PacketHandler::MakeSendBuffer(exitPkt);

	if (dungeon->GetStatus() == Protocol::RoomStatus::IN_PROGRESS)
	{
		dungeon->Broadcast(exitBuffer);
	}

	WaitingRoomBroadcast(exitBuffer, -1);

	_dungeons.erase(it);
	_freeDungeonIds.insert(dungeonId);
}

void DungeonWaitingRoom::StartDungeonPacket(DungeonRef dungeon)
{
	auto dedi = dungeon->GetDediSession();
	dungeon->SetStatus(Protocol::RoomStatus::IN_PROGRESS);
	if (!dedi) return;
	std::string dediIp = "125.133.163.145";

	int32 port = dedi->GetPort();
	std::cout << port << std::endl;
	int32 dungeonId = dungeon->GetId();
	std::string token;
	for (auto member : dungeon->GetMembers())
	{
		if (auto session = member->_ownerSession.lock())
		{
			Protocol::S_DungeonStartPacket startPkt;
			startPkt.set_dungeonid(dungeonId);
			startPkt.set_dungeonip(dediIp);
			startPkt.set_port(port);
			token = session->GetMyToken();
			startPkt.set_clienttoken(token);
			SendBufferRef startBuffer = PacketHandler::MakeSendBuffer(startPkt);
			session->Send(startBuffer);
		}
	}

	{
		auto leader = dungeon->GetLeader();
		if (auto leaderSession = leader->_ownerSession.lock())
		{
			Protocol::S_DungeonStartPacket startPkt;
			startPkt.set_dungeonid(dungeonId);
			startPkt.set_dungeonip(dediIp);
			startPkt.set_port(port);
			token = leaderSession->GetMyToken();
			startPkt.set_clienttoken(token);
			SendBufferRef startBuffer = PacketHandler::MakeSendBuffer(startPkt);
			leaderSession->Send(startBuffer);
		}
	}

}


void DungeonWaitingRoom::HandleDungeonEnd(int32 dungeonId, int32 gold)
{
	auto it = _dungeons.find(dungeonId);
	if (it == _dungeons.end()) return;

	DungeonRef dungeon = it->second;
	if (!dungeon) return;
	Protocol::S_RequestDungeonCompletePacket endPacket;
	endPacket.set_gold(gold);
	SendBufferRef endBuffer = PacketHandler::MakeSendBuffer(endPacket);

	for (auto& member : dungeon->GetMembers())
	{
		if (auto session = member->_ownerSession.lock())
		{
			session->Send(endBuffer);
		}
	}

	auto leader = dungeon->GetLeader();
	if (auto leaderSession = leader->_ownerSession.lock())
	{
		leaderSession->Send(endBuffer);
	}
	
	auto dedi = dungeon->GetDediSession();
	if (dedi)
	{
		dedi->SetUsing(false);
		dedi->_dungeon = nullptr;
	}

	_dungeons.erase(dungeonId);
	_freeDungeonIds.insert(dungeonId);
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
