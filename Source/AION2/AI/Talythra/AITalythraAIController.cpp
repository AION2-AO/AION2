// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Talythra/AITalythraAIController.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"




AAITalythraAIController::AAITalythraAIController()
{
	PrimaryActorTick.bCanEverTick = true; 


}

void AAITalythraAIController::BeginPlay()
{
	Super::BeginPlay();


	 if(HasAuthority() == false)
	 {
		 return; 
	 }


	 ControlledTalythra = Cast<ATalythra>(ControlledMonster);

}

void AAITalythraAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 나중에는 target player를 랜덤으로 잡아줘도 될듯함.

}


void AAITalythraAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	//서버에서만 실행
	
	if(HasAuthority())
	{
		ControlledTalythra = Cast<ATalythra>(InPawn);

		if (ControlledTalythra != NULL)
		{
			UE_LOG(LogTemp, Warning, TEXT("TalythraAIController possessed invalid pawn."));
		}

	}
}

