// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_TH_WaveAoe.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"



void UAnimNotifyState_TH_WaveAoe::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, 
	float TotalDuration, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();

	if (OwnerActor == nullptr)
	{
		return;
	}

	if (OwnerActor->HasAuthority() == false)
	{
		return;
	}


	ATalythra* Talythra = Cast<ATalythra>(OwnerActor);
	if (!Talythra)
	{
		return;
	}


	Talythra->Set_AoeScale(10.f);
	Talythra->Set_AoeWaringTargetScale(10.f);
	Talythra->Set_AoeWaringDuartion(1.3f);
	Talythra->Attack_RangeRender(true);

}

void UAnimNotifyState_TH_WaveAoe::NotifyTick(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);



}

void UAnimNotifyState_TH_WaveAoe::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);



	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();

	if (OwnerActor == nullptr)
	{
		return;
	}

	if (OwnerActor->HasAuthority() == false)
	{
		return;
	}

	ATalythra* Talythra = Cast<ATalythra>(OwnerActor);
	if (!Talythra)
	{
		return;
	}


}
