// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_WaveCircleBlue.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UAN_WaveCircleBlue : public UAnimNotify
{
	GENERATED_BODY()
	

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
