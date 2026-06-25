#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Run.generated.h"

class UGameplayEffect;

UCLASS()
class AION2_API UGA_Run : public UGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Run")
	TSubclassOf<UGameplayEffect> RunMoveSpeedEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Run")
	TSubclassOf<UGameplayEffect> RunStaminaDrainEffect;

private:
	void OnStaminaChanged(const FOnAttributeChangeData& Data);

	FActiveGameplayEffectHandle RunMoveSpeedEffectHandle;
	FActiveGameplayEffectHandle RunStaminaDrainEffectHandle;

	FDelegateHandle StaminaChangedDelegateHandle;
};