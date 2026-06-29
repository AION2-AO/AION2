#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/GA_Attack.h"
#include "GA_RangeCheckAttack.generated.h"

UCLASS()
class AION2_API UGA_RangeCheckAttack : public UGA_Attack
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
