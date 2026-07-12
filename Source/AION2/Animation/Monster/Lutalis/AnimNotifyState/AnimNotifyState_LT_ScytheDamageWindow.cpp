// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Monster/Lutalis/AnimNotifyState/AnimNotifyState_LT_ScytheDamageWindow.h"

#include "Character/Monster/Boss/Lutalis/LutalisScytheZone.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_LT_ScytheDamageWindow::NotifyBegin(
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

	ALutalisScytheZone* ScytheZone = Cast<ALutalisScytheZone>(OwnerActor);
	if (!ScytheZone)
	{
		return;
	}

	ScytheZone->BeginDamageWindow();
}

void UAnimNotifyState_LT_ScytheDamageWindow::NotifyEnd(
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

	ALutalisScytheZone* ScytheZone = Cast<ALutalisScytheZone>(OwnerActor);
	if (!ScytheZone)
	{
		return;
	}

	ScytheZone->EndDamageWindow();
}
