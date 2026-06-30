// AOPacketHandler.cpp
#include "Manager/PacketHandlerManager.h"
#include "PacketHandler.h"
#include "Manager/AOPlayerManager.h"
#include "Game/AOGameInstance.h"
#include "UI/AOLoginUserWidget.h"
#include "UI/AODungeonEntranceWidget.h"
#include "Player/AOPlayerController.h"
#include "Manager/AOUIManager.h"
#include "AONetworkManager.h"


PacketHandlerFunc GAOPacketHandler[UINT16_MAX];

// 초기화 함수: 전역 배열에 패킷 ID와 변환 정책 함수를 바인딩함
void InitPacketHandler()
{
	for (int32 i = 1000; i < UINT16_MAX; i++)
		GAOPacketHandler[i] = &Handle_INVALID;

#if  UE_SERVER
	// 서버 핸들러

#else

#if UE_BUILD_DEVELOPMENT

	// 템플릿을 활용해 자동으로 파싱 정책을 맵핑함
	GAOPacketHandler[PKT_S_SIGNUP] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_SignUpResultPacket>(Handle_S_SIGNUP, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_FLOGIN] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_LoginFailPacket>(Handle_S_FLOGIN, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_SLOGIN] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_LoginSuccessPacket>(Handle_S_SLOGIN, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_ITEM] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_ItemDataPacket>(Handle_S_ITEM, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_SPAWN] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_SpawnPacket>(Handle_S_SPAWN, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_MOVE] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_MovePacket>(Handle_S_MOVE, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEONCREATE] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_DungeonCreatePacket>(Handle_S_CREATE, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEONENTER] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_DungeonEnterPacket>(Handle_S_ENTER, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEONREADY] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_DungeonReadyPacket>(Handle_S_READY, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEONSTART] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_DungeonStartPacket>(Handle_S_START, Mng, Buf, Len); };

#endif
#endif
}

bool Handle_INVALID(UAONetworkManager* NetworkMng, uint8* Buffer, int32 Len)
{
	return false;
}

bool Handle_S_SIGNUP(UAONetworkManager* NetworkMng, Protocol::S_SignUpResultPacket& Pkt)
{
	bool bIsSuccess = Pkt.success();
	if (bIsSuccess)
		NetworkMng->GameInstance->LoginWidget->HandleRegisterResult();
	NetworkMng->GameInstance->LoginWidget->HandleRegisterError();
	return true;
}

bool Handle_S_SLOGIN(UAONetworkManager* NetworkMng, Protocol::S_LoginSuccessPacket& pkt)
{
	if (NetworkMng->PlayerMng && pkt.has_playerinfo())
	{
		uint64 PlayerId = pkt.playerinfo().playerid();
		uint8 ClassType = static_cast<uint8>(pkt.playerinfo().playerclass());
		NetworkMng->PlayerMng->HandleLogin(PlayerId, ClassType);
		NetworkMng->GameInstance->OnReadyoOpenLevel();
	}
	return true;
}

bool Handle_S_FLOGIN(UAONetworkManager* NetworkMng, Protocol::S_LoginFailPacket& Pkt)
{
	NetworkMng->GameInstance->LoginWidget->HandleLoginResult();
	return true;
}

bool Handle_S_ITEM(UAONetworkManager* NetworkMng, Protocol::S_ItemDataPacket& Pkt)
{
	NetworkMng->PlayerMng->HandleItem(Pkt);
	return true;
}

bool Handle_S_SPAWN(UAONetworkManager* NetworkMng, Protocol::S_SpawnPacket& Pkt)
{
	if (Pkt.playerstates_size() > 0)
	{
		int32 SpawnCount = Pkt.playerstates_size();

		UE_LOG(LogTemp, Log, TEXT("Received Players Count: %d"), SpawnCount);
		for (int i = 0; i < SpawnCount; ++i)
		{
			const Protocol::PlayerState& State = Pkt.playerstates(i);
			uint64 PlayerId = State.playerid();
			FString PlayerName = TCHAR_TO_UTF8(State.playername().c_str());
			FVector Location = FVector(State.playerlocation().x(), State.playerlocation().y(), State.playerlocation().z());
			FRotator Rotation = FRotator(State.playerrotation().pitch(), State.playerrotation().yaw(), State.playerrotation().roll());
			uint8 CalssType = static_cast<uint8>(State.playerclass());

			NetworkMng->PlayerMng->HandleSpawn(PlayerId, PlayerName, CalssType, Location, Rotation);
		}
	}
	return true;
}

