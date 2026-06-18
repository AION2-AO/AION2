// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AITalythraAIController.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"




AAITalythraAIController::AAITalythraAIController()
{
	PrimaryActorTick.bCanEverTick = true; 


	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	



}

void AAITalythraAIController::BeginPlay()
{
	Super::BeginPlay();

	if(AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&AAITalythraAIController::TargetPerceptionOn
		);
	}


	 if(HasAuthority() == false)
	 {
		 return; 
	 }

	 Phase = ETalythraPhase::OutOfCombat;

	 if(StateTreeAIComponent)
	 {
	 	StateTreeAIComponent->StartLogic();
	 }

	



}

void AAITalythraAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 나중에는 target player를 랜덤으로 잡아줘도 될듯함.

	if(HasAuthority())
	{
		if (ControlledTalythra && CurrentTargetPlayer)
		{
			DistanceToTarget = FVector::Dist(
				ControlledTalythra->GetActorLocation(),
				CurrentTargetPlayer->GetActorLocation()
			);
		}
		else
		{
			DistanceToTarget = TNumericLimits<float>::Max();
		}

	}

	if (HasAuthority())
	{
		ControlledTalythra->Set_Phase(Phase);
		ControlledTalythra->Set_State(State);

	}

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

void AAITalythraAIController::TargetPerceptionOn(AActor* Actor, FAIStimulus Stimlus)
{

	if (HasAuthority())
	{
		if (HasDetectedTarget == false)
		{
			HasDetectedTarget = true;

			Phase = ETalythraPhase::PreCombat;

			ControlledTalythra->Set_Phase(ETalythraPhase::PreCombat);

			ArrayTargetPlayers.Add(Actor);

			CurrentTargetPlayer = Actor; 
		}

	}
	
}
