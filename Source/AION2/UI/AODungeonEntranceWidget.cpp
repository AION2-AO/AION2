// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AODungeonEntranceWidget.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Game/AOGameInstance.h"
#include "UI/AOClassSwitcherWidget.h"
#include "Manager/AOPlayerManager.h"
#include "Manager/AOUIManager.h"
#include "UI/AODungeonRoomWidget.h"
#include "AION2.h"

void UAODungeonEntranceWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// My Dungeon Room Member Class Switcher Widget을 쉽게 관리하기 위해 배열에 넣기.
	MemberClassSlots = {
	Member1,
	Member2,
	Member3,
	Member4
	};

	// Dungeon Room Widget을 쉽게 관리하기 위해 배열에 넣기.
	DungeonRoomWidgets = {
	WBP_DunzeonRoom_0,
	WBP_DunzeonRoom_1,
	WBP_DunzeonRoom_2,
	WBP_DunzeonRoom_3
	};

	ClearDungeonRooms();

	// 처음에는 없는 것으로 생성
	for (UAODungeonRoomWidget* RoomWidget : DungeonRoomWidgets)
	{
		if (!RoomWidget)
		{
			continue;
		}

		RoomWidget->SetVisibility(ESlateVisibility::Hidden);
		RoomWidget->OnJoinRequested.RemoveAll(this);
		RoomWidget->OnJoinRequested.AddDynamic(this, &UAODungeonEntranceWidget::RequestEnterDungeon);
	}


	// EnterButton 이벤트 바인딩
	if (EnterButton)
	{
		EnterButton->OnClicked.RemoveAll(this);
		EnterButton->OnClicked.AddDynamic(this, &UAODungeonEntranceWidget::OnEnterButtonClicked);
	}

	// CreateButton 이벤트 바인딩
	if (CreateButton)
	{
		CreateButton->OnClicked.RemoveAll(this);
		CreateButton->OnClicked.AddDynamic(this, &UAODungeonEntranceWidget::OnCreateButtonClicked);
	}

	if (ReadyButton)
	{
		ReadyButton->OnClicked.RemoveAll(this);
		ReadyButton->OnClicked.AddDynamic(this, &UAODungeonEntranceWidget::OnReadyButtonClicked);
	}

	if (StartButton)
	{
		StartButton->OnClicked.RemoveAll(this);
		StartButton->OnClicked.AddDynamic(this, &UAODungeonEntranceWidget::OnStartButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveAll(this);
		ExitButton->OnClicked.AddDynamic(this, &UAODungeonEntranceWidget::OnExitButtonClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveAll(this);
		CloseButton->OnClicked.AddDynamic(this, &UAODungeonEntranceWidget::OnCloseButtonClicked);
	}

	// 처음에는 참가하지 않은 상태로 조정
	SetNotJoined();
}

void UAODungeonEntranceWidget::OnEnterButtonClicked()
{
	if (bIsEnter)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("C++: Enter Button Clicked!"));
	bIsEnter = true;
	Protocol::C_DungeonEnterPacket  EnterPacket;

	if (const UAOGameInstance* GI = Cast<UAOGameInstance>(GetGameInstance()))
	{
		EnterPacket.set_playerid(GI->GetMyPlayerId());
		EnterPacket.set_dungeonid(0);
	}

	SEND_PACKET(EnterPacket, PKT_C_DUNGEONENTER);
}

void UAODungeonEntranceWidget::OnCreateButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("C++: Create Button Clicked!"));

	Protocol::C_DungeonCreatePacket CreatePacket;

	SEND_PACKET(CreatePacket, PKT_C_DUNGEONCREATE);

}
void UAODungeonEntranceWidget::OnStartButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("C++: Start Button Clicked!"));

	const UAOPlayerManager* PlayerManager = GetPlayerManager();
	if (!PlayerManager)
	{
		return;
	}

	const FPlayerDungeonRoomState& State = PlayerManager->GetMyDungeonRoomState();
	if (!State.IsLeader() || State.DungeonId <= 0)
	{
		return;
	}


	Protocol::C_DungeonStartPacket StartPacket;
	StartPacket.set_dungeonid(State.DungeonId);
	SEND_PACKET(StartPacket, PKT_C_DUNGEONSTART);
}

