#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Monster/GA_Monster_Attack.h"
#include "GA_Siliator_PT2.generated.h"

UCLASS()
class AION2_API UGA_Siliator_PT2 : public UGA_Monster_Attack
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void StartMoveToTarget();

	UFUNCTION()
	void OnMoveFinished();
};
