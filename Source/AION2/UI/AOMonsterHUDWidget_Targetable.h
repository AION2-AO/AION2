// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOMonsterHUDWidget.h"
#include "AOMonsterHUDWidget_Targetable.generated.h"

class UImage;
class UCanvasPanel;

/**
 * 
 */
UCLASS()
class AION2_API UAOMonsterHUDWidget_Targetable : public UAOMonsterHUDWidget
{
	GENERATED_BODY()
	
public:
	void SetHiddenUIVisibliblity(bool bInVisiblity);

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UImage> Image_NameBG;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCanvasPanel> CanvasPanel_TargetArrowRight;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCanvasPanel> CanvasPanel_TargetArrowLeft;
	
};
