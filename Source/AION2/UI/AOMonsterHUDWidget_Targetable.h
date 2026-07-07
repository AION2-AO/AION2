// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOMonsterHUDWidget.h"
#include "AOMonsterHUDWidget_Targetable.generated.h"

class UImage;
class UCanvasPanel;
class UOverlay;
class UTextBlock;
class UAODistanceDisplayWidget;

class AAOMonsterBase;

UENUM(BlueprintType)
enum class ETargetableWidgetType : uint8
{
	None,
	FullScreen,
	HeadUp
};

UCLASS()
class AION2_API UAOMonsterHUDWidget_Targetable : public UAOMonsterHUDWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

public:
	virtual void BindToAbilitySystemActor(AActor* InActor) override;
	virtual void ClearBinding() override;

public:
	void SetMonsterHUDVisibility(bool bInVisiblity);


public:
	// 이건 Blueprint에서 지정하기.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targetable Widget", meta = (AllowPrivateAccess = "true"))
	ETargetableWidgetType TargetableWidgetType = ETargetableWidgetType::None;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> Image_NameBG;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCanvasPanel> CanvasPanel_TargetArrowRight;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UCanvasPanel> CanvasPanel_TargetArrowLeft;

	/*
	* ! Warning !: MainHUD에 붙은 UI를 위해 있는 하위 Widget인데,
	* 기능이 같아서 넣은 것이므로 건드리지 말 것
	*/
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UOverlay> Overlay_Root;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UAODistanceDisplayWidget> DistanceDisplayWidget;
	
};
