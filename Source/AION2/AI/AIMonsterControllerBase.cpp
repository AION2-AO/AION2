// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIMonsterControllerBase.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Character/Monster/AOMonsterBase.h"
#include "GAS/AOGameplayTags.h"

AAIMonsterControllerBase::AAIMonsterControllerBase()
{
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
}

void AAIMonsterControllerBase::BeginPlay()
{
	Super::BeginPlay(); 


	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&AAIMonsterControllerBase::TargetPerceptionOn
		);
	}


	if (HasAuthority() == false)
	{
		return;
	}

	// 이거 초기값 설정해주기 
	PhaseTag = PHASE_MONSTER_OUTOFCOMBAT;

	if (StateTreeAIComponent)
	{
		StateTreeAIComponent->StartLogic();
	}


}

void AAIMonsterControllerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds); 

	if (HasAuthority())
	{
		if (ControlledMonster && CurrentTargetPlayer)
		{
			DistanceToTarget = FVector::Dist(
				ControlledMonster->GetActorLocation(),	
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
		ControlledMonster->Set_Phase(PhaseTag);
		ControlledMonster->Set_State(StateTag);
	}


}

void AAIMonsterControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);


	ControlledMonster = Cast<AAOMonsterBase>(GetPawn());

}


void AAIMonsterControllerBase::TargetPerceptionOn(AActor* Actor, FAIStimulus Stimlus)
{

	if (HasAuthority())
	{
		if (HasDetectedTarget == false)
		{
			HasDetectedTarget = true;

			PhaseTag = PHASE_MONSTER_PRECOMBAT;

			ControlledMonster->Set_Phase(PHASE_MONSTER_PRECOMBAT);

			ArrayTargetPlayers.Add(Actor);

			CurrentTargetPlayer = Actor;
		}

	}

}