bool Handle_S_MOVE(UAONetworkManager* NetworkMng, Protocol::S_MovePacket& Pkt)
{
	uint64 PlayerId = Pkt.playerid();

	Protocol::Vector3* Loc = Pkt.mutable_playerlocation();
	FVector TargetLoc = FVector(Loc->x(), Loc->y(), Loc->z());

	Protocol::Vector3* Vel = Pkt.mutable_playervelocity();
	FVector TargetVel = FVector(Vel->x(), Vel->y(), Vel->z());

	Protocol::Rotator3* Rot = Pkt.mutable_playerrotation();
	FRotator TargetRot = FRotator(Rot->pitch(), Rot->yaw(), Rot->roll());

	UE_LOG(LogTemp, Log, TEXT("PacketHandler - Handle_S_MOVE: %d, Location (%f, %f, %f)"), PlayerId, TargetLoc.X, TargetLoc.Y, TargetLoc.Z);

	NetworkMng->PlayerMng->HnadleMove(PlayerId, TargetLoc, TargetRot, TargetVel);

	return true;
}

bool Handle_S_CREATE(UAONetworkManager* NetworkMng, Protocol::S_DungeonCreatePacket& Pkt)
{
	Protocol::DungeonInfo* DungeonInfo = Pkt.mutable_dungeoninfo();
	int32 DungeonId = DungeonInfo->dungeonid();

	Protocol::DungeonPlayerInfo LeaderInfo = DungeonInfo->leaderinfo();
	FString LeaderName = UTF8_TO_TCHAR(LeaderInfo.membername().c_str());
	Protocol::ClassType LeaderClass = LeaderInfo.memberclass();

	UE_LOG(LogTemp, Log, TEXT("PacketHandler - Handle_S_CREATE /LeaderName: %s"), *LeaderName);
	UAOUIManager* UIManager = NetworkMng->GameInstance->GetSubsystem<UAOUIManager>();
	if (UIManager)
	{
		if (UAODungeonEntranceWidget* DungeonWidget = UIManager->GetWidget<UAODungeonEntranceWidget>())
		{
			DungeonWidget->SetLeaderName(LeaderName);
			DungeonWidget->SetLeaderClass(LeaderClass);
		}
	}

	return true;
}

bool Handle_S_ENTER(UAONetworkManager* NetworkMng, Protocol::S_DungeonEnterPacket& Pkt)
{
	int32 DungeonId = Pkt.dungeonid();
	Protocol::DungeonPlayerInfo NewPlayer = Pkt.enterplayer();
	FString NewPlayerName = UTF8_TO_TCHAR(NewPlayer.membername().c_str());
	Protocol::ClassType NewPlayerClass = NewPlayer.memberclass();
	UE_LOG(LogTemp, Log, TEXT("PacketHandler - Handle_S_Enter/LeaderName: %s"), *NewPlayerName);


	return true;
}

bool Handle_S_READY(UAONetworkManager* NetworkMng, Protocol::S_DungeonReadyPacket& Pkt)
{
	return true;
}

bool Handle_S_START(UAONetworkManager* NetworkMng, Protocol::S_DungeonStartPacket& Pkt)
{
	FString ServerIp = UTF8_TO_TCHAR(Pkt.dungeonip().c_str());
	int32 ServerPort = Pkt.port();

	FString ConnectionURL = FString::Printf(TEXT("%s:%d"), *ServerIp, ServerPort);

	NetworkMng->PlayerMng->HandleDungeonStart(ConnectionURL);
	return true;
}
