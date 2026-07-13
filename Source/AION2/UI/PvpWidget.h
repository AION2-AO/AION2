// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PvpWidget.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UPvpWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 서버 패킷이 도달했을 때 호출하는 함수
	UFUNCTION(BlueprintCallable, Category = "Pvp")
	void UpdatePvpWidget(uint8 State, int32 RemainingSeconds);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PvpText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PvpCountDown;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PvpTimer;

protected:
	virtual void NativeConstruct() override;

private:
	uint8 CurrentState = 0;
	int32 CurrentRemainingSeconds = 0;
	FTimerHandle CountdownTimerHandle;

	// 매초 마다 작동하는 로컬 타이머 함수
	void TickTime();
	
	// UI 리프레시 함수
	void RefreshVisuals();
};
