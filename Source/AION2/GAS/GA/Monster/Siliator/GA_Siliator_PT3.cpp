#include "GAS/GA/Monster/Siliator/GA_Siliator_PT3.h"
#include "Character/AOCharacter.h"
#include "GAS/AOGameplayTags.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

void UGA_Siliator_PT3::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    UAbilityTask_WaitGameplayEvent* WaitHitCheckBeinTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EVENT_CHECKATTACKHIT_BEGIN);
    WaitHitCheckBeinTask->EventReceived.AddDynamic(this, &ThisClass::OnHitCheckBegin);
    WaitHitCheckBeinTask->ReadyForActivation();

    UAbilityTask_WaitGameplayEvent* WaitHitCheckEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EVENT_CHECKATTACKHIT_END);
    WaitHitCheckEndTask->EventReceived.AddDynamic(this, &ThisClass::OnHitCheckEnd);
    WaitHitCheckEndTask->ReadyForActivation();
}

void UGA_Siliator_PT3::OnHitCheckBegin(FGameplayEventData Payload)
{
    if (!HasAuthority(&CurrentActivationInfo))
    {
        return;
    }

    AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
    if (!AOCharacter)
    {
        return;
    }

    AOCharacter->SetOwnedAttackCollidersCollisionEnabled(AttackData, true);
}

void UGA_Siliator_PT3::OnHitCheckEnd(FGameplayEventData Payload)
{
    if (!HasAuthority(&CurrentActivationInfo))
    {
        return;
    }

    AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
    if (!AOCharacter)
    {
        return;
    }

    AOCharacter->SetOwnedAttackCollidersCollisionEnabled(AttackData, false);
}

void UGA_Siliator_PT3::OnCheckAttackHitEvent(FGameplayEventData Payload)
{
    if (!HasAuthority(&CurrentActivationInfo))
    {
        return;
    }

    AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
    if (!AOCharacter)
    {
        return;
    }

    AOCharacter->CheckAttackHitSector(AttackData, 22.5f);
}
