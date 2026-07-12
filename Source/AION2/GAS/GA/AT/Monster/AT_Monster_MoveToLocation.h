// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_Monster_MoveToLocation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMonsterMoveToLocationDelegate);

/**
 * Server-side actor movement task for monster patterns that need guaranteed capsule movement.
 */
UCLASS()
class AION2_API UAT_Monster_MoveToLocation : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAT_Monster_MoveToLocation();

	UPROPERTY(BlueprintAssignable)
	FMonsterMoveToLocationDelegate OnFinished;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAT_Monster_MoveToLocation* MoveToLocation(
		UGameplayAbility* OwningAbility,
		FVector InTargetLocation,
		float InDuration,
		bool bInSweepMovement = false,
		bool bInSnapToTargetOnEnd = true
	);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

protected:
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	void FinishMove();

private:
	FVector StartLocation = FVector::ZeroVector;
	FVector TargetLocation = FVector::ZeroVector;
	float Duration = 0.0f;
	float ElapsedTime = 0.0f;
	bool bSweepMovement = false;
	bool bSnapToTargetOnEnd = true;
	bool bMoveStarted = false;
	bool bFinished = false;
};
