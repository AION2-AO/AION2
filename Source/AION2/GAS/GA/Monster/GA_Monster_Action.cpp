// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Monster_Action.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/AOGameplayTags.h"

void UGA_Monster_Action::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AAOMonsterBase* pMonster = CastChecked<AAOMonsterBase>(ActorInfo->AvatarActor.Get());
    
    //pMonster->OnAbilityFinishedEvent.AddDynamic(this, &UGA_Monster_Attack::AbilityEnd);


    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, pMonster->GetMontageByTag(MontageTag), FMath::Max(MontagePlayRate, 1.0f));
    MontageTask->OnCompleted.AddDynamic(this, &UGA_Monster_Action::OnMontageTaskFinished);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_Monster_Action::OnMontageTaskFinished);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Monster_Action::OnMontageTaskCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Monster_Action::OnMontageTaskCancelled);
    MontageTask->ReadyForActivation();
    

}

void UGA_Monster_Action::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);




}


void UGA_Monster_Action::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    AAOMonsterBase* pMonster = CastChecked<AAOMonsterBase>(ActorInfo->AvatarActor.Get());

   


}



void UGA_Monster_Action::OnMontageTaskFinished()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Monster_Action::OnMontageTaskCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}



void UGA_Monster_Action::AbilityEnd()
{



}

