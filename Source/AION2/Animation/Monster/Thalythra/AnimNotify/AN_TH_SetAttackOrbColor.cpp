// Fill out your copyright notice in the Description page of Project Settings.

#include "AN_TH_SetAttackOrbColor.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"

void UAN_TH_SetAttackOrbColor::Notify(
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


	// 여기서 오브 소환 
	Talythra->Set_OrbAttackColor(AttackOrbColor);

}
