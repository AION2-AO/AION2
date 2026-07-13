// AOPacketHandler.cpp
#include "Manager/PacketHandlerManager.h"
#include "AONetworkManager.h"
#include "PacketHandler.h"

#include "Manager/AOPlayerManager.h"
#include "Game/AODungeonGameMode.h"
#include "Game/AOGameInstance.h"

#include "Manager/AOUIManager.h"
#include "UI/AOLoginUserWidget.h"
#include "UI/AODungeonEntranceWidget.h"
#include "UI/Mail/MailListRowWidget.h"
#include "UI/Mail/MainMailWidget.h"
#include "UI/Mail/MailData.h"
#include "UI/PvpWidget.h"
#include "Player/AOPlayerController.h"

PacketHandlerFunc GAOPacketHandler[UINT16_MAX];

// 초기화 함수
void InitPacketHandler()
{
	for (int32 i = 1000; i < UINT16_MAX; i++)
		GAOPacketHandler[i] = &Handle_INVALID;


	// 서버 핸들러
	GAOPacketHandler[PKT_S_DUNGEON_CREATE] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_DungeonCreatePacket>(&FPacketHandler::Handle_S_DEDI_CREATE, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_DUNGEON_DEDI_START] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) {return HandlePacketPolicy<Protocol::S_DungeonStartDediPacket>(&FPacketHandler::Handle_S_DUNGEON_SET_PLAYER, Mng, Buf, Len); };


#if UE_BUILD_DEVELOPMENT
	// 템플릿을 사용하여 자동 파싱 및 핸들러 맵핑
	GAOPacketHandler[PKT_S_SIGNUP] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_SignUpResultPacket>(&FPacketHandler::Handle_S_SIGNUP, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_SET_NICNNAME] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_SetNicknamePacket>(&FPacketHandler::Handle_S_SET_NICKNAME, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_LOGIN_FAIL] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_LoginFailPacket>(&FPacketHandler::Handle_S_LOGIN_FAIL, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_LOGIN_SUCCEED] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_LoginSuccessPacket>(&FPacketHandler::Handle_S_LOGIN_SUCCEED, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_ITEM] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_ItemDataPacket>(&FPacketHandler::Handle_S_ITEM, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_SPAWN] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_SpawnPacket>(&FPacketHandler::Handle_S_SPAWN, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_MOVE] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_MovePacket>(&FPacketHandler::Handle_S_MOVE, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DASH] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DashPacket>(&FPacketHandler::Handle_S_DASH, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_PVP_STATE] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_PvpStatePacket>(&FPacketHandler::Handle_S_PVP_STATE, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_ATTACK_RESULT] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_AttackResultPacket>(&FPacketHandler::Handle_S_ATTACK_RESULT, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_JUMP] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_JumpPacket>(&FPacketHandler::Handle_S_JUMP, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_CHAT] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_ChatPacket>(&FPacketHandler::Handle_S_CHAT, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_STORE_PURCHASE] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_StorePurchasePacket>(&FPacketHandler::Handle_S_STORE, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_USE_ITEM] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_UseItemPacket>(&FPacketHandler::Handle_S_USE_ITEM, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_DUNGEON_CREATE] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonCreatePacket>(&FPacketHandler::Handle_S_CREATE, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEON_ENTER_WAITING_ROOM] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonWaitingRoomEnterPacket>(&FPacketHandler::Handle_S_ENTER_WAITING, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEON_ENTER] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonEnterPacket>(&FPacketHandler::Handle_S_ENTER, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEON_READY] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonReadyPacket>(&FPacketHandler::Handle_S_READY, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEON_START] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonStartPacket>(&FPacketHandler::Handle_S_START, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_DUNGEON_EXIT] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonExitPacket>(&FPacketHandler::Handle_S_EXIT, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_DUNGEON_COMPLETE_REQUEST] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_RequestDungeonCompletePacket>(&FPacketHandler::Handle_S_DUNGEON_END, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_DUNGEON_FAIL] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DungeonFailPacket>(&FPacketHandler::Handle_S_DUNGEON_FAIL, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_MAIL_SEND] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_MailSendPacket>(&FPacketHandler::Handle_S_MAIL_SEND, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_MAIL_LIST] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_MailListPacket>(&FPacketHandler::Handle_S_MAIL_LIST, Mng, Buf, Len); };
	GAOPacketHandler[PKT_S_MAIL_CONTENT] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_MailContentPacket>(&FPacketHandler::Handle_S_MAIL_CONTENT, Mng, Buf, Len); };

	GAOPacketHandler[PKT_S_DISCONNECT] = [](UAONetworkManager* Mng, uint8* Buf, int32 Len) { return HandlePacketPolicy<Protocol::S_DisconnectPacket>(&FPacketHandler::Handle_S_DISCONNECT, Mng, Buf, Len); };

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

