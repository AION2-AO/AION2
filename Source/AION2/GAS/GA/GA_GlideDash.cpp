#include "GAS/GA/GA_GlideDash.h"
#include "Character/Daeva/Daeva.h"
#include "Character/AOCharacterMovementComponent.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UGA_GlideDash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get());
    if (!Daeva)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAOCharacterMovementComponent* MoveComp = Cast<UAOCharacterMovementComponent>(Daeva->GetCharacterMovement());

    if (!MoveComp ||
        MoveComp->MovementMode != MOVE_Custom ||
        MoveComp->CustomMovementMode != static_cast<uint8>(EAOMovementMode::Glide))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    MoveComp->StartGlideDash();

    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Daeva->GetMontageByID(EMontageID::GlideDash), 1.0f);
    if (Daeva->HasAuthority())
    {
        Daeva->Multicast_PlayWingMontage(EMontageID::GlideDash, 1.0f);
    }

    MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageTaskFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageTaskFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageTaskCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageTaskCancelled);
    MontageTask->ReadyForActivation();
}

void UGA_GlideDash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get()))
    {

    }

    Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_GlideDash::OnMontageTaskFinished()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_GlideDash::OnMontageTaskCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
