#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h" 
#include "Character/Daeva/Daeva.h"
#include "Interface/AOCooldownTagProvider.h"
#include "GA_Attack.generated.h"

class UGameplayEffect;

UCLASS()
class AION2_API UGA_Attack : 
	public UGameplayAbility
	, public IAOCooldownTagProvider

{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;


public:
	// UI에 CooldownTag를 제공하는 별도 함수.
	virtual bool GetUICooldownTag(FGameplayTag& OutCooldownTag) const override;


protected:
	UFUNCTION()
	void OnMontageTaskFinished();

	UFUNCTION()
	void OnMontageTaskCancelled();

	UFUNCTION()
	virtual void OnCheckAttackHitEvent(FGameplayEventData Payload);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	FAttackData AttackData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mana")
	float ManaCost = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mana")
	TSubclassOf<UGameplayEffect> ManaCostEffect;

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
