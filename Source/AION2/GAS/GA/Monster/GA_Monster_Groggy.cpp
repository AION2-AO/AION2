// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Monster_Groggy.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/AOGameplayTags.h"


void UGA_Monster_Groggy::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	AAOMonsterBase* pMonster = CastChecked<AAOMonsterBase>(ActorInfo->AvatarActor.Get());

	pMonster->OnGroggyMontageEnd.AddDynamic(pMonster, &AAOMonsterBase::EndGroggy);


	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, pMonster->GetMontageByTag(MontageTag), FMath::Max(MontagePlayRate, 1.0f));
	MontageTask->OnCompleted.AddDynamic(this, &UGA_Monster_Groggy::OnMontageTaskFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Monster_Groggy::OnMontageTaskFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Monster_Groggy::OnMontageTaskCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Monster_Groggy::OnMontageTaskCancelled);
	MontageTask->ReadyForActivation();




}

void UGA_Monster_Groggy::CancelAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);



}

void UGA_Monster_Groggy::EndAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{

	AAOMonsterBase* pMonster = CastChecked<AAOMonsterBase>(ActorInfo->AvatarActor.Get());

	pMonster->OnGroggyMontageEnd.Broadcast();
	pMonster->OnGroggyMontageEnd.RemoveDynamic(pMonster, &AAOMonsterBase::EndGroggy);


	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

}


void UGA_Monster_Groggy::OnMontageTaskFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Monster_Groggy::OnMontageTaskCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}



void UGA_Monster_Groggy::AbilityEnd()
{


}

