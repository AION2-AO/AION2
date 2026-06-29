// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Interface/CombatInterface.h"
#include "GA_Monster_AttackWithCooldown.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UGA_Monster_AttackWithCooldown : public UGameplayAbility
{
	GENERATED_BODY()	

public:
	UGA_Monster_AttackWithCooldown();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnMontageTaskFinished();

	UFUNCTION()
	void OnMontageTaskCancelled();

	UFUNCTION()
	void OnCheckAttackHitEvent(FGameplayEventData Payload);

	UFUNCTION()
	void AbilityEnd();


	UFUNCTION()
	virtual void PostInitProperties() override;

	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;


protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	FAttackData AttackData;


	/** 쿨다운 중임을 나타낼 태그. ActivationBlockedTags에 자동 추가됨. */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown", meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;

	/** 쿨다운 지속 시간 (초). BP에서 직접 입력 가능. */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown", meta = (ClampMin = "0.0"))
	float CooldownDuration = 1.0f;

	/** 쿨다운에 사용할 공통 GE. 보통 자식이 안 건드려도 됨 (디폴트 사용). */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	TSubclassOf<UGameplayEffect> CooldownGEClass;


private:

	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	FGameplayTag MontageTag;




	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	float MontagePlayRate;

	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	FName StartSectionName;


};
