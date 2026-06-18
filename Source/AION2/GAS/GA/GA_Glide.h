#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Glide.generated.h"

UCLASS()
class AION2_API UGA_Glide : public UGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void OnLandedCallback();

	UFUNCTION()
	void OnLandMontageFinished();

	UFUNCTION()
	void OnLandMontageCancelled();
};
