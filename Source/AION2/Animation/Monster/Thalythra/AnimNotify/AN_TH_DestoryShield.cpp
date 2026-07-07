// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_TH_DestoryShield.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"

void UAN_TH_DestoryShield::Notify(
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


	// 여기서 소환했던 쉴드들 스폰해제. 
	Talythra->Destroy_OrbShield();

}