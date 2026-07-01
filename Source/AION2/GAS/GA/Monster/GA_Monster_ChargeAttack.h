// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Monster/GA_Monster_Action.h"
#include "interface/CombatInterface.h"
#include "GA_Monster_AttackWithCooldown.h"
#include "GA_Monster_ChargeAttack.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UGA_Monster_ChargeAttack : public UGA_Monster_AttackWithCooldown
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
	void OnHitCheckBegin(FGameplayEventData Payload);

	UFUNCTION()
	void OnHitCheckEnd(FGameplayEventData Payload);


	UFUNCTION()
	void OnCapsuleBindOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);




	void BindCapsuleOverlap();
	void UnBindCapsuleOverlap(); 
	bool IsVaildTarget(AActor* OtherActor) const; 


protected:
	//UPROPERTY(EditDefaultsOnly, Category = "ChargeAttack")
	//FAttackData AttackData;


	// 한 번의 돌진 동안 중복 타격 방지 
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> HitActors;

	FDelegateHandle OverlapHandle; 


	// 히트체크 활성 여부 - 중복 Bind/Unbind 방어
	bool bHitCheckActive = false;
	

private:
	// 돌진 시작 지점의 Pawn 채널 응답 저장 (원복용) 
	ECollisionResponse SavedPawnResponse = ECR_Block;

};
