// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TH_LaunchProjectile.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UAnimNotify_TH_LaunchProjectile : public UAnimNotify
{
	GENERATED_BODY()
	

public:
	UFUNCTION()
	virtual void Notify(
		USkeletalMeshComponent* MeshComp, 
		UAnimSequenceBase* Animation, 
		const FAnimNotifyEventReference& EventReference) override;




};
