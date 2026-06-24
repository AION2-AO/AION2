#include "GAS/GA/GA_Dash.h"
#include "Character/Daeva/Daeva.h"
#include "GAS/AOGameplayTags.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/AOCharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

void UGA_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
    const UAOAttributeSet* AttributeSet = ASC ? ASC->GetSet<UAOAttributeSet>() : nullptr;

    if (AttributeSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Dash Start] Stamina: %.1f / %.1f"),
            AttributeSet->GetStamina(),
            AttributeSet->GetMaxStamina());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Dash Start] AttributeSet is null"));
    }

    if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
    {
        if (Character->GetMovementComponent()->IsFalling() || Character->GetMovementComponent()->IsFlying() || (Character->GetCharacterMovement()->MovementMode == MOVE_Custom && Character->GetCharacterMovement()->CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide)))
        {
            UE_LOG(LogTemp, Warning, TEXT("[Dash Failed - Movement State] Stamina: %.1f / %.1f"),
                AttributeSet ? AttributeSet->GetStamina() : -1.f,
                AttributeSet ? AttributeSet->GetMaxStamina() : -1.f);

            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[Dash Before Commit] Stamina: %.1f / %.1f"),
        AttributeSet ? AttributeSet->GetStamina() : -1.f,
        AttributeSet ? AttributeSet->GetMaxStamina() : -1.f);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Dash Commit Failed] Stamina: %.1f / %.1f"),
            AttributeSet ? AttributeSet->GetStamina() : -1.f,
            AttributeSet ? AttributeSet->GetMaxStamina() : -1.f);

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Dash After Commit] Stamina: %.1f / %.1f"),
        AttributeSet ? AttributeSet->GetStamina() : -1.f,
        AttributeSet ? AttributeSet->GetMaxStamina() : -1.f);

    ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get());
    UAbilityTask_PlayMontageAndWait* MontageTask;
    if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(STATE_COMBAT))
    {
        MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Daeva->GetMontageByID(EMontageID::CombatDash), 1.3f);
    }
    else
    {
        MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Daeva->GetMontageByID(EMontageID::Dash), 1.0f);
    }

    MontageTask->OnCompleted.AddDynamic(this, &UGA_Dash::OnDashFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_Dash::OnDashFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Dash::OnDashCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Dash::OnDashCancelled);

    MontageTask->ReadyForActivation();
}

void UGA_Dash::OnDashFinished()
{
    UE_LOG(LogTemp, Warning, TEXT("[Dash Finished]"));

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Dash::OnDashCancelled()
{
    UE_LOG(LogTemp, Warning, TEXT("[Dash Cancelled]"));

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}