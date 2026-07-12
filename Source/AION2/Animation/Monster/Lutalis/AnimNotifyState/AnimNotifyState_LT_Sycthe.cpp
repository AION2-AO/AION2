// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Monster/Lutalis/AnimNotifyState/AnimNotifyState_LT_Sycthe.h"
#include "Character/Monster/Boss/Lutalis/Lutalis.h"

void UAnimNotifyState_LT_Sycthe::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	ALutalis* Lutalis = Cast<ALutalis>(OwnerActor);
	if (!Lutalis)
	{
		return;
	}

	Lutalis->BeginScytheWarning(TotalDuration);
}

void UAnimNotifyState_LT_Sycthe::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
}

void UAnimNotifyState_LT_Sycthe::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	ALutalis* Lutalis = Cast<ALutalis>(OwnerActor);
	if (!Lutalis)
	{
		return;
	}

	Lutalis->ActivateScytheSweep();
}
