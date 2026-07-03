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

// 초기화 함수
void InitPacketHandler()
{
	for (int32 i = 1000; i < UINT16_MAX; i++)
		GAOPacketHandler[i] = &Handle_INVALID;

#if UE_SERVER
	// 서버 핸들러

#else

#if UE_BUILD_DEVELOPMENT
	// 템플릿을 사용하여 자동 파싱 및 핸들러 맵핑
	GAOPacketHandler[PKT_S_SIGNUP] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_SignUpResultPacket>(&FPacketHandler::Handle_S_SIGNUP, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_FLOGIN] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_LoginFailPacket>(&FPacketHandler::Handle_S_FLOGIN, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_SLOGIN] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_LoginSuccessPacket>(&FPacketHandler::Handle_S_SLOGIN, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_ITEM] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_ItemDataPacket>(&FPacketHandler::Handle_S_ITEM, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_SPAWN] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_SpawnPacket>(&FPacketHandler::Handle_S_SPAWN, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_MOVE] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_MovePacket>(&FPacketHandler::Handle_S_MOVE, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_CHAT] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_ChatPacket>(&FPacketHandler::Handle_S_CHAT, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_STOREPURCHASE] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_StorePurchasePacket>(&FPacketHandler::Handle_S_STORE, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_DUNGEONCREATE] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonCreatePacket>(&FPacketHandler::Handle_S_CREATE, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEONENTER] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonEnterPacket>(&FPacketHandler::Handle_S_ENTER, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEONREADY] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonReadyPacket>(&FPacketHandler::Handle_S_READY, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEONSTART] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonStartPacket>(&FPacketHandler::Handle_S_START, Mng, Buf, Len); };


#endif

#endif
}

bool Handle_INVALID(UAONetworkManager* NetworkMng, uint8* Buffer, int32 Len)
{
	return false;
}

FPacketHandler::FPacketHandler(UAONetworkManager* InMng)
	: NetworkMng(InMng)
	, GameInstance(InMng ? InMng->GameInstance : nullptr)
	, PlayerMng(InMng ? InMng->PlayerMng : nullptr)
{
}

UAOLoginUserWidget* FPacketHandler::GetLoginWidget() const
{
	if (GameInstance)
	{
		return GameInstance->LoginWidget;
	}
	return nullptr;
}

bool FPacketHandler::Handle_S_SIGNUP(Protocol::S_SignUpResultPacket& Pkt)
{
	if (UAOLoginUserWidget* RegisterWidget = GameInstance->RegisterWidget)
	{
		if (Pkt.success() == 1)
		{
			RegisterWidget->HandleRegisterResult();
		}
		else
		{
			RegisterWidget->HandleRegisterError();
		}
		return true;
	}
	return false;
}

bool FPacketHandler::Handle_S_SLOGIN(Protocol::S_LoginSuccessPacket& pkt)
{
	if (PlayerMng && pkt.has_playerinfo())
	{
		uint64 PlayerId = pkt.playerinfo().playerid();
		uint8 ClassType = static_cast<uint8>(pkt.playerinfo().playerclass());
		PlayerMng->HandleLogin(PlayerId, ClassType);
		if (GameInstance)
		{
			GameInstance->OnReadyoOpenLevel();
		}
		return true;
	}
	return false;
}

bool FPacketHandler::Handle_S_FLOGIN(Protocol::S_LoginFailPacket& Pkt)
{
	if (UAOLoginUserWidget* LoginWidget = GetLoginWidget())
	{
		LoginWidget->HandleLoginResult();
		return true;
	}
	return false;
}

bool FPacketHandler::Handle_S_ITEM(Protocol::S_ItemDataPacket& Pkt)
{
	if (PlayerMng)
	{
		PlayerMng->HandleItem(Pkt);
		return true;
	}
	return false;
}

