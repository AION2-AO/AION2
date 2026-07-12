// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DungeonClearWidget.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UDungeonClearWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:

	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* ClearPopup;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Clear1;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Clear2;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GoldPrice;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Check;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CountdownText;

protected:
	virtual void NativeConstruct() override;

public:
	void SetDungeonClearWidget(int32 Gold);
	void DungeonReward();
	void UpdateCountdown();

public:
	UFUNCTION()
	void OnCheckClicked();
	
	FTimerHandle ClearTimer;
	int32 RemainingTime = 3;

	float ClearTime = 3.0f;
	

};
