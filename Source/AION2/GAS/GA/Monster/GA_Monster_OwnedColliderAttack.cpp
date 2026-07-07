#include "GAS/GA/Monster/GA_Monster_OwnedColliderAttack.h"
#include "GAS/AOGameplayTags.h"
#include "Character/AOCharacter.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

void UGA_Monster_OwnedColliderAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilityTask_WaitGameplayEvent* WaitHitCheckBeinTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EVENT_CHECKATTACKHIT_BEGIN);
	WaitHitCheckBeinTask->EventReceived.AddDynamic(this, &ThisClass::OnHitCheckBegin);
	WaitHitCheckBeinTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitHitCheckEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EVENT_CHECKATTACKHIT_END);
	WaitHitCheckEndTask->EventReceived.AddDynamic(this, &ThisClass::OnHitCheckEnd);
	WaitHitCheckEndTask->ReadyForActivation();
}

void UGA_Monster_OwnedColliderAttack::OnHitCheckBegin(FGameplayEventData Payload)
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

void UGA_Monster_OwnedColliderAttack::OnHitCheckEnd(FGameplayEventData Payload)
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
