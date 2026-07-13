#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Character/Daeva/Daeva.h"
#include "Interface/AOCooldownTagProvider.h"
#include "GA_RangedAttack.generated.h"

class UGameplayEffect;

UCLASS()
class AION2_API UGA_RangedAttack 
	: public UGameplayAbility
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
	// UI용 Cooldown Tag를 제공하는 별도 함수를 갖도록 함
	virtual bool GetUICooldownTag(FGameplayTag& OutCooldownTag) const override;

protected :
	UPROPERTY(EditDefaultsOnly, Category = "Mana")
	float ManaCost = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mana")
	TSubclassOf<UGameplayEffect> ManaCostEffect;

protected:
	UFUNCTION()
	void OnMontageTaskFinished();

	UFUNCTION()
	void OnMontageTaskCancelled();

	UFUNCTION()
	void OnLaunchProjectileEvent(FGameplayEventData Payload);

	//HY
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Ragner")
	bool bUseMoveAttackMontage = false;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Ranger")
	TObjectPtr<UAnimMontage> RangerStandAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Ranger")
	TObjectPtr<UAnimMontage> RangerMoveAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Ranger")
	float MoveAttackSpeedThreshold = 10.0f;

	//

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	FAttackData AttackData;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AAOProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	FName ProjectileSpawnSocket;

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
