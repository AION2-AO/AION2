// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_TH_SpawnZone.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"


void UAnimNotifyState_TH_SpawnZone::NotifyBegin(
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


	Talythra->Render_PlayerAoeOnOff(true);
	


}

void UAnimNotifyState_TH_SpawnZone::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, 
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);




}

void UAnimNotifyState_TH_SpawnZone::NotifyEnd(
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


	
	Talythra->Render_PlayerAoeOnOff(false);
	Talythra->Player_Orb_RenderOnOff(false);


	// 여기서 쉴드 소환 및 해당 쉴드의 색깔 전달
	Talythra->SpawnColorSheid();
	Talythra->Reset_PlayerOrbStackAndColor();



}
