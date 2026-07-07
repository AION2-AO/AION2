#include "GAS/GA/Assassin/GA_Assassin_Key3.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "GameFramework/RootMotionSource.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

void UGA_Assassin_Key3::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	static const FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Assassin.Key3"));
	const int32 Count = ActorInfo->AbilitySystemComponent->GetTagCount(CooldownTag);
	if (Count >= 2)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAOCharacter* Target = AOCharacter->GetCurrentTarget();
	if (!IsValid(Target))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector StartLocation = AOCharacter->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	if (FVector::Distance(StartLocation, TargetLocation) > AttackData.AvailableRange)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsActive())
	{
		return;
	}

	FVector MoveDir = TargetLocation - StartLocation;
	MoveDir.Z = 0.f;
	MoveDir = MoveDir.GetSafeNormal();

	const float BehindDistance = 200.f;
	FVector Destination = TargetLocation + MoveDir * BehindDistance;
	Destination.Z = StartLocation.Z;

	AOCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	const float Duration = 0.2f;
	UAbilityTask_ApplyRootMotionMoveToForce* MoveTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(this, NAME_None, Destination, Duration, false, EMovementMode::MOVE_None, true, nullptr, ERootMotionFinishVelocityMode::ClampVelocity, FVector::ZeroVector, 0.f);
	MoveTask->OnTimedOut.AddDynamic(this, &ThisClass::OnMoveFinished);
	MoveTask->OnTimedOutAndDestinationReached.AddDynamic(this, &ThisClass::OnMoveFinished);
	MoveTask->ReadyForActivation();
}

void UGA_Assassin_Key3::OnMoveFinished()
{
	if (ADaeva* Daeva = Cast<ADaeva>(GetAvatarActorFromActorInfo()))
	{
		Daeva->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		Daeva->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

		const FRotator LookAtRot = Daeva->GetLookAtToTarget();
		Daeva->SetCameraByLookAt(LookAtRot);
	}
}

void UGA_Assassin_Key3::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo()))
	{
		AOCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