bool FPacketHandler::Handle_S_SPAWN(Protocol::S_SpawnPacket& Pkt)
{
	if (!PlayerMng)
		return false;

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

			PlayerMng->HandleSpawn(PlayerId, PlayerName, CalssType, Location, Rotation);
		}
	}
	return true;
}

bool FPacketHandler::Handle_S_MOVE(Protocol::S_MovePacket& Pkt)
{
	if (!PlayerMng)
		return false;

	uint64 PlayerId = Pkt.playerid();

	Protocol::Vector3* Loc = Pkt.mutable_playerlocation();
	FVector TargetLoc = FVector(Loc->x(), Loc->y(), Loc->z());

	Protocol::Vector3* Vel = Pkt.mutable_playervelocity();
	FVector TargetVel = FVector(Vel->x(), Vel->y(), Vel->z());

	Protocol::Rotator3* Rot = Pkt.mutable_playerrotation();
	FRotator TargetRot = FRotator(Rot->pitch(), Rot->yaw(), Rot->roll());

	UE_LOG(LogTemp, Log, TEXT("PacketHandler - Handle_S_MOVE: %d, Location (%f, %f, %f)"), PlayerId, TargetLoc.X, TargetLoc.Y, TargetLoc.Z);

	PlayerMng->HnadleMove(PlayerId, TargetLoc, TargetRot, TargetVel);

	return true;
}

bool FPacketHandler::Handle_S_CREATE(Protocol::S_DungeonCreatePacket& Pkt)
{
	if (!GameInstance)
		return false;

	Protocol::DungeonInfo* DungeonInfo = Pkt.mutable_dungeoninfo();
	int32 DungeonId = DungeonInfo->dungeonid();

	Protocol::DungeonPlayerInfo LeaderInfo = DungeonInfo->leaderinfo();
	FString LeaderName = UTF8_TO_TCHAR(LeaderInfo.membername().c_str());
	Protocol::ClassType LeaderClass = LeaderInfo.memberclass();

	UE_LOG(LogTemp, Log, TEXT("PacketHandler - Handle_S_CREATE /LeaderName: %s"), *LeaderName);
	UAOUIManager* UIManager = GameInstance->GetSubsystem<UAOUIManager>();
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

bool FPacketHandler::Handle_S_ENTER(Protocol::S_DungeonEnterPacket& Pkt)
{
	int32 DungeonId = Pkt.dungeonid();
	Protocol::DungeonPlayerInfo NewPlayer = Pkt.enterplayer();
	FString NewPlayerName = UTF8_TO_TCHAR(NewPlayer.membername().c_str());
	Protocol::ClassType NewPlayerClass = NewPlayer.memberclass();
	UE_LOG(LogTemp, Log, TEXT("PacketHandler - Handle_S_Enter/LeaderName: %s"), *NewPlayerName);

	return true;
}

bool FPacketHandler::Handle_S_READY(Protocol::S_DungeonReadyPacket& Pkt)
{
	return true;
}

bool FPacketHandler::Handle_S_START(Protocol::S_DungeonStartPacket& Pkt)
{
	if (!PlayerMng)
		return false;

	FString ServerIp = UTF8_TO_TCHAR(Pkt.dungeonip().c_str());
	int32 ServerPort = Pkt.port();

	FString ConnectionURL = FString::Printf(TEXT("%s:%d"), *ServerIp, ServerPort);

	PlayerMng->HandleDungeonStart(ConnectionURL);
	return true;
}

bool FPacketHandler::Handle_S_CHAT(Protocol::S_ChatPacket& Pkt)
{
	FString SenderName = UTF8_TO_TCHAR(Pkt.playerid().c_str());
	FString ChatMsg = UTF8_TO_TCHAR(Pkt.chat().c_str());

	PlayerMng->HandleChatting(SenderName, ChatMsg);
	return false;
}

bool FPacketHandler::Handle_S_STORE(Protocol::S_StorePurchasePacket& Pkt)
{
	Protocol::ItemData Item = Pkt.iteminfo();

	PlayerMng->HandleStorePurchase(Item);
	return false;
}
