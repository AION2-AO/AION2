#include "GAS/GA/GA_StopGlide.h"
#include "GAS/GA/AT/AT_WaitLanding.h"
#include "Character/AOCharacterMovementComponent.h"
#include "Character/Daeva/Daeva.h"

#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UGA_StopGlide::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (!Character)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);

    ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get());
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Daeva->GetMontageByID(EMontageID::StopGlide), 1.5f);
    if (Daeva->HasAuthority())
    {
        Daeva->Multicast_PlayWingMontage(EMontageID::StopGlide, 1.8f);
    }

    MontageTask->OnCompleted.AddDynamic(this, &UGA_StopGlide::OnMontageTaskFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_StopGlide::OnMontageTaskFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_StopGlide::OnMontageTaskCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_StopGlide::OnMontageTaskCancelled);
    MontageTask->ReadyForActivation();
}

void UGA_StopGlide::OnMontageTaskFinished()
{
    ADaeva* Daeva = Cast<ADaeva>(GetAvatarActorFromActorInfo());
    if (Daeva->HasAuthority())
    {
        Daeva->SetWingVisibilityOnServer(false);
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_StopGlide::OnMontageTaskCancelled()
{
    ADaeva* Daeva = Cast<ADaeva>(GetAvatarActorFromActorInfo());
    if (Daeva->HasAuthority())
    {
        Daeva->SetWingVisibilityOnServer(false);
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
