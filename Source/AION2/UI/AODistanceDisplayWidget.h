// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "AODistanceDisplayWidget.generated.h"

/**
 * 
 */

class UTextBlock;

UCLASS()
class AION2_API UAODistanceDisplayWidget : public UAOUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;


protected:
	// Distance를 표시
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> TB_DistanceFromPlayer;

private:
	// Time to Tick.
	float DistanceUpdateElapsed = 0.0f;
	float DistanceUpdateInterval = 0.1f;
};
