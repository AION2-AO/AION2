// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Monster/GA_Monster_Attack.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/AOGameplayTags.h"

void UGA_Monster_Attack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AAOMonsterBase* pMonster = CastChecked<AAOMonsterBase>(ActorInfo->AvatarActor.Get());
    
    //pMonster->OnAbilityFinishedEvent.AddDynamic(this, &UGA_Monster_Attack::AbilityEnd);


    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, pMonster->GetMontageByTag(MontageTag));
    MontageTask->OnCompleted.AddDynamic(this, &UGA_Monster_Attack::OnMontageTaskFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_Monster_Attack::OnMontageTaskFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Monster_Attack::OnMontageTaskCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Monster_Attack::OnMontageTaskCancelled);
    MontageTask->ReadyForActivation();
    

    UAbilityTask_WaitGameplayEvent* WaitHitCheckTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EVENT_CHECKATTACKHIT);
    WaitHitCheckTask->EventReceived.AddDynamic(this, &UGA_Monster_Attack::OnCheckAttackHitEvent);
    WaitHitCheckTask->ReadyForActivation();

}

void UGA_Monster_Attack::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);




}


void UGA_Monster_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    AAOMonsterBase* pMonster = CastChecked<AAOMonsterBase>(ActorInfo->AvatarActor.Get());

   


}



void UGA_Monster_Attack::OnMontageTaskFinished()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Monster_Attack::OnMontageTaskCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}


void UGA_Monster_Attack::OnCheckAttackHitEvent(FGameplayEventData Payload)
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



    AOCharacter->CheckAttackHit(AttackData);
}

void UGA_Monster_Attack::AbilityEnd()
{



}

