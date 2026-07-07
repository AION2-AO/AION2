// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "AOMainHUDWidget.generated.h"

/*
* [Main HUD Root]
* 
 * Blueprint가 GAS를 직접 뒤지지 않도록 하고,
 * HUD Widget Class에서 ASC delegate을 받아 event 기반 화면 갱신을 하도록 둔다.
 */

class UAOPlayerHUDWidget;
class UAODungeonHUDWidget;
class UAOChattingWidget;
class ADaeva;

class UAbilitySystemComponent;
class AAOMonsterBase;

UCLASS()
class AION2_API UAOMainHUDWidget : public UAOUserWidgetBase
{
	GENERATED_BODY()
	
public:
    void BindToPlayerState(AAOPlayerState* InPlayerState) override;

public:
    // Show Full-Screen Monster Stat visibility value & Bind ASC.
    void ShowTargetMonsterHUD(AAOMonsterBase* InMonster);

    // Hide Full-Screen Monster Stat visibility value & Unbind ASC.
    void HideTargetMonsterHUD();

public:
    // 외부에서 PlayerHUDWidget의 함수를 사용할 때.
    FORCEINLINE UAOPlayerHUDWidget* GetPlayerHUDWidget() const { return PlayerHUDWidget; }

protected:
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UAOPlayerHUDWidget> PlayerHUDWidget;

    // 마을에서는 없을 수도 있으니 BindWidgetOptional.
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    TObjectPtr<UAODungeonHUDWidget> DungeonHUDWidget;

public:
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    TObjectPtr<UAOChattingWidget> ChattingWidget;
};