UAOUIManager* FPacketHandler::GetUIManager() const
{
	if (GameInstance)
	{
		UAOUIManager* UIManager = GameInstance->GetSubsystem<UAOUIManager>();
		if (UIManager)
			return UIManager;
	}
	return nullptr;
}
#pragma region Member Info
bool FPacketHandler::Handle_S_SIGNUP(Protocol::S_SignUpResultPacket& Pkt)
{
	if (UAOLoginUserWidget* RegisterWidget = GameInstance->RegisterWidget)
	{
		if (Pkt.success() == 1)
		{
			UE_LOG(LogTemp, Log, TEXT("회원가입 성공"));
			RegisterWidget->HandleRegisterResult();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("회원가입 실패"));
			RegisterWidget->HandleRegisterError();
		}
		return true;
	}
	return false;
}

bool FPacketHandler::Handle_S_SET_NICKNAME(Protocol::S_SetNicknamePacket& Pkt)
{
	if (UAOLoginUserWidget* RegisterWidget = GameInstance->RegisterWidget)
	{
		RegisterWidget->ReceiveNicknameResult(Pkt.issucceed());
	}
	return false;
}

bool FPacketHandler::Handle_S_LOGIN_SUCCEED(Protocol::S_LoginSuccessPacket& pkt)
{
	if (PlayerMng && pkt.has_playerinfo())
	{
		PlayerMng->HandleLogin(pkt);
		if (GameInstance)
		{
			GameInstance->OnReadyoOpenLevel();
		}
		return true;
	}
	return false;
}

bool FPacketHandler::Handle_S_LOGIN_FAIL(Protocol::S_LoginFailPacket& Pkt)
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

bool FPacketHandler::Handle_S_DASH(Protocol::S_DashPacket& Pkt)
{
	if (!PlayerMng) return false;

	Protocol::Vector3* Loc = Pkt.mutable_playerlocation();
	FVector TargetLoc = FVector(Loc->x(), Loc->y(), Loc->z());

	Protocol::Vector3* Vel = Pkt.mutable_playervelocity();
	FVector TargetVel = FVector(Vel->x(), Vel->y(), Vel->z());

	Protocol::Rotator3* Rot = Pkt.mutable_playerrotation();
	FRotator TargetRot = FRotator(Rot->pitch(), Rot->yaw(), Rot->roll());

	PlayerMng->HandleDash(Pkt.playerid(), TargetLoc, TargetRot, TargetVel);
	return true;
}

bool FPacketHandler::Handle_S_PVP_STATE(Protocol::S_PvpStatePacket& Pkt)
{
	uint8 State = Pkt.state();
	int32 Time = Pkt.remainingseconds();
	UAOUIManager* UIManager = GetUIManager();
	if (UIManager && GameInstance)
	{
		UPvpWidget* PvpWidget = UIManager->GetWidget<UPvpWidget>();
		if (!PvpWidget)
		{
			AAOPlayerController* PC = Cast<AAOPlayerController>(GameInstance->GetWorld()->GetFirstPlayerController());
			if (PC && !PC->PvpWidgetClass.IsNull())
			{
				PvpWidget = Cast<UPvpWidget>(UIManager->ShowWidget(PC->PvpWidgetClass, EUILayer::Default));
			}
		}

		if (PvpWidget)
		{
			PvpWidget->UpdatePvpWidget(State, Time);
		}
	}
	PlayerMng->HandleSetPvpState(Pkt.state());
	return false;
}

