// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/AOTypes.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_TH_SetAttackOrbColor.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UAN_TH_SetAttackOrbColor : public UAnimNotify
{
	GENERATED_BODY()
	

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;


private:
	UPROPERTY(EditAnywhere, Category = "AttackOrb", meta = (AllowPrivateAccess = "true"))
	EOrbColor AttackOrbColor = EOrbColor::None;

};
