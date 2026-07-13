// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOClassSwitcherWidget.h"

#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

// 나중에 지워야 함!!!
#include "Player/AOPlayerController.h"
// 나중에 지워야 함!!
#include "Character/Daeva/Assassin.h"
#include "Character/Daeva/Cleric.h"
#include "Character/Daeva/Ranger.h"
#include "Character/Daeva/Templar.h"
#include "Game/AOGameInstance.h"
#include "Manager/AOPlayerManager.h"

void UAOClassSwitcherWidget::SetClassWidget(uint8 ClassType)
{
	SetClassWidget(static_cast<EDaevaClassType>(ClassType));
}

void UAOClassSwitcherWidget::SetClassWidget(EDaevaClassType ClassType)
{
	UWidget* TargetWidget = nullptr;

	if (ClassType == EDaevaClassType::None && CachedPlayerId != 0)
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (const UAOPlayerManager* PlayerManager = GI->GetSubsystem<UAOPlayerManager>())
			{
				ClassType = static_cast<EDaevaClassType>(PlayerManager->GetPlayerClassType(CachedPlayerId));
			}
		}
	}

	// TODO(SuYeon): Data 기반으로 만들기.

	/* 
	* 서버에서 값을 안 넣어줬다면 Daeva를 상속받은 class를 기반으로 1번 force input.
	* !! 서버에서 정상적으로 값을 넣어주도록 해야 함 !!
	* !! Editor용 코드 !!
	* 이거까지 안되면 진짜 문제 있는 거임.
	*/
	if (ClassType== EDaevaClassType::None)
	{
		APawn* Pawn = Cast<APawn>(GetOwningPlayerPawn());
		if (AAssassin* assassin = Cast<AAssassin>(Pawn))
		{
			ClassType = EDaevaClassType::Assassin;
		}
		else if (ACleric* cleric = Cast<ACleric>(Pawn))
		{
			ClassType = EDaevaClassType::Cleric;
		}
		else if (ARanger* ranger = Cast<ARanger>(Pawn))
		{
			ClassType = EDaevaClassType::Ranger;
		}
		else if (ATemplar* templar = Cast<ATemplar>(Pawn))
		{
			ClassType = EDaevaClassType::Templar;
		}
	}


	switch (ClassType)
	{
	case EDaevaClassType::Assassin:
		TargetWidget = AssassinClassWidget;
		break;
	case EDaevaClassType::Cleric:
		TargetWidget = ClericClassWidget;
		break;
	case EDaevaClassType::Ranger:
		TargetWidget = RangerClassWidget;
		break;
	case EDaevaClassType::Templar:
		TargetWidget = TemplarClassWidget;
		break;
	default:
		TargetWidget = None_Class;
		break;
	}


	if (TargetWidget)
	{
		WidgetSwitcher_Class->SetActiveWidget(TargetWidget);
	}
}

void UAOClassSwitcherWidget::SetReadyState(bool InReadyState)
{
	ESlateVisibility NewVisibility = InReadyState ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (ReadyImage)
	{
		ReadyImage->SetVisibility(NewVisibility);
	}

	if (TB_Ready)
	{
		TB_Ready->SetVisibility(NewVisibility);
	}

	if (TB_PlayerName)
	{
		TB_PlayerName->SetVisibility(ESlateVisibility::Visible);
	}
}

void UAOClassSwitcherWidget::SetLeaderState(bool InLeaderState)
{
	ESlateVisibility NewVisibility = InLeaderState ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (LeaderIcon)
	{
		LeaderIcon->SetVisibility(NewVisibility);
	}

}

void UAOClassSwitcherWidget::SetPlayerName(FText InName)
{
	if (TB_PlayerName)
	{
		TB_PlayerName->SetText(InName);
		TB_PlayerName->SetVisibility(ESlateVisibility::Visible);
	}
}