bool FPacketHandler::Handle_S_ATTACK_RESULT(Protocol::S_AttackResultPacket& Pkt)
{
	PlayerMng->HandleAttack(Pkt);
	return false;
}

bool FPacketHandler::Handle_S_JUMP(Protocol::S_JumpPacket& Pkt)
{
	PlayerMng->HandleJump(Pkt.playerid(), Pkt.isgliding());
	return false;
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
			PlayerMng->TryUpdateMyDungeonRoomState(*DungeonInfo);
			DungeonWidget->SetDungeonCreated(*DungeonInfo);
		}
	}

	return true;
}
# pragma endregion

bool FPacketHandler::Handle_S_DEDI_CREATE(Protocol::S_DungeonCreatePacket& Pkt)
{
	Protocol::DungeonInfo dungeonInfo = Pkt.dungeoninfo();
	if (NetworkMng)
	{
		NetworkMng->PendingDungeonId = dungeonInfo.dungeonid();
	}

	if (GameInstance && GameInstance->GetWorld())
	{
		AAODungeonGameMode* GameMode = Cast<AAODungeonGameMode>(GameInstance->GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->SetDungeonId(dungeonInfo.dungeonid());
			UE_LOG(LogTemp, Warning, TEXT("Handle_S_DEDI_CREATE: Set DungeonId %d directly to GameMode."), dungeonInfo.dungeonid());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Handle_S_DEDI_CREATE: GameMode is not AAODungeonGameMode or is null. Stored PendingDungeonId: %d"), dungeonInfo.dungeonid());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_S_DEDI_CREATE: GameInstance or World is null. Stored PendingDungeonId: %d"), dungeonInfo.dungeonid());
	}
	return false;
}

#pragma region Dungeon State
bool FPacketHandler::Handle_S_ENTER_WAITING(Protocol::S_DungeonWaitingRoomEnterPacket& Pkt)
{
	if (PlayerMng)
	{
		PlayerMng->UpdateMyDungeonRoomStateFromList(Pkt.dungeoninfos());
	}

	UAOUIManager* UIManager = GameInstance
		? GameInstance->GetSubsystem<UAOUIManager>()
		: nullptr;

	// 서버 결과에 따라 Widget 갱신 불러주기
	if (UIManager)
	{
		if (UAODungeonEntranceWidget* DungeonWidget = UIManager->GetWidget<UAODungeonEntranceWidget>())
		{
			DungeonWidget->RefreshDungeonRooms(Pkt.dungeoninfos());
			DungeonWidget->ApplyEntranceState();
		}
	}

	return true;
}


bool FPacketHandler::Handle_S_ENTER(Protocol::S_DungeonEnterPacket& Pkt)
{
	int32 DungeonId = Pkt.dungeonid();
	Protocol::DungeonPlayerInfo NewPlayer = Pkt.enterplayer();
	FString NewPlayerName = UTF8_TO_TCHAR(NewPlayer.membername().c_str());
	//Protocol::ClassType NewPlayerClass = NewPlayer.memberclass();
	UE_LOG(LogTemp, Log, TEXT("PacketHandler - Handle_S_Enter/LeaderName: %s"), *NewPlayerName);
	PlayerMng->UpdateMyDungeonEnterState(DungeonId, Pkt.enterplayer());
	PlayerMng->TryUpdateMyDungeonRoomState(Pkt.dungeoninfo());
	if (UAOUIManager* UIManager = GameInstance ? GameInstance->GetSubsystem<UAOUIManager>() : nullptr)
	{
		if (UAODungeonEntranceWidget* DungeonWidget = UIManager->GetWidget<UAODungeonEntranceWidget>())
		{
			DungeonWidget->SetDungeonEntered(DungeonId, Pkt.enterplayer());
			if (PlayerMng->GetMyDungeonRoomState().DungeonId == DungeonId)
			{
				DungeonWidget->SetDungeonInfo(Pkt.dungeoninfo());
			}
		}
	}

	return true;
}

