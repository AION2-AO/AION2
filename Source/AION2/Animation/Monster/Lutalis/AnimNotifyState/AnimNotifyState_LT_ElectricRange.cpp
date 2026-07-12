// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotifyState_LT_ElectricRange.h"

#include "Character/Monster/Boss/Lutalis/Lutalis.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UAnimNotifyState_LT_ElectricRange::NotifyBegin(
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
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	ALutalis* Lutalis = Cast<ALutalis>(OwnerActor);
	if (!Lutalis)
	{
		return;
	}

	Lutalis->BeginElectricRangeWarning(TotalDuration);
}

void UAnimNotifyState_LT_ElectricRange::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
}

void UAnimNotifyState_LT_ElectricRange::NotifyEnd(
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
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	ALutalis* Lutalis = Cast<ALutalis>(OwnerActor);
	if (!Lutalis)
	{
		return;
	}

	Lutalis->ActivateElectricRangeDamage();
}
