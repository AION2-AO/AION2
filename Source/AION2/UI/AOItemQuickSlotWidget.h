// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOSlotWidgetBase.h"
#include "Item/AOItemDataBase.h" 
#include "AOItemQuickSlotWidget.generated.h"

/**
 * 
 */

class UTextBlock;

UCLASS()
class AION2_API UAOItemQuickSlotWidget : public UAOSlotWidgetBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "QuickSlot")
	void UpdateQuickSlot(const FAOSlotData& SlotData, const FItemData& TemplateData);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "QuickSlot")
	void ClearQuickSlot();

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> TextBlock_ItemCount;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Slot")
	TObjectPtr<UImage> Image_ItemImage;
protected:
	ESlotType slotType = ESlotType::Item_Quick;
};
