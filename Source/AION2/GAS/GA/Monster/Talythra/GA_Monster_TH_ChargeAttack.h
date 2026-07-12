// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Monster/GA_Monster_ChargeAttack.h"
#include "GAS/GA/AT/Monster/AT_Monster_ChargeHitTick.h"
#include "GA_Monster_TH_ChargeAttack.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API UGA_Monster_TH_ChargeAttack : public UGA_Monster_ChargeAttack
{
	GENERATED_BODY()

protected:

	virtual void OnHitCheckBegin(FGameplayEventData Payload) override; 
	virtual void OnHitCheckEnd(FGameplayEventData Payload) override; 
	
	UFUNCTION()
	void PerformChargeHitCheck(); 
	
private:
	float PreviousCapsuleRadius = 0.f; 

	UPROPERTY(EditAnywhere, Category = "ProjectileSpawn", meta = (AllowPrivateAccess = "true"))
	float ChargeHitRadius ; 


	UPROPERTY()
	TObjectPtr<UAT_Monster_ChargeHitTick> ChargeHitTickTask;

	FVector PrevLocation; 

};
