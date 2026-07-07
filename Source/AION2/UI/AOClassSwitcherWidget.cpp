// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOClassSwitcherWidget.h"
#include "Components/WidgetSwitcher.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"


void UAOClassSwitcherWidget::SetClassWidget(uint8 ClassType)
{
	UWidget* TargetWidget = nullptr;

	switch (ClassType)
	{
	case 1:
		TargetWidget = AssassinClassWidget;
		break;
	case 2:
		TargetWidget = ClericClassWidget;
		break;
	case 3:
		TargetWidget = RangerClassWidget;
		break;
	case 4:
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

