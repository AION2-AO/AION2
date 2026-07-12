// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "AOSlotWidgetBase.generated.h"

/**
 * 
 */

class UButton;
class UTexture2D;

struct FButtonStyle;
struct FSlateBrush;

enum class ESlotType : uint8
{
	None,
	Item_Quick,
	Skill_Quick,
	Buff
};

UCLASS()
class AION2_API UAOSlotWidgetBase : public UAOUserWidgetBase
{
	GENERATED_BODY()
	

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> SlotButton;
};
