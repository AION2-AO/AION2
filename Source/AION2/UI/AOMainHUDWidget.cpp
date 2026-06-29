// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOMainHUDWidget.h"

#include "UI/AOPlayerHUDWidget.h"
#include "UI/AORaidHUDWidget.h"
#include "Game/AORaidGameState.h"

void UAOMainHUDWidget::BindToPlayerState(AAOPlayerState* InPlayerState)
{
	Super::BindToPlayerState(InPlayerState);

	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->BindToPlayerState(InPlayerState);
	}

	if (RaidHUDWidget)
	{
		RaidHUDWidget->BindToPlayerState(InPlayerState);
	}
}

void UAOMainHUDWidget::SetRaidHUDVisible()
{
	const bool bIsRaidLevel =
		GetWorld() && GetWorld()->GetGameState<AAORaidGameState>() != nullptr;
	if (RaidHUDWidget)
	{
		RaidHUDWidget->SetVisibility(bIsRaidLevel? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
