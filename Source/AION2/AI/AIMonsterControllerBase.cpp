// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIMonsterControllerBase.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Character/Monster/AOMonsterBase.h"
#include "GAS/AOGameplayTags.h"
#include "Character/Daeva/Daeva.h"

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
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(
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

	if (HasAuthority() == false)
		return;

	if (ControlledMonster == nullptr)
		return;

	if (CurrentTargetPlayer)
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

	if (ControlledMonster->IsDead())
	{
		PhaseTag = PHASE_MONSTER_DEAD;
	}

	ControlledMonster->Set_Phase(PhaseTag);
	ControlledMonster->Set_State(StateTag);
}

void AAIMonsterControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);


	ControlledMonster = Cast<AAOMonsterBase>(GetPawn());

}

void AAIMonsterControllerBase::RemoveTarget(AActor* Target)
{
	if (ADaeva* pPlayer = Cast<ADaeva>(Target))
	{
		pPlayer->OnPlayerDead.RemoveDynamic(this, &AAIMonsterControllerBase::OnTargetDead);
	}
	ArrayTargetPlayers.Remove(Target);
}

bool AAIMonsterControllerBase::RefreshOrReset()
{
	// 배열에 살아있는 후보가 남아있으면 그 중 하나 선택
	if (ArrayTargetPlayers.Num() == 0)
	{
		if (RefreshPerceivedTargets() == false)
		{
			// 보스 HP 및 Groggy 게이지 초기화. 
			UAOAttributeSet* pAttributeSet = ControlledMonster->GetAttributeSet();
			pAttributeSet->SetHealth(pAttributeSet->GetMaxHealth());
			pAttributeSet->SetGroggy(pAttributeSet->GetMaxGroggy());
			
			if (ControlledMonster->Get_GimmickArray() != nullptr)
			{
				TArray<AAOMonsterBase::FGimmickEntry>* ArrayGimmick = ControlledMonster->Get_GimmickArray();

				for (auto& iter : *ArrayGimmick)
				{
					iter.bTriggered = false;
				}
			}

			
			// =================================



			Set_Phase(PHASE_MONSTER_RESET);	
			Set_State(STATE_MONSTER_TH_IDLE);
			CurrentTargetPlayer = nullptr;
			HasDetectedTarget = false;
			return false;
		}
	}

	CurrentTargetPlayer = ArrayTargetPlayers[0];
	return true;
}


// 감지만 하고 현재 타겟은 설정 x 
void AAIMonsterControllerBase::TargetPerceptionOn(AActor* Actor, FAIStimulus Stimlus)
{
	// 로직 관련 설명
	// 보스가 Target으로 설정한 플레이어가 죽었다면, 
	// 해당 플레이어에서는 Controller 클래스의 OnTargetDead 함수를 호출하여 
	// 현재 범위안에 있는 플레이어를 기반으로 다시 Target을 설정 


	if (HasAuthority() == false) // 타겟 인식은 서버에서만 할 수 있게 설정. 
		return;

	ADaeva* pPlayer = Cast<ADaeva>(Actor);
	if (pPlayer == nullptr || pPlayer->IsDead() == true)
	{
		return;
	}


	// 감지 범위 안에 들어오거나 감지 상태가 갱신될때 마다 호출됨 
	if (Stimlus.WasSuccessfullySensed())
	{
		// 처음 보스를 마주할 때 보스가 전투준비 페이즈로 갈 수 있게 설정. 
		if (HasDetectedTarget == false)
		{
			PhaseTag = PHASE_MONSTER_PRECOMBAT;
			CurrentTargetPlayer = Actor;
			HasDetectedTarget = true;
		}

		// 해당 플레이어가 시야에 처음 인식되었을 때 
		if (ArrayTargetPlayers.Find(Actor) == -1)
		{
			pPlayer->OnPlayerDead.AddUniqueDynamic(this, &AAIMonsterControllerBase::OnTargetDead);
			ArrayTargetPlayers.Add(Actor);
		}
	}

	// 감지 범위 밖으로 벗어나 감지가 실패했을 때 
	else
	{
		// 이거 있으면 어쌔신 뒤잡기할때 타겟 목록에 사라져서 문제 생김. 
		// 시야에서 완전히 벗어남 → 후보에서 제거 + 언바인딩
		//
		//RemoveTarget(Actor);
		//
		//// 벗어난 게 현재 타겟이면 갱신
		//if (Actor == CurrentTargetPlayer)
		//{
		//	RefreshOrReset();
		//}
	}


}



