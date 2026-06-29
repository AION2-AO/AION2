// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/TalythraTypes.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Monster_Attack.generated.h"


/**
 * 
 */
UCLASS()
class AION2_API UGA_Monster_Attack : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateCancelAbility) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION() 
	void OnMontageTaskFinished(); 

	UFUNCTION() 
	void OnMontageTaskCancelled();

	UFUNCTION()
	void OnCheckAttackHitEvent(FGameplayEventData Payload);

	UFUNCTION()
	void AbilityEnd(); 
	

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	FAttackData AttackData;


private:

	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	FGameplayTag MontageTag;


	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	float MontagePlayRate;

	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	FName StartSectionName;

};
