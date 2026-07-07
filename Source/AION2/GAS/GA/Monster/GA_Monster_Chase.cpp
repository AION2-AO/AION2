#include "GAS/GA/Monster/GA_Monster_Chase.h"
#include "AI/AIMonsterControllerBase.h"
#include "Character/AOCharacter.h"
#include "GAS/GA/AT/AT_RotateToTarget.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"

UGA_Monster_Chase::UGA_Monster_Chase()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;
}

void UGA_Monster_Chase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
    if (!IsValid(Pawn))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    CachedAIController = Cast<AAIMonsterControllerBase>(Pawn->GetController());
    if (!IsValid(CachedAIController))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!CachedAIController->HasAuthority())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    CachedAIController->ChangeCurrentTargetPlayerByNearest();

    AAOCharacter* Character = Cast<AAOCharacter>(Pawn);
    if (!IsValid(Character))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    CachedTarget = Character->GetCurrentTarget();
    if (!IsValid(CachedTarget))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    RotateTask = UAT_RotateToTarget::RotateToTarget(this, RotateAvailableRange, RotateInterpSpeed);
    if (IsValid(RotateTask))
    {
        RotateTask->ReadyForActivation();
    }

    const EPathFollowingRequestResult::Type MoveResult = CachedAIController->MoveToActor(CachedTarget, AcceptableRadius, true, true, true, nullptr, true);

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        FinishChase(false);
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(TimeoutTimerHandle, this, &UGA_Monster_Chase::OnChaseTimeout, ChaseTimeout, false);
    }
}

void UGA_Monster_Chase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CheckTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    if (IsValid(RotateTask))
    {
        RotateTask->EndTask();
        RotateTask = nullptr;
    }

    if (IsValid(CachedAIController))
    {
        CachedAIController->StopMovement();
    }

    CachedAIController = nullptr;
    CachedTarget = nullptr;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Monster_Chase::OnChaseTimeout()
{
    FinishChase(true);
}

void UGA_Monster_Chase::FinishChase(bool bSuccess)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, !bSuccess);
}