void AAIMonsterControllerBase::OnTargetDead(AActor* DeadActor)
{
	RemoveTarget(DeadActor);

	if (DeadActor != CurrentTargetPlayer)
		return;

	RefreshOrReset();

}



bool AAIMonsterControllerBase::RefreshPerceivedTargets()
{
	// AI Perception에 현재 감지되고 있는 오브젝트들을 모두 배열에 넣어 놓고 
	// 해당 오브젝트가 데바인지 확인하고 
	// 데바라면, 다시 상태가 죽음 ( 알 상태) 인지 확인하고 
	// 죽음상태가 아니라면, 이미 TargetPlayer 배열에 들어갔는지 확인하고 
	// 이 두가지 조건이 다 아니라면, player를 arrayTarget에 추가해주고 델리게이트 이벤트 바인딩.



	TArray<AActor*> PerceivedActors;
	// 시야로 현재 감지 중인 엑터만 가져오기 
	AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	for (AActor* Actor : PerceivedActors)
	{
		ADaeva* pPlayer = Cast<ADaeva>(Actor);

		if (pPlayer == nullptr)
			continue;

		if (ArrayTargetPlayers.Find(pPlayer) == -1 && pPlayer->IsDead() == false)
		{
			pPlayer->OnPlayerDead.AddUniqueDynamic(this, &AAIMonsterControllerBase::OnTargetDead);
			ArrayTargetPlayers.Add(pPlayer);
		}
	}

	return ArrayTargetPlayers.Num() > 0;

}

void AAIMonsterControllerBase::ChangeCurrentTargetPlayer()
{
	if (ArrayTargetPlayers.IsEmpty() || !ControlledMonster)
	{
		return;
	}

	CurrentTargetPlayer = ArrayTargetPlayers[FMath::RandRange(0, ArrayTargetPlayers.Num() - 1)];
	if (AAOCharacter* TargetPlayer = Cast<AAOCharacter>(CurrentTargetPlayer))
	{
		ControlledMonster->SetCurrentTarget(TargetPlayer);
	}
}

void AAIMonsterControllerBase::ChangeCurrentTargetPlayerByNearest()
{
	if (ArrayTargetPlayers.IsEmpty() || !ControlledMonster)
	{
		return;
	}

	AActor* FarthestTarget = nullptr;
	float MinDistSq = 123456789.f;

	const FVector MonsterLocation = ControlledMonster->GetActorLocation();

	for (AActor* Target : ArrayTargetPlayers)
	{
		if (!IsValid(Target))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(MonsterLocation, Target->GetActorLocation());

		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			FarthestTarget = Target;
		}
	}

	CurrentTargetPlayer = FarthestTarget;
	if (AAOCharacter* TargetPlayer = Cast<AAOCharacter>(CurrentTargetPlayer))
	{
		ControlledMonster->SetCurrentTarget(TargetPlayer);
	}
}

void AAIMonsterControllerBase::ChangeCurrentTargetPlayerByFarthest()
{
	if (ArrayTargetPlayers.IsEmpty() || !ControlledMonster)
	{
		return;
	}

	AActor* FarthestTarget = nullptr;
	float MaxDistSq = -1.f;

	const FVector MonsterLocation = ControlledMonster->GetActorLocation();

	for (AActor* Target : ArrayTargetPlayers)
	{
		if (!IsValid(Target))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(MonsterLocation, Target->GetActorLocation());

		if (DistSq > MaxDistSq)
		{
			MaxDistSq = DistSq;
			FarthestTarget = Target;
		}
	}

	CurrentTargetPlayer = FarthestTarget;
	if (AAOCharacter* TargetPlayer = Cast<AAOCharacter>(CurrentTargetPlayer))
	{
		ControlledMonster->SetCurrentTarget(TargetPlayer);
	}
}