void UAODungeonEntranceWidget::OnReadyButtonClicked()
{
	const UAOPlayerManager* PlayerManager = GetPlayerManager();
	if (!PlayerManager)
	{
		return;
	}

	const FPlayerDungeonRoomState& State = PlayerManager->GetMyDungeonRoomState();
	if (!State.IsMember() || State.DungeonId <= 0)
	{
		return;
	}

	Protocol::C_DungeonReadyPacket ReadyPacket;
	ReadyPacket.set_dungeonid(State.DungeonId);
	SEND_PACKET(ReadyPacket, PKT_C_DUNGEONREADY);
}

void UAODungeonEntranceWidget::OnExitButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("C++: Exit Button Clicked!"));
	const UAOPlayerManager* PlayerManager = GetPlayerManager();
	if (!PlayerManager)
	{
		return;
	}

	const FPlayerDungeonRoomState& State = PlayerManager->GetMyDungeonRoomState();
	if (!State.IsMember() || State.DungeonId <= 0)
	{
		return;
	}
	if (ExitButton)
	{
		Protocol::C_DungeonExitPacket exitPacket;
		exitPacket.set_dungeonid(State.DungeonId);
		if (const UAOGameInstance* GI = Cast<UAOGameInstance>(GetGameInstance()))
		{
			exitPacket.set_playerid(GI->GetMyPlayerId());
		}
		SEND_PACKET(exitPacket, PKT_C_DUNGEONEXIT);
	}
}

void UAODungeonEntranceWidget::OnCloseButtonClicked()
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (UAOUIManager* UIManager = GI->GetSubsystem<UAOUIManager>())
		{
			UIManager->HideWidget(this);
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	OnWidgetClosed.Broadcast();
}

UAOPlayerManager* UAODungeonEntranceWidget::GetPlayerManager() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UAOPlayerManager>();
	}

	return nullptr;
}

void UAODungeonEntranceWidget::SetNotJoined()
{
	bIsEnter = false;
	ClearMemberSlots();
	ApplyEntranceState();
}

void UAODungeonEntranceWidget::SetDungeonInfo(const Protocol::DungeonInfo& DungeonInfo)
{
	SetMemberSlots(DungeonInfo);
	ApplyEntranceState();
}


void UAODungeonEntranceWidget::SetDungeonCreated(const Protocol::DungeonInfo& DungeonInfo)
{
	SetDungeonInfo(DungeonInfo);
}

void UAODungeonEntranceWidget::SetDungeonEntered(int32 DungeonId, const Protocol::DungeonPlayerInfo& EnterPlayer)
{
	bIsEnter = false;
	if (UAOPlayerManager* PlayerManager = GetPlayerManager())
	{
		if (PlayerManager->GetMyDungeonRoomState().DungeonId == DungeonId)
		{
			SetMemberSlot(EnterPlayer.index(), EnterPlayer);
		}
	}

	for (UAODungeonRoomWidget* RoomWidget : DungeonRoomWidgets)
	{
		if (RoomWidget && RoomWidget->GetDungeonId() == DungeonId)
		{
			RoomWidget->AddOrUpdateMemberInfo(EnterPlayer);
			break;
		}
	}

	ApplyEntranceState();
}

void UAODungeonEntranceWidget::SetDungeonReady(int32 DungeonId, uint64 PlayerId, bool bIsReady)
{
	// 내가 현재 참가 중인 방의 Ready Packet인지를 거르기
	// 내가 참가하지 않은 방의 Ready 상태는 보이지 않는다.
	const UAOPlayerManager* PlayerManager = GetPlayerManager();
	const FPlayerDungeonRoomState State = PlayerManager
		? PlayerManager->GetMyDungeonRoomState()
		: FPlayerDungeonRoomState();

	if (!State.IsJoined() || State.DungeonId != DungeonId)
	{
		return;
	}

	for (UAOClassSwitcherWidget* Slot : MemberClassSlots)
	{
		if (Slot && Slot->GetCachedPlayerId() == PlayerId)
		{		
			if (bIsReady)
				Slot->SetReadyState(true);
			else
				Slot->SetReadyState(false);
			break;
		}
	}

	// 내가 있는 RoomWidget에도 반영
	for (UAODungeonRoomWidget* RoomWidget : DungeonRoomWidgets)
	{
		if (RoomWidget && RoomWidget->GetDungeonId() == DungeonId)
		{
			RoomWidget->SetDungeonReady(PlayerId);
			break;
		}
	}

	ApplyEntranceState();
}

