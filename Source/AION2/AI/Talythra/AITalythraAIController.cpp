// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Talythra/AITalythraAIController.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "GAS/AOGameplayTags.h"
#include "Perception/AIPerceptionTypes.h"
#include "Character/Daeva/Daeva.h"
#include "Game/DungeonGameState.h"



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
	 ControlledMonsterAttributeSet = ControlledTalythra->GetAttributeSet();
}

void AAITalythraAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 나중에는 target player를 랜덤으로 잡아줘도 될듯함.
		// 서버에서만 로직 호출할 수 있도록 설정 
	if (HasAuthority() == false)
		return;


	if (ControlledMonsterAttributeSet->GetHealth() <= 0)
	{
		PhaseTag = PHASE_MONSTER_DEAD;
	}



}


void AAITalythraAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	//서버에서만 실행
	
	if(HasAuthority())
	{
		ControlledTalythra = Cast<ATalythra>(InPawn);

	}
}

void AAITalythraAIController::TargetPerceptionOn(AActor* Actor, FAIStimulus Stimlus)
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

			// 보스 브금 재생 
			if (auto* GS = GetWorld()->GetGameState<ADungeonGameState>())
			{
				GS->SetMusic(EDungeonMusic::Talythra);
			}
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

	}

}