// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/WaterEle/AIWaterEleController.h"
#include "Character/Monster/WaterEle/WaterEle.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "GAS/AOGameplayTags.h"

AAIWaterEleController::AAIWaterEleController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAIWaterEleController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)
	{
		return;
	}
	ControlledWaterEle = Cast<AWaterEle>(ControlledMonster);
	ControlledMonsterAttributeSet = ControlledWaterEle->GetAttributeSet();
}

void AAIWaterEleController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() == false)
	{
		return;
	}

	if (ControlledMonsterAttributeSet->GetHealth() <= 0)
	{
		PhaseTag = PHASE_MONSTER_DEAD;
	}
}

void AAIWaterEleController::OnPossess(APawn* InPawn)
{
	Super::Possess(InPawn);

	if (HasAuthority())
	{
		ControlledWaterEle = Cast<AWaterEle>(InPawn);
	}
}
