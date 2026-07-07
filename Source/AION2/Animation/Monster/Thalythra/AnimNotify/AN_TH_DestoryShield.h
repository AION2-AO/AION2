// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_TH_DestoryShield.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UAN_TH_DestoryShield : public UAnimNotify
{
	GENERATED_BODY()


public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	
};
