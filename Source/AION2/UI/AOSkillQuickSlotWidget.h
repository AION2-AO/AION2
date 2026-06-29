// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOSlotWidgetBase.h"
#include "AOSkillQuickSlotWidget.generated.h"

/**
 * 
 */

class UTextBlock;

UCLASS()
class AION2_API UAOSkillQuickSlotWidget : public UAOSlotWidgetBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> TextBlock_SkillLevel;

protected:
	ESlotType slotType = ESlotType::Skill_Quick;
	
	
};
