// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOSlotWidgetBase.h"
#include "AOItemQuickSlotWidget.generated.h"

/**
 * 
 */

class UTextBlock;

UCLASS()
class AION2_API UAOItemQuickSlotWidget : public UAOSlotWidgetBase
{
	GENERATED_BODY()
	
	

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> TextBlock_ItemCount;

protected:
	ESlotType slotType = ESlotType::Item_Quick;
};
