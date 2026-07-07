#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Monster/GA_Monster_Attack.h"
#include "GA_Siliator_PT3.generated.h"

UCLASS()
class AION2_API UGA_Siliator_PT3 : public UGA_Monster_Attack
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void OnHitCheckBegin(FGameplayEventData Payload);

	UFUNCTION()
	void OnHitCheckEnd(FGameplayEventData Payload);
	
	virtual void OnCheckAttackHitEvent(FGameplayEventData Payload) override;
};
