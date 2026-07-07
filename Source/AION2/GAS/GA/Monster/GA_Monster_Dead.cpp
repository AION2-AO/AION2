// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Monster_Dead.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/AOGameplayTags.h"

void UGA_Monster_Dead::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AAOMonsterBase* pMonster = CastChecked<AAOMonsterBase>(ActorInfo->AvatarActor.Get());
    
    pMonster->OnDeadMontageEnd.AddDynamic(pMonster, &AAOMonsterBase::HandleBossDeathMontageEnd);


    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, pMonster->GetMontageByTag(MontageTag), FMath::Max(MontagePlayRate, 1.0f));
    MontageTask->OnCompleted.AddDynamic(this, &UGA_Monster_Dead::OnMontageTaskFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_Monster_Dead::OnMontageTaskFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Monster_Dead::OnMontageTaskCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Monster_Dead::OnMontageTaskCancelled);
    MontageTask->ReadyForActivation();
    

}

void UGA_Monster_Dead::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);




}


void UGA_Monster_Dead::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    AAOMonsterBase* pMonster = CastChecked<AAOMonsterBase>(ActorInfo->AvatarActor.Get());

    pMonster->OnDeadMontageEnd.Broadcast();


}



void UGA_Monster_Dead::OnMontageTaskFinished()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Monster_Dead::OnMontageTaskCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}



void UGA_Monster_Dead::AbilityEnd()
{



}

