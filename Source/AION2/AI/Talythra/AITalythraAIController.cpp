// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Talythra/AITalythraAIController.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "GAS/AOGameplayTags.h"




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

