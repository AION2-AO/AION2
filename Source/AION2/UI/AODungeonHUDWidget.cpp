// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AODungeonHUDWidget.h"
#include "UI/AOMonsterHUDWidget_Targetable.h"

#include "Character/Monster/AOMonsterBase.h"

void UAODungeonHUDWidget::ShowTargetMonsterHUD(AAOMonsterBase* InMonster)
{
	if (!InMonster || !MonsterHUDWidget)
	{
		return;
	}

	// TODO(suyeon): MonsterDisplayName을 설정해주는 Logic으로, 나중에 Data-Oriented로 바꾸면 변경해야 함.
	MonsterHUDWidget->SetMonsterIndex(InMonster->DungeonBossIndex);

	// Bind Monster Ability.
	MonsterHUDWidget->BindToAbilitySystemActor(InMonster);

	MonsterHUDWidget->SetMonsterHUDVisibility(true);
}

void UAODungeonHUDWidget::HideTargetMonsterHUD()
{
	if (MonsterHUDWidget)
	{
		MonsterHUDWidget->SetMonsterHUDVisibility(false);
		MonsterHUDWidget->ClearBinding();
	}
}

