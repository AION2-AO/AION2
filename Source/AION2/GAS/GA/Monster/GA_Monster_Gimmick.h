// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Monster_Action.h"
#include "GA_Monster_Gimmick.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UGA_Monster_Gimmick : public UGA_Monster_Action
{
	GENERATED_BODY()
	
public:
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;


protected:

	UPROPERTY(EditDefaultsOnly) 
	FGameplayTag PendingTag; // 기믹 진행중인 태그 상태 
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DoneTag; // 기믹이 완료된 상태를 나타내는 태그 
	

};
