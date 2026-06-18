#include "GAS/GA/GA_Glide.h"
#include "GAS/GA/AT/AT_WaitLanding.h"
#include "Character/AOCharacterMovementComponent.h"
#include "Character/Daeva/Daeva.h"

#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

void UGA_Glide::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (!Character || !Character->GetCharacterMovement()->IsFalling())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    Character->GetCharacterMovement()->SetMovementMode(MOVE_Custom, static_cast<uint8>(EAOMovementMode::Glide));

    UAT_WaitLanding* WaitLandingTask = UAT_WaitLanding::CreateTask(this);
    WaitLandingTask->OnComplete.AddDynamic(this, &UGA_Glide::OnLandedCallback);
    WaitLandingTask->ReadyForActivation();

    ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get());
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Daeva->GetMontageByAbilityInputID(EMontageID::Glide), 1.0f);
    if (Daeva->HasAuthority())
    {
        Daeva->Multicast_SetWingVisibility(true);
        Daeva->Multicast_PlayWingMontage(EMontageID::Glide, 1.0f);
    }
    MontageTask->ReadyForActivation();
}

void UGA_Glide::OnLandedCallback()
{
    ADaeva* Daeva = Cast<ADaeva>(GetAvatarActorFromActorInfo());
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Daeva->GetMontageByAbilityInputID(EMontageID::GlideLand), 2.f);
    if (Daeva->HasAuthority())
    {
        Daeva->Multicast_PlayWingMontage(EMontageID::GlideLand, 2.f);
    }

    MontageTask->OnCompleted.AddDynamic(this, &UGA_Glide::OnLandMontageFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_Glide::OnLandMontageFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Glide::OnLandMontageCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Glide::OnLandMontageCancelled);
    MontageTask->ReadyForActivation();
}

void UGA_Glide::OnLandMontageFinished()
{
    ADaeva* Daeva = Cast<ADaeva>(GetAvatarActorFromActorInfo());
    if (Daeva->HasAuthority())
    {
        Daeva->Multicast_SetWingVisibility(false);
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Glide::OnLandMontageCancelled()
{
    ADaeva* Daeva = Cast<ADaeva>(GetAvatarActorFromActorInfo());
    if (Daeva->HasAuthority())
    {
        Daeva->Multicast_SetWingVisibility(false);
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