bool FPacketHandler::Handle_S_READY(Protocol::S_DungeonReadyPacket& Pkt)
{
	PlayerMng->UpdateMyDungeonReadyState(Pkt.dungeonid(), Pkt.playerid());

	// UI 갱신
	if (UAOUIManager* UIManager = GameInstance ? GameInstance->GetSubsystem<UAOUIManager>() : nullptr)
	{
		if (UAODungeonEntranceWidget* DungeonWidget = UIManager->GetWidget<UAODungeonEntranceWidget>())
		{
			DungeonWidget->SetDungeonReady(Pkt.dungeonid(), Pkt.playerid(), Pkt.isready());
		}
	}

	return true;
}

bool FPacketHandler::Handle_S_START(Protocol::S_DungeonStartPacket& Pkt)
{
	if (!PlayerMng)
		return false;

	FString ServerIp = UTF8_TO_TCHAR(Pkt.dungeonip().c_str());
	int32 ServerPort = Pkt.port();

	FString ConnectionURL = FString::Printf(TEXT("%s:%d?Token=%s"), *ServerIp, ServerPort, UTF8_TO_TCHAR(Pkt.clienttoken().c_str()));

	PlayerMng->HandleDungeonStart(ConnectionURL);
	return true;
}

bool FPacketHandler::Handle_S_EXIT(Protocol::S_DungeonExitPacket& Pkt)
{
	if (!PlayerMng)
		return false;
	if (Pkt.playerid() == GameInstance->GetMyPlayerId())
	{
		PlayerMng->ClearMyDungeonRoomState();
	}

	if (UAOUIManager* UIManager = GameInstance ? GameInstance->GetSubsystem<UAOUIManager>() : nullptr)
	{
		if (UAODungeonEntranceWidget* DungeonWidget = UIManager->GetWidget<UAODungeonEntranceWidget>())
		{
			DungeonWidget->SetDungeonExit(Pkt.dungeoninfo().dungeonid(), Pkt.playerid(), Pkt.dungeoninfo());
		}
	}
	return true;
}

bool FPacketHandler::Handle_S_DUNGEON_END(Protocol::S_RequestDungeonCompletePacket& Pkt)
{
	if (!PlayerMng)
		return false;
	PlayerMng->HandleDungeonEnd(Pkt.gold());
	return false;
}

bool FPacketHandler::Handle_S_DUNGEON_FAIL(Protocol::S_DungeonFailPacket& Pkt)
{
	if (UAOUIManager* UIManager = GameInstance ? GameInstance->GetSubsystem<UAOUIManager>() : nullptr)
	{
		if (UAODungeonEntranceWidget* DungeonWidget = UIManager->GetWidget<UAODungeonEntranceWidget>())
		{
			DungeonWidget->ShowErrorMessage(Pkt.reason());
		}
	}
	return true;
}
#pragma endregion

bool FPacketHandler::Handle_S_CHAT(Protocol::S_ChatPacket& Pkt)
{
	FString SenderName = UTF8_TO_TCHAR(Pkt.playerid().c_str());
	FString ChatMsg = UTF8_TO_TCHAR(Pkt.chat().c_str());

	PlayerMng->HandleChatting(SenderName, ChatMsg);
	return true;
}

#pragma region Mail

