// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_WaveCircleRed.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"

void UAN_WaveCircleRed::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{



	Super::Notify(MeshComp, Animation, EventReference);


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

	Talythra->SpawnWaveRed();

}
