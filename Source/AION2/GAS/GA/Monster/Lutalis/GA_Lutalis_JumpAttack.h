// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/GA/Monster/GA_Monster_Attack.h"
#include "GA_Lutalis_JumpAttack.generated.h"

/**
 * Lutalis jump attack selects the farthest target by the existing monster AI flow,
 * then applies a gameplay root-motion move while the montage plays.
 */
UCLASS()
class AION2_API UGA_Lutalis_JumpAttack : public UGA_Monster_Attack
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void RestoreJumpState(class AAOCharacter* Character);

	UFUNCTION()
	void StartMoveToTarget();

	UFUNCTION()
	void OnMoveFinished();

private:
	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MinJumpDistance = 0.0f;

	// 0 means no max distance check.
	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxJumpDistance = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack")
	bool bRefreshPerceivedTargetsBeforeSelect = true;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|MotionWarping")
	bool bUseMotionWarping = true;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|MotionWarping")
	FName MotionWarpTargetName = TEXT("LutalisJumpTarget");

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|Movement")
	bool bUseManualActorMovement = false;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|Movement", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MoveStartDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|Movement", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float MoveDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|Movement", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float LandingDistanceFromTarget = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|Movement")
	bool bSweepMovement = false;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|Movement")
	bool bSnapToLandingLocationOnEnd = true;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|RootMotion")
	bool bUseFlyingMovementModeDuringJump = false;

	UPROPERTY(EditDefaultsOnly, Category = "JumpAttack|RootMotion")
	bool bIgnorePawnCollisionDuringJump = true;

	bool bSavedMovementMode = false;
	TEnumAsByte<enum EMovementMode> SavedMovementMode = MOVE_None;
	uint8 SavedCustomMovementMode = 0;

	bool bSavedPawnCollisionResponse = false;
	TEnumAsByte<enum ECollisionResponse> SavedPawnCollisionResponse = ECR_Block;
};
