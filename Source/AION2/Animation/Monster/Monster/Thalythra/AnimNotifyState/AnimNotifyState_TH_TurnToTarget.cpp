// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_TH_TurnToTarget.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"

void UAnimNotifyState_TH_TurnToTarget::NotifyBegin(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	float TotalDuration, 
	const FAnimNotifyEventReference& EventReference)
{

	/* 회전 시작 */


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



	Talythra->Set_RotationAble(true);
}

void UAnimNotifyState_TH_TurnToTarget::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime, 
	const FAnimNotifyEventReference& EventReference)
{

	/* 보간하면서 회전 */

	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);


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



	Talythra->TurnToTarget();

}

void UAnimNotifyState_TH_TurnToTarget::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{

	/* 회전 종료 */

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



	Talythra->Set_RotationAble(true);



}
