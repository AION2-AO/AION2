// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "AOClassSwitcherWidget.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class AION2_API UAOClassSwitcherWidget : public UAOUserWidgetBase
{
	GENERATED_BODY()
	
public:
	void SetClassWidget(uint8 ClassType);
	void SetReadyState(bool InReadyState);

	// Leader인 ClassSwitcher에 보이는 아이콘을 보이도록 설정
	void SetLeaderState(bool InLeaderState);

	void SetPlayerName(FText InName);

public:
	FORCEINLINE uint64 GetCachedPlayerId() const { return CachedPlayerId; }
	FORCEINLINE void SetCachedPlayerId(uint64 InId) { CachedPlayerId = InId; }
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* WidgetSwitcher_Class;

	UPROPERTY(meta = (BindWidget))
	class UUserWidget* None_Class;

	UPROPERTY(meta = (BindWidget))
	class UUserWidget* AssassinClassWidget;

	UPROPERTY(meta = (BindWidget))
	class UUserWidget* ClericClassWidget;

	UPROPERTY(meta = (BindWidget))
	class UUserWidget* RangerClassWidget;

	UPROPERTY(meta = (BindWidget))
	class UUserWidget* TemplarClassWidget;

	// Optional Player State:: IsLeader?
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> LeaderIcon;

	// Optional_UserName
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TB_PlayerName;

	// 준비되면 보일 것
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ReadyImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TB_Ready;


private:
	uint64 CachedPlayerId = 0;
};
