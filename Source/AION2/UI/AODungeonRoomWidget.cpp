// Fill out your copyright notice in the Description page of Project Settings.


#include "AODungeonRoomWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Overlay.h"

#include "UI/AOClassSwitcherWidget.h"
#include "Game/AOGameInstance.h"
#include "AODungeonEntranceWidget.h"
#include "Manager/AOPlayerManager.h"



void UAODungeonRoomWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (JoinButton)
	{
		JoinButton->OnClicked.RemoveAll(this);
		JoinButton->OnClicked.AddDynamic(this, &UAODungeonRoomWidget::HandleJoinClicked);
	}

	// MemberClassSlot 초기화
	MemberClassSlots = {
	MemberClassSlot1,
	MemberClassSlot2,
	MemberClassSlot3
	};
}

void UAODungeonRoomWidget::SetLeaderInfo(const Protocol::DungeonPlayerInfo& InInfo)
{
	if (LeaderClassSlot)
	{
		LeaderClassSlot->SetCachedPlayerId(InInfo.memberid());
		SetLeaderClassType(static_cast<uint8>(InInfo.memberclass()));
		LeaderClassSlot->SetLeaderState(true);
		LeaderClassSlot->SetReadyState(InInfo.isready());
	}

	// 리더 이름 설정 (방제와 같은 역할)
	if (TB_LeaderName)
	{
		TB_LeaderName->SetText(FText::FromString(UTF8_TO_TCHAR(InInfo.membername().c_str())));
	}
}


void UAODungeonRoomWidget::SetDungeonInfo(const Protocol::DungeonInfo& InInfo)
{
	// 정상적이지 않은 방 정보
	if (InInfo.dungeonid() <= 0)
	{
		// 기본 빈 상태로 초기화
		if (TB_LeaderName)
		{
			TB_LeaderName->SetText(FText::GetEmpty());
		}

		if (TB_StatusText)
		{
			TB_StatusText->SetText(FText::FromString(TEXT("즉시 참가")));
		}

		if (Image_PlayerJoined)
		{
			Image_PlayerJoined->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (Overlay_NotJoin)
		{
			Overlay_NotJoin->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}

		// 클래스 상태 초기화
		if (LeaderClassSlot)
		{
			LeaderClassSlot->SetCachedPlayerId(0);
			LeaderClassSlot->SetClassWidget(0);
			LeaderClassSlot->SetLeaderState(false);
			LeaderClassSlot->SetReadyState(false);
			LeaderClassSlot->SetPlayerName(FText::GetEmpty());
		}


		for (int i = 0; i < 3; i++)
		{
			MemberClassSlots[i]->SetCachedPlayerId(0);
			MemberClassSlots[i]->SetClassWidget(0);
			MemberClassSlots[i]->SetLeaderState(false);
			MemberClassSlots[i]->SetReadyState(false);
		}
		return;
	}

	DungeonId = InInfo.dungeonid();

	const Protocol::DungeonPlayerInfo& LeaderInfo = InInfo.leaderinfo();

	// 정보 넣어주기
	SetLeaderInfo(LeaderInfo);
	SetMemberInfos(InInfo);
	ApplyParticipationState(InInfo);
}

void UAODungeonRoomWidget::ClearDungeonInfo()
{
	SetDungeonInfo(Protocol::DungeonInfo());
	SetVisibility(ESlateVisibility::Collapsed);
}

void UAODungeonRoomWidget::AddOrUpdateMemberInfo(const Protocol::DungeonPlayerInfo& MemberInfo)
{
	const int32 SlotIndex = MemberInfo.index() - 1;

	if (!MemberClassSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	UAOClassSwitcherWidget* Slot = MemberClassSlots[SlotIndex];
	if (!Slot)
	{
		return;
	}

	Slot->SetCachedPlayerId(MemberInfo.memberid());
	Slot->SetClassWidget(static_cast<uint8>(MemberInfo.memberclass()));
	Slot->SetLeaderState(false);
	Slot->SetReadyState(MemberInfo.isready());
	Slot->SetPlayerName(
		FText::FromString(UTF8_TO_TCHAR(MemberInfo.membername().c_str()))
	);
}

void UAODungeonRoomWidget::SetDungeonReady(uint64 PlayerId)
{
	if (LeaderClassSlot && LeaderClassSlot->GetCachedPlayerId() == PlayerId)
	{
		LeaderClassSlot->SetReadyState(true);
		return;
	}

	for (UAOClassSwitcherWidget* Slot : MemberClassSlots)
	{
		if (!Slot)
		{
			continue;
		}

		if (Slot->GetCachedPlayerId() == PlayerId)
		{
			Slot->SetReadyState(true);
			return;
		}
	}
}

void UAODungeonRoomWidget::HandleJoinClicked()
{
	OnJoinRequested.Broadcast(DungeonId);
}

void UAODungeonRoomWidget::SetLeaderClassType(uint8 ClassType)
{
	// Class Type 위젯 바꿔주기
	LeaderClassSlot->SetClassWidget(ClassType);
}

void UAODungeonRoomWidget::AddMemberInfo(const Protocol::DungeonPlayerInfo& MemberInfo)
{
	AddOrUpdateMemberInfo(MemberInfo);
}

void UAODungeonRoomWidget::SetMemberInfos(const Protocol::DungeonInfo& DungeonInfo)
{
	for (UAOClassSwitcherWidget* Slot : MemberClassSlots)
	{
		if (Slot)
		{
			Slot->SetCachedPlayerId(0);
			Slot->SetClassWidget(0);
			Slot->SetLeaderState(false);
			Slot->SetReadyState(false);
		}
	}

	for (int32 Index = 0; Index < DungeonInfo.members_size(); ++Index)
	{
		AddMemberInfo(DungeonInfo.members(Index));
	}
}

void UAODungeonRoomWidget::ApplyParticipationState(const Protocol::DungeonInfo& DungeonInfo)
{
	const UAOPlayerManager* PlayerManager = nullptr;

	if (const UGameInstance* GI = GetGameInstance())
	{
		PlayerManager = GI->GetSubsystem<UAOPlayerManager>();
	}

	const FPlayerDungeonRoomState State = PlayerManager
		? PlayerManager->GetMyDungeonRoomState()
		: FPlayerDungeonRoomState();

	const bool bIsMyRoom = State.IsJoined() && State.DungeonId == DungeonInfo.dungeonid();
	const bool bRecruiting = DungeonInfo.status() == Protocol::RoomStatus::RECRUITING;

	if (Image_PlayerJoined)
	{
		Image_PlayerJoined->SetVisibility(
			bIsMyRoom ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed
		);
	}

	if (Overlay_NotJoin)
	{
		Overlay_NotJoin->SetVisibility(
			bIsMyRoom ? ESlateVisibility::Hidden : ESlateVisibility::SelfHitTestInvisible
		);
	}

	if (TB_StatusText)
	{
		if (bIsMyRoom)
		{
			TB_StatusText->SetText(FText::FromString(TEXT("참가 중")));
		}
		else if (bRecruiting)
		{
			TB_StatusText->SetText(FText::FromString(TEXT("즉시 참가")));
		}
		else
		{
			TB_StatusText->SetText(FText::FromString(TEXT("진행중")));
		}
	}

	if (JoinButton)
	{
		JoinButton->SetIsEnabled(bRecruiting && !bIsMyRoom);
	}
}
