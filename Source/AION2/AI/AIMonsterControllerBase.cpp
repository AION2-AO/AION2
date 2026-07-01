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
		AAOCharacter* pPlayer = Cast<AAOCharacter>(Actor);
		if (pPlayer == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("pPlayer nullptr"));
		}

		// 그러면 Deva에서 interface만들어서 해당 플레이어의 맴버변수 bisDead가 true라면, 
		// 해당플레이어를 타겟에서 제거하면 될듯. 

		// 해당 타겟이 이전에 발견하지 못했던 것이라면, 
		if (ArrayTargetPlayers.Find(Actor) == -1)
		{

			if (HasDetectedTarget == false)
			{
				PhaseTag = PHASE_MONSTER_PRECOMBAT;
				ControlledMonster->Set_Phase(PHASE_MONSTER_PRECOMBAT);
				CurrentTargetPlayer = Actor;



			}

			if (pPlayer->IsDead() == false)
			{
				pPlayer->OnPlayerDead.AddDynamic(this, &AAIMonsterControllerBase::OnTargetDead);
				ArrayTargetPlayers.Add(Actor);
				UE_LOG(LogTemp, Warning, TEXT("targetCount: %d"),
					ArrayTargetPlayers.Num());
			}

			HasDetectedTarget = true;
		}



		else
			return;
	}

}



void AAIMonsterControllerBase::OnTargetDead(AActor * DeadActor)
{

	// 타겟 리스트에서 제거해주는 작업

	ArrayTargetPlayers.Remove(DeadActor);


	UE_LOG(LogTemp, Warning, TEXT("targetCount_after_delete: %d"),
		ArrayTargetPlayers.Num());

	CurrentTargetPlayer = ArrayTargetPlayers[0];

}