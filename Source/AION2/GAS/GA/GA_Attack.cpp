#include "GAS/GA/GA_Attack.h"
#include "Character/Daeva/Daeva.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"

void UGA_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    for (const TSubclassOf<UGameplayEffect> GameplayEffect : GameplayEffectsToApply)
    {
        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, 1.f);
        ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
    }

    if (!RemoveTagsOnActivate.IsEmpty())
    {
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
        {
            ASC->RemoveActiveEffectsWithGrantedTags(RemoveTagsOnActivate);
        }
    }

    ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get());
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Daeva->GetMontageByID(MontageIDToPlay), MontagePlayRate, StartSectionName);

    MontageTask->OnCompleted.AddDynamic(this, &UGA_Attack::OnMontageTaskFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_Attack::OnMontageTaskFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Attack::OnMontageTaskCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Attack::OnMontageTaskCancelled);
    MontageTask->ReadyForActivation();
}

void UGA_Attack::OnMontageTaskFinished()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Attack::OnMontageTaskCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
