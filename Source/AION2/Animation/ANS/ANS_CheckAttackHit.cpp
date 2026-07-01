// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ANS/ANS_CheckAttackHit.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/AOGameplayTags.h"


void UANS_CheckAttackHit::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* OwnerActor = MeshComp->GetOwner();

	if (OwnerActor == nullptr)
	{
		return;
	}

	if (OwnerActor->HasAuthority() == false)
	{
		return;
	}

	if (MeshComp)
	{
		AActor* OwnerActor = MeshComp->GetOwner();
		if (OwnerActor)
		{
			FGameplayEventData PayloadData;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EVENT_CHECKATTACKHIT_BEGIN, PayloadData);
		}
	}


}

void UANS_CheckAttackHit::NotifyTick(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	float FrameDeltaTime, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

}

void UANS_CheckAttackHit::NotifyEnd(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	const FAnimNotifyEventReference& EventReference)
{

	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		AActor* OwnerActor = MeshComp->GetOwner();
		if (OwnerActor)
		{
			FGameplayEventData PayloadData;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EVENT_CHECKATTACKHIT_END, PayloadData);
		}
	}



}
