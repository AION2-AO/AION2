#include "GAS/GA/Monster/Siliator/GA_Siliator_PT2.h"
#include "AI/AIMonsterControllerBase.h"
#include "Character/AOCharacter.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "GameFramework/RootMotionSource.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

void UGA_Siliator_PT2::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAOCharacter* AOCharacter = Cast<AAOCharacter>(ActorInfo->AvatarActor.Get());
	if (!AOCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAIMonsterControllerBase* AIController = Cast<AAIMonsterControllerBase>(AOCharacter->GetController());
	if (!IsValid(AIController))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!AIController->HasAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AIController->ChangeCurrentTargetPlayerByFarthest();

	AAOCharacter* Target = AOCharacter->GetCurrentTarget();
	if (!IsValid(Target))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsActive())
	{
		return;
	}

	const FVector StartLocation = AOCharacter->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();

	FVector MoveDir = TargetLocation - StartLocation;
	MoveDir.Z = 0.f;
	MoveDir = MoveDir.GetSafeNormal();
	AOCharacter->SetActorRotation(MoveDir.Rotation());

	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, 1.0f);
	DelayTask->OnFinish.AddDynamic(this, &ThisClass::StartMoveToTarget);
	DelayTask->ReadyForActivation();
}

void UGA_Siliator_PT2::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo()))
	{
		AOCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Siliator_PT2::StartMoveToTarget()
{
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
	if (!IsValid(AOCharacter))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	AActor* Target = AOCharacter->GetCurrentTarget();
	if (!IsValid(Target))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const FVector StartLocation = AOCharacter->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();

	FVector MoveDir = TargetLocation - StartLocation;
	MoveDir.Z = 0.f;
	MoveDir = MoveDir.GetSafeNormal();
	AOCharacter->SetActorRotation(MoveDir.Rotation());

	const float FrontDistance = -50.f;
	FVector Destination = TargetLocation + MoveDir * FrontDistance;
	Destination.Z = StartLocation.Z;

	AOCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	const float Duration = 0.5f;

	UAbilityTask_ApplyRootMotionMoveToForce* MoveTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(this, NAME_None, Destination, Duration, false, MOVE_None, true, nullptr, ERootMotionFinishVelocityMode::ClampVelocity, FVector::ZeroVector, 0.f);

	MoveTask->OnTimedOut.AddDynamic(this, &ThisClass::OnMoveFinished);
	MoveTask->OnTimedOutAndDestinationReached.AddDynamic(this, &ThisClass::OnMoveFinished);
	MoveTask->ReadyForActivation();
}

void UGA_Siliator_PT2::OnMoveFinished()
{
	if (AAOCharacter* Character = Cast<AAOCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	}
}
