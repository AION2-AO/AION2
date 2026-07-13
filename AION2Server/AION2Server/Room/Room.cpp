#include "pch.h"
#include <cmath>
#include "Room.h"
#include "Player.h"
#include "Protocol.pb.h"
#include "PacketHandler.h"
#include "GameSession.h"
#include "DB/RedisManager.h"
#include "DB/DBBind.h"
#include "DB/DBConnection.h"
#include "DB/DBConnectionPool.h"

RoomRef GRoom = std::make_shared<Room>();

// Village
Room::Room()
{
	_currentPvpState = Protocol::PVP_STATE_INACTIVE;
	_currentStateStartTime = ::GetTickCount64();
	_currentStateDuration = 20; // 최초 타이머가 20초(20000ms) 후에 동작하므로 20초로 설정
}

Room::~Room()
{

}

void Room::EnterRoom(PlayerRef player)
{
	if (player == nullptr)
		return;

	// 세션 스레드에서 미리 조회한 사망 페널티 정보를 이용해 타이머 등록
	if (player->_deathPenaltyTtl > 0)
	{
		player->_isInvulnerable = true;
		player->_canAttack = false;

		// 남은 시간만큼 대기 후 페널티 해제하는 타이머 등록
		uint64 remainingMs = static_cast<uint64>(player->_deathPenaltyTtl) * 1000;
		DoTimer(remainingMs, &Room::ClearDeathPenalty, player->GetId());
		std::cout << "Player " << player->GetName() << " entered with death penalty. Remaining: " << player->_deathPenaltyTtl << "s\n";
		player->_deathPenaltyTtl = 0; // 사용 후 초기화
	}

	// 새로 들어온 플레이어의 위치를 기존 플레이어들에게 전송
	{
		Protocol::S_SpawnPacket spawnPkt;
		Protocol::PlayerState* playerState = spawnPkt.add_playerstates();

		playerState->set_playerid(player->_playerId);
		playerState->set_playerclass(static_cast<Protocol::ClassType>(player->_class));
		playerState->set_playername(player->GetName());

		Protocol::Vector3* pos = playerState->mutable_playerlocation();
		pos->set_x(player->GetPos().x());
		pos->set_y(player->GetPos().y());
		pos->set_z(player->GetPos().z());

		Protocol::Rotator3* rot = playerState->mutable_playerrotation();
		rot->set_pitch(player->_playerRot.pitch());
		rot->set_yaw(player->_playerRot.yaw());
		rot->set_roll(player->_playerRot.roll());



		SendBufferRef spawnBuffer = PacketHandler::MakeSendBuffer(spawnPkt);
		Broadcast(spawnBuffer, player->_playerId);
	}

	// 기존 플레이어들의 위치를 새로 들어온 플레이어에게 전송
	{
		Protocol::S_SpawnPacket spawnPkt;
		for (auto& userInfo : _players)
		{
			PlayerRef user = userInfo.second;
			Protocol::PlayerState* playerState = spawnPkt.add_playerstates();
			playerState->set_playerid(user->_playerId);
			playerState->set_playerclass(static_cast<Protocol::ClassType>(user->_class));
			playerState->set_playername(user->GetName());

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

	// PVP 상태 동기화 패킷 전송
	{
		uint64 elapsedMs = ::GetTickCount64() - _currentStateStartTime;
		int32 remainingSeconds = 0;
		uint64 durationMs = static_cast<uint64>(_currentStateDuration) * 1000;
		if (elapsedMs < durationMs)
		{
			remainingSeconds = static_cast<int32>((durationMs - elapsedMs) / 1000);
		}

		Protocol::S_PvpStatePacket statePacket;
		statePacket.set_state(_currentPvpState);
		statePacket.set_remainingseconds(remainingSeconds);

		SendBufferRef stateBuffer = PacketHandler::MakeSendBuffer(statePacket);
		if (auto session = player->_ownerSession.lock())
			session->Send(stateBuffer);
	}
}

void Room::LeaveRoom(PlayerRef player)
{
	if (player == nullptr)
		return;

	_players.erase(player->_playerId);
	std::cout << "Player Leave Room: " << player->GetId() << "\n";
}

void Room::UpdatePvpTimer()
{
	// 1. 현재 인덱스에 해당하는 PVP 설정 가져오기
	const FPvpStateConfig& currentConfig = PvpStateSequence[CurrentStateIndex];

	_currentPvpState = currentConfig.State;
	_currentStateStartTime = ::GetTickCount64();
	_currentStateDuration = currentConfig.Duration;

	// 2. 현재 상태 패킷 생성 
	Protocol::S_PvpStatePacket statePacket;
	statePacket.set_state(_currentPvpState);
	statePacket.set_remainingseconds(_currentStateDuration);

	SendBufferRef stateBuffer = PacketHandler::MakeSendBuffer(statePacket);
	Broadcast(stateBuffer, -1);

	std::cout << "[PVP State Changed] State: " << _currentPvpState
		<< ", Duration: " << _currentStateDuration << "s\n";

	// 3. 다음 상태의 인덱스로 미리 변경 
	CurrentStateIndex = (CurrentStateIndex + 1) % NumPvpStates;

	// 4. 방금 전송한 상태의 지속 시간(초)이 끝난 후에 다시 이 함수가 실행되도록 타이머 등록
	uint64 delayMs = static_cast<uint64>(_currentStateDuration) * 1000;
	DoTimer(delayMs, &Room::UpdatePvpTimer);
}

void Room::BroadcastPvpState()
{
	Protocol::S_PvpStatePacket statePacket;

	statePacket.set_state(PvpStateSequence[CurrentStateIndex].State);
	statePacket.set_remainingseconds(PvpTimerSeconds);
	SendBufferRef stateBuffer = PacketHandler::MakeSendBuffer(statePacket);
	Broadcast(stateBuffer, -1);
}
bool Room::HandleEnterPlayer(PlayerRef player)
{
	EnterRoom(player);
	return true;
}

bool Room::HandleLeavePlayer(PlayerRef player)
{
	LeaveRoom(player);

	Protocol::S_DisconnectPacket disconnectPkt;
	disconnectPkt.set_playerid(player->GetId());
	SendBufferRef disconnectBuffer = PacketHandler::MakeSendBuffer(disconnectPkt);
	Broadcast(disconnectBuffer, player->GetId());
	return true;
}

void Room::HandleMove(Protocol::C_MovePacket pkt, PlayerRef player)
{
	const uint64 playerId = player->GetId();
	const Protocol::Vector3& targetPos = pkt.playerlocation();
	const Protocol::Rotator3& targetRot = pkt.playerrotation();

	player->SetPos(targetPos);
	player->SetRot(targetRot);

	Protocol::S_MovePacket movePkt;

	movePkt.set_playerid(playerId);

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

void Room::HandleSavePlayerHp()
{
	DoTimer(18000, &Room::HandleSavePlayerHp);
	std::thread doThread([]() {
		auto pendingUpdates = GRedisManager.GetPendingHpUpdate();
		if (pendingUpdates.empty()) return;

		DBConnection* dbConnect = GDBConnectionPool->Pop();
		if (dbConnect == nullptr) return;

		std::vector<std::string> successIds;
		for (const auto& pair : pendingUpdates)
		{
			DBBind<2, 0>dbBind(*dbConnect, L"{CALL sp_UpdateHp(? ,?)}");
			int32 hp = pair.second;
			WCHAR widBuf[51] = { 0, };
			::mbstowcs_s(nullptr, widBuf, 51, pair.first.c_str(), _TRUNCATE);

			dbBind.BindParam(0, hp);
			dbBind.BindParam(1, widBuf);
			std::cout << "HandleSavePlayerHp:" << pair.first << ", " << hp <<"\n";

			if (dbBind.Execute())
			{
				successIds.push_back(pair.first);
			}
		}
		GDBConnectionPool->Push(dbConnect);

		GRedisManager.ClearPendingHpUpdate(successIds);
		});
	doThread.detach();
}

void Room::HandleChat(Protocol::S_ChatPacket pkt)
{
	SendBufferRef chatBuffer = PacketHandler::MakeSendBuffer(pkt);
	Broadcast(chatBuffer, -1);
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

void Room::HandlePlayerDash(Protocol::C_DashPacket pkt, PlayerRef player)
{
	uint64 playerId = player->GetId();

	Protocol::S_DashPacket dashPkt;
	dashPkt.set_playerid(player->_playerId);
	const Protocol::Vector3& targetPos = pkt.playerlocation();
	const Protocol::Rotator3& targetRot = pkt.playerrotation();

	player->SetPos(targetPos);
	player->SetRot(targetRot);

	dashPkt.set_playerid(playerId);

	Protocol::Vector3* loc = dashPkt.mutable_playerlocation();
	loc->set_x(targetPos.x());
	loc->set_y(targetPos.y());
	loc->set_z(targetPos.z());

	Protocol::Vector3* vel = dashPkt.mutable_playervelocity();
	vel->set_x(pkt.playervelocity().x());
	vel->set_y(pkt.playervelocity().y());
	vel->set_z(pkt.playervelocity().z());

	Protocol::Rotator3* rot = dashPkt.mutable_playerrotation();
	rot->set_pitch(targetRot.pitch());
	rot->set_yaw(targetRot.yaw());
	rot->set_roll(targetRot.roll());

	SendBufferRef sendBuffer = PacketHandler::MakeSendBuffer(dashPkt);
	Broadcast(sendBuffer, player->_playerId);
}

void Room::HandleAttack(Protocol::C_AttackPacket pkt, PlayerRef player)
{
	if (player == nullptr)
		return;

	// 공격 불가 상태 체크 (사망 페널티)
	if (!player->_canAttack)
	{
		std::cout << "Player " << player->GetName() << " cannot attack (Death Penalty active).\n";
		return;
	}

	uint64 attackerId = player->GetId();
	uint64 targetId = pkt.targetid();
	int32 skillId = pkt.skillid();
	int32 damage = pkt.damage();

	PlayerRef target = nullptr;
	if (targetId != 0)
	{
		auto it = _players.find(targetId);
		if (it != _players.end())
		{
			target = it->second;
		}
	}

	int32 nextHp = 0;
	bool isDead = false;

	if (target != nullptr)
	{
		const auto& playerPos = player->GetPos();
		const auto& targetPos = target->GetPos();

		float dist = std::sqrt(
			std::pow(targetPos.x() - playerPos.x(), 2) +
			std::pow(targetPos.y() - playerPos.y(), 2) +
			std::pow(targetPos.z() - playerPos.z(), 2)
		);

		const float MAX_ATTACK_DISTANCE = 250.f; 
		if (dist > MAX_ATTACK_DISTANCE)
		{
			std::cout << "Attack verification failed: target too far (distance: " << dist << ").\n";
			return;
		}

		// 방향 검증
		const float playerYaw = player->GetRot().yaw();
		const float PI = 3.1415926535f;
		float rad = playerYaw * (PI / 180.0f);

		// 플레이어 전방 바라보는 2D 방향 벡터
		float forwardX = std::cos(rad);
		float forwardY = std::sin(rad);

		// 플레이어에서 타겟으로 향하는 2D 방향 벡터
		float dirX = targetPos.x() - playerPos.x();
		float dirY = targetPos.y() - playerPos.y();
		float dirDist = std::sqrt(dirX * dirX + dirY * dirY);

		if (dirDist > 0.01f)
		{
			float dirXNorm = dirX / dirDist;
			float dirYNorm = dirY / dirDist;

			float dot = forwardX * dirXNorm + forwardY * dirYNorm;

			const float MIN_ATTACK_DOT = 0.5f;
			if (dot < MIN_ATTACK_DOT)
			{
				std::cout << "Attack verification failed: target not in front cone (dot: " << dot << ").\n";
				return;
			}
		}

		if (target->_isInvulnerable)
		{
			std::cout << "Target " << target->GetName() << " is invulnerable.\n";
			return;
		}

		// 데미지 적용 및 사망 처리
		if (damage <= 0)
		{
			damage = 10;
		}

		nextHp = target->GetHp() - damage;
		if (nextHp < 0)
			nextHp = 0;

		target->SetHp(nextHp);

		// Redis에 변경된 HP 정보 업데이트 (비동기 처리로 Room 스레드 블로킹 방지)
		std::string targetName = target->GetName();
		std::thread([targetName, nextHp]() {
			GRedisManager.UpdatePlayerHp(targetName, nextHp);
		}).detach();

		// 사망 시 페널티 적용 (무적 상태 & 공격 불가 5분)
		if (nextHp == 0)
		{
			target->_isInvulnerable = true;
			target->_canAttack = false;

			// Redis 저장은 별도 스레드로 비동기 처리하여 Room 스레드 블로킹 방지
			std::thread([targetName]() {
				GRedisManager.SetDeathPenalty(targetName, 300);
			}).detach();

			// 5분 후 페널티 해제하는 타이머 등록
			DoTimer(300000, &Room::ClearDeathPenalty, target->GetId());
			std::cout << "Player " << target->GetName() << " died. Invulnerability applied for 5 minutes.\n";
		}
		isDead = (nextHp <= 0);
	}
	else
	{
		damage = 0;
	}

	Protocol::S_AttackResultPacket resultPkt;
	resultPkt.set_attackerid(attackerId);
	resultPkt.set_targetid(target ? targetId : 0);
	resultPkt.set_damage(damage);
	resultPkt.set_targethp(target ? target->GetHp() : 0);
	resultPkt.set_isdead(isDead);
	resultPkt.set_skillid(skillId);
	SendBufferRef sendBuffer = PacketHandler::MakeSendBuffer(resultPkt);
	Broadcast(sendBuffer, -1); 
}

void Room::ClearDeathPenalty(uint64 playerId)
{
	auto it = _players.find(playerId);
	if (it == _players.end())
		return;

	PlayerRef player = it->second;
	if (player == nullptr)
		return;

	player->_isInvulnerable = false;
	player->_canAttack = true;
	std::cout << "Player " << player->GetName() << " death penalty expired.\n";
}

void Room::HandlePlayerJump(Protocol::C_JumpPacket pkt, PlayerRef player)
{
	Protocol::S_JumpPacket jumpPkt;
	jumpPkt.set_playerid(pkt.playerid());
	jumpPkt.set_isgliding(pkt.isgliding());

	SendBufferRef sendBuffer = PacketHandler::MakeSendBuffer(jumpPkt);
	Broadcast(sendBuffer, player->_playerId);
}

