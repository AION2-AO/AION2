// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Monster/Lutalis/GA_Lutalis_JumpAttack.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AI/AIMonsterControllerBase.h"
#include "Character/AOCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/GA/AT/Monster/AT_Monster_MoveToLocation.h"
#include "MotionWarpingComponent.h"

void UGA_Lutalis_JumpAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo)
	{
		UGameplayAbility::EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bSavedMovementMode = false;
	bSavedPawnCollisionResponse = false;

	AAOCharacter* Character = Cast<AAOCharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Character))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAIMonsterControllerBase* AIController = Cast<AAIMonsterControllerBase>(Character->GetController());
	if (!IsValid(AIController) || !AIController->HasAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (bRefreshPerceivedTargetsBeforeSelect)
	{
		AIController->RefreshPerceivedTargets();
	}

	AIController->ChangeCurrentTargetPlayerByFarthest();

	AAOCharacter* Target = Cast<AAOCharacter>(AIController->Get_CurrentTargetPlayer());
	if (!IsValid(Target) || Target->IsDead())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Character->SetCurrentTarget(Target);

	const float Distance = FVector::Dist2D(
		Character->GetActorLocation(),
		Target->GetActorLocation()
	);

	if (Distance < MinJumpDistance || (MaxJumpDistance > 0.0f && Distance > MaxJumpDistance))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AIController->StopMovement();

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();

		if (bUseFlyingMovementModeDuringJump)
		{
			SavedMovementMode = MovementComponent->MovementMode;
			SavedCustomMovementMode = MovementComponent->CustomMovementMode;
			bSavedMovementMode = true;

			MovementComponent->SetMovementMode(MOVE_Flying);
		}
	}

	if (bIgnorePawnCollisionDuringJump)
	{
		if (UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
		{
			SavedPawnCollisionResponse = CapsuleComponent->GetCollisionResponseToChannel(ECC_Pawn);
			bSavedPawnCollisionResponse = true;

			CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		}
	}

	FVector Direction = Target->GetActorLocation() - Character->GetActorLocation();
	Direction.Z = 0.0f;
	if (!Direction.IsNearlyZero())
	{
		Character->SetActorRotation(Direction.Rotation());
	}

	if (bUseMotionWarping && !MotionWarpTargetName.IsNone())
	{
		if (UMotionWarpingComponent* MotionWarpingComponent = Character->FindComponentByClass<UMotionWarpingComponent>())
		{
			FVector MoveDir = Direction.GetSafeNormal();
			if (!MoveDir.IsNearlyZero())
			{
				FVector WarpLocation = Target->GetActorLocation() - MoveDir * LandingDistanceFromTarget;
				WarpLocation.Z = Character->GetActorLocation().Z;

				MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
					MotionWarpTargetName,
					WarpLocation,
					MoveDir.Rotation()
				);
			}
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsActive() || !bUseManualActorMovement)
	{
		return;
	}

	if (MoveStartDelay > 0.0f)
	{
		UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, MoveStartDelay);
		DelayTask->OnFinish.AddDynamic(this, &ThisClass::StartMoveToTarget);
		DelayTask->ReadyForActivation();
	}
	else
	{
		StartMoveToTarget();
	}
}

void UGA_Lutalis_JumpAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	AAOCharacter* Character = nullptr;
	if (ActorInfo)
	{
		Character = Cast<AAOCharacter>(ActorInfo->AvatarActor.Get());
	}

	RestoreJumpState(Character);

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		UGameplayAbility::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		return;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Lutalis_JumpAttack::RestoreJumpState(AAOCharacter* Character)
{
	if (!IsValid(Character))
	{
		bSavedMovementMode = false;
		bSavedPawnCollisionResponse = false;
		return;
	}

	if (bSavedMovementMode)
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			MovementComponent->SetMovementMode(SavedMovementMode.GetValue(), SavedCustomMovementMode);
		}

		bSavedMovementMode = false;
	}

	if (bSavedPawnCollisionResponse)
	{
		if (UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
		{
			CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, SavedPawnCollisionResponse.GetValue());
		}

		bSavedPawnCollisionResponse = false;
	}
}

void UGA_Lutalis_JumpAttack::StartMoveToTarget()
{
	AAOCharacter* Character = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
	if (!IsValid(Character))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	AAOCharacter* Target = Cast<AAOCharacter>(Character->GetCurrentTarget());
	if (!IsValid(Target) || Target->IsDead())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const FVector StartLocation = Character->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();

	FVector MoveDir = TargetLocation - StartLocation;
	MoveDir.Z = 0.0f;
	MoveDir = MoveDir.GetSafeNormal();
	if (MoveDir.IsNearlyZero())
	{
		return;
	}

	Character->SetActorRotation(MoveDir.Rotation());

	FVector Destination = TargetLocation - MoveDir * LandingDistanceFromTarget;
	Destination.Z = StartLocation.Z;

	const float Duration = FMath::Max(MoveDuration, KINDA_SMALL_NUMBER);

	UAT_Monster_MoveToLocation* MoveTask = UAT_Monster_MoveToLocation::MoveToLocation(
		this,
		Destination,
		Duration,
		bSweepMovement,
		bSnapToLandingLocationOnEnd
	);

	MoveTask->OnFinished.AddDynamic(this, &ThisClass::OnMoveFinished);
	MoveTask->ReadyForActivation();
}

void UGA_Lutalis_JumpAttack::OnMoveFinished()
{
}
