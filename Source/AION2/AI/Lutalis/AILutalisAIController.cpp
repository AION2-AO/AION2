// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Lutalis/AILutalisAIController.h"
#include "Character/Monster/Boss/Lutalis/Lutalis.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "GAS/AOGameplayTags.h"
#include "Perception/AIPerceptionTypes.h"
#include "AbilitySystemComponent.h"
#include "Game/DungeonGameState.h"


AAILutalisAIController::AAILutalisAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAILutalisAIController::TargetPerceptionOn(AActor* Actor, FAIStimulus Stimlus)
{
	if (HasAuthority())
	{
		if (HasDetectedTarget == false)
		{
			HasDetectedTarget = true;
	
			PhaseTag = PHASE_MONSTER_COMBAT;
			ControlledMonster->Set_Phase(PHASE_MONSTER_COMBAT);
	
			ArrayTargetPlayers.Add(Actor);
			CurrentTargetPlayer = Actor;

			// 보스 브금 재생 
			if (auto* GS = GetWorld()->GetGameState<ADungeonGameState>())
			{
				GS->SetMusic(EDungeonMusic::Boss);
			}

		}
	}
}


void AAILutalisAIController::BeginPlay()
{
	Super::BeginPlay();


	if (HasAuthority() == false)
	{
		return;
	}

	ControlledLutails = Cast<ALutalis>(ControlledMonster);
	if (IsValid(ControlledLutails))
	{
		ControlledMonsterAttributeSet = ControlledLutails->GetAttributeSet();
	}

	AddElectricZoneRequestTag();
}

void AAILutalisAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() == false)
		return;

	if (!IsValid(ControlledMonsterAttributeSet))
		return;

	if (ControlledMonsterAttributeSet->GetHealth() <= 0)
	{
		PhaseTag = PHASE_MONSTER_DEAD;
		return;
	}

	TryRequestElectricZoneByHealth();
}

void AAILutalisAIController::TryRequestElectricZoneByHealth()
{
	if (!PhaseTag.MatchesTagExact(PHASE_MONSTER_COMBAT))
		return;

	const float MaxHealth = ControlledMonsterAttributeSet->GetMaxHealth();
	if (MaxHealth <= 0.0f)
		return;

	const float HealthRatio =
		ControlledMonsterAttributeSet->GetHealth() / MaxHealth;

	if (ElectricZoneTriggerStep == 0 && HealthRatio <= (2.0f / 3.0f))
	{
		if (AddElectricZoneRequestTag())
		{
			ElectricZoneTriggerStep = 1;
		}

		return;
	}

	if (ElectricZoneTriggerStep == 1 && HealthRatio <= (1.0f / 3.0f))
	{
		if (AddElectricZoneRequestTag())
		{
			ElectricZoneTriggerStep = 2;
		}

		return;
	}
}

bool AAILutalisAIController::AddElectricZoneRequestTag()
{
	if (!IsValid(ControlledLutails))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ElectricZone] ControlledLutails is invalid"));
		return false;
	}

	UAbilitySystemComponent* ASC = ControlledLutails->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ElectricZone] ASC is null"));
		return false;
	}

	const FGameplayTag ElectricZoneRequestTag =
		FGameplayTag::RequestGameplayTag(TEXT("State.Monster.LT.ElectricZone"));

	if (ASC->HasMatchingGameplayTag(ElectricZoneRequestTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ElectricZone] Tag already exists"));
		return false;
	}

	ASC->AddLooseGameplayTag(ElectricZoneRequestTag);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[ElectricZone] Add Tag Success: %s / HasTag=%d"),
		*ElectricZoneRequestTag.ToString(),
		ASC->HasMatchingGameplayTag(ElectricZoneRequestTag)
	);

	return true;
}

void AAILutalisAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HasAuthority())
	{
		ControlledLutails = Cast<ALutalis>(InPawn);
	}
}

