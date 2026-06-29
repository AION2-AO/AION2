#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Character/Daeva/Daeva.h"
#include "GA_Attack.generated.h"

class UGameplayEffect;

UCLASS()
class AION2_API UGA_Attack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	UFUNCTION()
	void OnMontageTaskFinished();

	UFUNCTION()
	void OnMontageTaskCancelled();

	UFUNCTION()
	void OnCheckAttackHitEvent(FGameplayEventData Payload);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	FAttackData AttackData;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffectsToApply;

	UPROPERTY(EditDefaultsOnly, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer RemoveTagsOnActivate;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	EMontageID MontageIDToPlay;

	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	float MontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	FName StartSectionName;
};
