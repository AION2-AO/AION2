// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOMainHUDWidget.h"

#include "UI/AOPlayerHUDWidget.h"
#include "UI/AODungeonHUDWidget.h"
#include "Character/Monster/AOMonsterBase.h"
#include "AbilitySystemComponent.h"


void UAOMainHUDWidget::BindToPlayerState(AAOPlayerState* InPlayerState)
{
	Super::BindToPlayerState(InPlayerState);

	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->BindToPlayerState(InPlayerState);
	}

	// TODO(suyeon): 던전 HUD에 각 파티원 Stat 추가
	if (DungeonHUDWidget)
	{
		DungeonHUDWidget->BindToPlayerState(InPlayerState);
	}
}

void UAOMainHUDWidget::ShowTargetMonsterHUD(AAOMonsterBase* InMonster)
{
	if (DungeonHUDWidget)
	{
		DungeonHUDWidget->ShowTargetMonsterHUD(InMonster);
	}
}

void UAOMainHUDWidget::HideTargetMonsterHUD()
{
	if (DungeonHUDWidget)
	{
		DungeonHUDWidget->HideTargetMonsterHUD();
	}
}