void UAODungeonEntranceWidget::SetDungeonExit(int32 DungeonId, uint64 ExitPlayerId, const Protocol::DungeonInfo& DungeonInfo)
{
	UAOPlayerManager* PlayerManager = GetPlayerManager();
	if (!PlayerManager) return;
	const FPlayerDungeonRoomState& MyState = PlayerManager->GetMyDungeonRoomState();
	const bool bIsInSameRoom = MyState.IsJoined() && (MyState.DungeonId == DungeonId);

	// 리더 나가면 방 없앰
	const bool bIsRoomExploded = DungeonInfo.has_leaderinfo() ? (DungeonInfo.leaderinfo().memberid() == ExitPlayerId) : true;

	if (bIsRoomExploded)
	{
		if (bIsInSameRoom)
		{
			PlayerManager->ClearMyDungeonRoomState();
			SetNotJoined();
		}
		for (UAODungeonRoomWidget* RoomWidget : DungeonRoomWidgets)
		{
			if (RoomWidget && RoomWidget->GetDungeonId() == DungeonId)
			{
				RoomWidget->ClearDungeonInfo();
				break;
			}
		}
	}
	// 그냐ㅕㅇ 멤버면 룸 새로고침
	else
	{
		if (bIsInSameRoom)
		{
			SetDungeonInfo(DungeonInfo);
		}
		for (UAODungeonRoomWidget* RoomWidget : DungeonRoomWidgets)
		{
			if (RoomWidget && RoomWidget->GetDungeonId() == DungeonId)
			{
				RoomWidget->SetDungeonInfo(DungeonInfo);
				break;
			}
		}
	}
	ApplyEntranceState();
}

void UAODungeonEntranceWidget::InitializeWaitingRoom()
{
	bIsEnter = false;
	// 이전 목록 노출 방지 목적
	SetNotJoined();
	ClearDungeonRooms();
}

void UAODungeonEntranceWidget::RequestEnterDungeon(int32 DungeonId)
{
	if (DungeonId <= 0)
	{
		DungeonId = 0;
	}

	UE_LOG(LogTemp, Warning, TEXT("C++: Request Enter DungeonId: %d"), DungeonId);

	Protocol::C_DungeonEnterPacket EnterPacket;

	if (const UAOGameInstance* GI = Cast<UAOGameInstance>(GetGameInstance()))
	{
		EnterPacket.set_playerid(GI->GetMyPlayerId());
	}

	EnterPacket.set_dungeonid(DungeonId);

	SEND_PACKET(EnterPacket, PKT_C_DUNGEONENTER);
}

void UAODungeonEntranceWidget::ApplyEntranceState()
{
	const UAOPlayerManager* PlayerManager = GetPlayerManager();
	const FPlayerDungeonRoomState State = PlayerManager
		? PlayerManager->GetMyDungeonRoomState()
		: FPlayerDungeonRoomState();

	const bool bNotJoined = !State.IsJoined();
	const bool bLeader = State.IsLeader();
	const bool bMember = State.IsMember();
	const bool bReady = State.ReadyState == EReadyState::Ready;

	if (Overlay_PlayerRoomState)
	{
		Overlay_PlayerRoomState->SetVisibility(
			bNotJoined ? ESlateVisibility::Hidden : ESlateVisibility::SelfHitTestInvisible
		);
	}

	if (Overlay_TitleAreaBottom)
	{
		Overlay_TitleAreaBottom->SetVisibility(
			bNotJoined ? ESlateVisibility::Hidden : ESlateVisibility::SelfHitTestInvisible
		);
	}

	if (Overlay_OutRoom)
	{
		Overlay_OutRoom->SetVisibility(
			bNotJoined ? ESlateVisibility::Visible : ESlateVisibility::Hidden
		);
	}

	if (Overlay_InRoom)
	{
		Overlay_InRoom->SetVisibility(
			bNotJoined ? ESlateVisibility::Hidden : ESlateVisibility::Visible
		);
	}

	if (Overlay_InRoom_Member)
	{
		Overlay_InRoom_Member->SetVisibility(
			bMember ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
		);
	}

	if (Overlay_InRoom_Leader)
	{
		Overlay_InRoom_Leader->SetVisibility(
			bLeader ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
		);
	}

}

void UAODungeonEntranceWidget::SetMemberSlots(const Protocol::DungeonInfo& DungeonInfo)
{
	ClearMemberSlots();

	if (DungeonInfo.has_leaderinfo())
	{
		SetMemberSlot(0, DungeonInfo.leaderinfo());
	}

	for (int32 Index = 0; Index < DungeonInfo.members_size(); ++Index)
	{
		const Protocol::DungeonPlayerInfo& MemberInfo = DungeonInfo.members(Index);
		SetMemberSlot(MemberInfo.index(), MemberInfo);
	}
}

