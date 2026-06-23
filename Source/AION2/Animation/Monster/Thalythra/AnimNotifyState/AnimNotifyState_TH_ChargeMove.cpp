// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_TH_ChargeMove.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAnimNotifyState_TH_ChargeMove::NotifyBegin(
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


	FVector MoveDir = Talythra->GetActorForwardVector(); 
	MoveDir.Z = 0.f; 

	

	Talythra->StartChargeMove();


}


void UAnimNotifyState_TH_ChargeMove::NotifyTick(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation,
	float FrameDeltaTime, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);


}

void UAnimNotifyState_TH_ChargeMove::NotifyEnd(
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


	Talythra->EndChargeMove();
}