bool FPacketHandler::Handle_S_MAIL_SEND(Protocol::S_MailSendPacket& Pkt)
{
	int8 FailReason = static_cast<int8>(Pkt.errorcode());
	UAOUIManager* UIManager = GetUIManager();
	if (UIManager)
	{
		UMainMailWidget* MainMailWidget = UIManager->GetWidget<UMainMailWidget>();
		if (MainMailWidget)
		{
			MainMailWidget->ShowErrorMessage(FailReason);
			return true;
		}
	}
	return false;
}

bool FPacketHandler::Handle_S_MAIL_LIST(Protocol::S_MailListPacket& Pkt)
{
	TArray<FMailData> MailList;
	for (int i = 0; i < Pkt.maillists_size(); ++i)
	{
		const Protocol::MailListInfo& mailInfo = Pkt.maillists(i);

		FMailData MailData;
		MailData.MailUID = mailInfo.mailid();
		MailData.Title = UTF8_TO_TCHAR(mailInfo.title().c_str());
		MailData.SenderName = UTF8_TO_TCHAR(mailInfo.sendername().c_str());
		MailData.ExpiredDate = UTF8_TO_TCHAR(mailInfo.expireddate().c_str());
		MailData.bIsReceived = mailInfo.hasitem();
		MailData.bIsRead = false;

		MailList.Add(MailData);
	}

	UAOUIManager* UIManager = GetUIManager();
	if (UIManager && GameInstance)
	{
		AAOPlayerController* PC = Cast<AAOPlayerController>(GameInstance->GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			UUserWidget* OpenedWidget = UIManager->ShowWidget(PC->MainMailWidgetClass, EUILayer::Default);
			UMainMailWidget* MainMailWidget = Cast<UMainMailWidget>(OpenedWidget);
			if (MainMailWidget)
			{
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(MainMailWidget->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = true;

				MainMailWidget->UpdateMailList(MailList);
			}
		}
	}

	return true;
}

bool FPacketHandler::Handle_S_MAIL_CONTENT(Protocol::S_MailContentPacket& Pkt)
{
	FMailData DetailData;
	DetailData.MailUID = Pkt.mailid();
	DetailData.SenderName = UTF8_TO_TCHAR(Pkt.sendername().c_str());
	DetailData.Title = UTF8_TO_TCHAR(Pkt.title().c_str());
	DetailData.Content = UTF8_TO_TCHAR(Pkt.content().c_str());
	DetailData.ExpiredDate = UTF8_TO_TCHAR(Pkt.expireddate().c_str());
	DetailData.Gold = Pkt.gold();
	DetailData.ItemId = Pkt.itemid();
	DetailData.ItemCount = Pkt.itemcount();
	DetailData.bIsReceived = Pkt.isreceived();
	DetailData.bIsRead = Pkt.isread();

	UAOUIManager* UIManager = GetUIManager();
	if (UIManager)
	{
		UMainMailWidget* MainMailWidget = UIManager->GetWidget<UMainMailWidget>();
		if (MainMailWidget)
		{
			MainMailWidget->UpdateMailContent(DetailData);
		}
	}

	return true;
}
#pragma endregion

bool FPacketHandler::Handle_S_STORE(Protocol::S_StorePurchasePacket& Pkt)
{
	Protocol::ItemData Item = Pkt.iteminfo();
	int32 Gold = Pkt.gold();
	PlayerMng->HandleStorePurchase(Item, Gold);
	return true;
}

bool FPacketHandler::Handle_S_USE_ITEM(Protocol::S_UseItemPacket& Pkt)
{
	PlayerMng->HandleUseItem(Pkt);
	return true;
}

bool FPacketHandler::Handle_S_DISCONNECT(Protocol::S_DisconnectPacket& Pkt)
{
	PlayerMng->HandleDisconnect(Pkt.playerid());
	return true;
}


bool FPacketHandler::Handle_S_DUNGEON_SET_PLAYER(Protocol::S_DungeonStartDediPacket Pkt)
{
	PlayerMng->HandleDungeonSetPlayerInfo(Pkt);
	return true;
}
