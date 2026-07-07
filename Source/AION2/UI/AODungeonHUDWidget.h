// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "AODungeonHUDWidget.generated.h"

class UAOMonsterHUDWidget_Targetable;
class AAOMonsterBase;

/**
 * 
 */
UCLASS()
class AION2_API UAODungeonHUDWidget : public UAOUserWidgetBase
{
	GENERATED_BODY()
	
public:
	void ShowTargetMonsterHUD(AAOMonsterBase* InMonster);

	void HideTargetMonsterHUD();

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UAOMonsterHUDWidget_Targetable> MonsterHUDWidget;
	
};
