// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AOSoundRow.generated.h"

USTRUCT(BlueprintType)
struct FAOSoundRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* SoundAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float FadeInTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float FadeOutTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float VolumeMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundAttenuation* AttenuationAsset;
};

