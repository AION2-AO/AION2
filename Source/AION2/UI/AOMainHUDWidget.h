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
class UAORaidHUDWidget;
class ADaeva;

UCLASS()
class AION2_API UAOMainHUDWidget : public UAOUserWidgetBase
{
	GENERATED_BODY()
	
public:
    void BindToPlayerState(AAOPlayerState* InPlayerState) override;

public:
    void SetRaidHUDVisible();
    UAOPlayerHUDWidget* GetPlayerHUDWidget() const { return PlayerHUDWidget; }

protected:
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UAOPlayerHUDWidget> PlayerHUDWidget;

    // 마을에서는 없을 수도 있으니 BindWidgetOptional.
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    TObjectPtr<UAORaidHUDWidget> RaidHUDWidget;

};