void UAODungeonEntranceWidget::SetMemberSlot(int32 SlotIndex, const Protocol::DungeonPlayerInfo& PlayerInfo)
{
	if (!MemberClassSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	if (MemberClassSlots[SlotIndex])
	{
		MemberClassSlots[SlotIndex]->SetCachedPlayerId(PlayerInfo.memberid());
		MemberClassSlots[SlotIndex]->SetClassWidget(static_cast<uint8>(PlayerInfo.memberclass()));
		MemberClassSlots[SlotIndex]->SetLeaderState(SlotIndex == 0);
		MemberClassSlots[SlotIndex]->SetReadyState(PlayerInfo.isready());
		MemberClassSlots[SlotIndex]->SetPlayerName(
			FText::FromString(UTF8_TO_TCHAR(PlayerInfo.membername().c_str())
			)
		);
	}
}

void UAODungeonEntranceWidget::ClearMemberSlots()
{
	for (UAOClassSwitcherWidget* Slot : MemberClassSlots)
	{
		if (Slot)
		{
			Slot->SetCachedPlayerId(0);
			Slot->SetClassWidget(0);
			Slot->SetLeaderState(false);
			Slot->SetReadyState(false);
			Slot->SetPlayerName(FText::GetEmpty());
		}
	}
}

void UAODungeonEntranceWidget::ClearDungeonRooms()
{
	for (UAODungeonRoomWidget* RoomWidget : DungeonRoomWidgets)
	{
		if (!RoomWidget)
		{
			continue;
		}

		RoomWidget->ClearDungeonInfo();
		RoomWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UAODungeonEntranceWidget::RefreshDungeonRooms(const google::protobuf::RepeatedPtrField<Protocol::DungeonInfo>& DungeonRooms)
{
	ClearDungeonRooms();

	int32 RoomWidgetIndex = 0;

	const UAOPlayerManager* PlayerManager = GetPlayerManager();
	const FPlayerDungeonRoomState State = PlayerManager
		? PlayerManager->GetMyDungeonRoomState()
		: FPlayerDungeonRoomState();

	for (const Protocol::DungeonInfo& DungeonInfo : DungeonRooms)
	{
		if (RoomWidgetIndex >= MaxDungeonRoomCount)
		{
			break;
		}

		// 유효하지 않은 방을 걸러냄
		if (DungeonInfo.dungeonid() <= 0)
		{
			continue;
		}

		if (State.IsJoined() && State.DungeonId == DungeonInfo.dungeonid())
		{
			SetDungeonInfo(DungeonInfo);
		}


		// 삭제된 방이 있으면 건너뜀
		UAODungeonRoomWidget* RoomWidget = DungeonRoomWidgets[RoomWidgetIndex];
		if (!RoomWidget)
		{
			continue;
		}

		// 유효한 방만 Slot에 채움
		RoomWidget->SetDungeonInfo(DungeonInfo);
		RoomWidget->OnJoinRequested.RemoveAll(this);
		RoomWidget->OnJoinRequested.AddDynamic(this, &UAODungeonEntranceWidget::RequestEnterDungeon);
		RoomWidget->SetVisibility(ESlateVisibility::Visible);

		++RoomWidgetIndex;
	}

	// 만약 빈 Slot이 있으면 숨김
	for (; RoomWidgetIndex < DungeonRoomWidgets.Num(); ++RoomWidgetIndex)
	{
		if (UAODungeonRoomWidget* RoomWidget = DungeonRoomWidgets[RoomWidgetIndex])
		{
			RoomWidget->ClearDungeonInfo();
			RoomWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
void UAODungeonEntranceWidget::ClearErrorMessage()
{
	if (ErrorMessage)
	{
		ErrorMessage->SetText(FText::GetEmpty());
	}
}

void UAODungeonEntranceWidget::ShowErrorMessage(Protocol::DungeonFailReason Reason)
{
	switch (Reason)
	{
	case Protocol::DungeonFailReason::Ready:
	{
		ErrorMessage->SetText(FText::FromString(TEXT("모든 참가자가 준비 중이어야 합니다.")));
		break;
	}
	case Protocol::DungeonFailReason::FullDungeon:
	{
		ErrorMessage->SetText(FText::FromString(TEXT("모든 참가자가 준비 중이어야 합니다.")));
		break;
	}
	default:
		break;
	}
	bIsEnter = false;
	GetWorld()->GetTimerManager().ClearTimer(ErrorMessageTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		ErrorMessageTimerHandle,
		this,
		&UAODungeonEntranceWidget::ClearErrorMessage,
		2.0f,
		false
	);

}

