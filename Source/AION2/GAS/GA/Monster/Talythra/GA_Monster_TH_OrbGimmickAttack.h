// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "../GA_Monster_Attack.h"
#include "GA_Monster_TH_OrbGimmickAttack.generated.h"

/**
 *
 */
UCLASS()
class AION2_API UGA_Monster_TH_OrbGimmickAttack : public UGA_Monster_Attack
{
	GENERATED_BODY()


protected:

	virtual void OnCheckAttackHitEvent(FGameplayEventData Payload) override;


public:
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;



protected:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag PendingTag; // 기믹 진행중인 태그 상태 



};
