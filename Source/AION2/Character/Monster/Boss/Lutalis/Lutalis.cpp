// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Monster/Boss/Lutalis/Lutalis.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Character/Monster/Boss/Lutalis/LutalisElectricZone.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

#include "Character/Monster/Boss/Lutalis/LutalisScytheZone.h"
#include "AI/AIMonsterControllerBase.h"
#include "Character/Daeva/Daeva.h"
#include "DrawDebugHelpers.h"

#include "GAS/AOGameplayTags.h"

ALutalis::ALutalis(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DungeonBossIndex = 2;
}

void ALutalis::BeginPlay()
{
	Super::BeginPlay();

	InitAttributeSet();
}

void ALutalis::InitAttributeSet()
{
	Super::InitAttributeSet();

	AttributeSet->InitHealth(8000.0f);
	AttributeSet->InitMaxHealth(8000.0f);

	AttributeSet->InitGroggy(1800.f);
	AttributeSet->InitMaxGroggy(1800.f);
}

void ALutalis::EndGroggy()
{
	if (!HasAuthority() || !bIsGroggy || bIsDead)
	{
		return;
	}

	bIsGroggy = false;

	if (AttributeSet)
	{
		AttributeSet->SetGroggy(AttributeSet->GetMaxGroggy());
	}



	AAIMonsterControllerBase* pMonsterController = Cast<AAIMonsterControllerBase>(GetController());
	if (pMonsterController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("pMonsterController nullptr"));
	}

	// 만약 PreCombat 페이즈를 안쓰신다면 EndGroggy를 virtual 함수로 선언하신 뒤 
	// Set_Phase를 다른걸로 사용하시면 될 거 같습니다.
	pMonsterController->Set_Phase(PHASE_MONSTER_COMBAT);
}

bool ALutalis::BeginElectricRangeWarning(float WarningDuration)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (PreparedElectricZoneArcIndex != INDEX_NONE)
	{
		return BeginPreparedElectricRangeWarning(WarningDuration);
	}

	if (!ResolveElectricZoneActor())
	{
		return false;
	}

	ElectricZoneActor->InitZone(
		this,
		ElectricZoneTickDamageData,
		ElectricZoneRadius,
		ElectricZoneSectorAngle
	);

	const int32 SelectedArcIndex = ElectricZoneActor->StartRandomArcWarning(WarningDuration);
	return SelectedArcIndex != INDEX_NONE;
}

bool ALutalis::BeginPreparedElectricRangeWarning(float WarningDuration)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!ElectricZoneActor)
	{
		return false;
	}

	if (PreparedElectricZoneArcIndex == INDEX_NONE)
	{
		return false;
	}

	const bool bStarted =
		ElectricZoneActor->StartArcWarning(PreparedElectricZoneArcIndex, WarningDuration);

	if (bStarted)
	{
		PreparedElectricZoneArcIndex = INDEX_NONE;
	}

	return bStarted;
}

bool ALutalis::ActivateElectricRangeDamage()
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!ElectricZoneActor)
	{
		return false;
	}

	return ElectricZoneActor->ActivatePendingArc();
}

// BP에서 작동 시킴.
void ALutalis::ConsumeElectricZoneRequestTag()
{
	if (!HasAuthority())
		return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
		return;

	const FGameplayTag ElectricZoneRequestTag =
		FGameplayTag::RequestGameplayTag(TEXT("State.Monster.LT.ElectricZone"));

	ASC->SetLooseGameplayTagCount(ElectricZoneRequestTag, 0);
}

bool ALutalis::PrepareElectricZoneCast()
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!ResolveElectricZoneActor())
	{
		return false;
	}

	ElectricZoneActor->InitZone(
		this,
		ElectricZoneTickDamageData,
		ElectricZoneRadius,
		ElectricZoneSectorAngle
	);

	PreparedElectricZoneArcIndex = ElectricZoneActor->SelectRandomUnusedArcIndex();

	if (PreparedElectricZoneArcIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Lutalis] Failed to select ElectricZone arc index."));
		return false;
	}

	float TargetYaw = 0.0f;
	if (!ElectricZoneActor->GetArcIndicatorForwardYaw(PreparedElectricZoneArcIndex, TargetYaw))
	{
		PreparedElectricZoneArcIndex = INDEX_NONE;
		return false;
	}

	PreparedElectricZoneTargetYaw = FRotator::NormalizeAxis(TargetYaw + ElectricZoneFacingYawOffset);

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	return true;
}

bool ALutalis::TickRotateToPreparedElectricZoneYaw(float DeltaTime)
{
	if (PreparedElectricZoneArcIndex == INDEX_NONE)
	{
		return false;
	}

	const FRotator CurrentRotation = GetActorRotation();

	FRotator TargetRotation = CurrentRotation;
	TargetRotation.Yaw = PreparedElectricZoneTargetYaw;

	const FRotator NewRotation = FMath::RInterpConstantTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		ElectricZoneRotateSpeed
	);

	SetActorRotation(NewRotation);

	if (AController* CurrentController = GetController())
	{
		CurrentController->SetControlRotation(NewRotation);
	}

	const float DeltaYaw = FMath::Abs(
		FMath::FindDeltaAngleDegrees(NewRotation.Yaw, PreparedElectricZoneTargetYaw)
	);

	if (DeltaYaw > ElectricZoneRotateTolerance)
	{
		return false;
	}

	SetActorRotation(TargetRotation);

	if (AController* CurrentController = GetController())
	{
		CurrentController->SetControlRotation(TargetRotation);
	}

	return true;
}

void ALutalis::ResetPreparedElectricZoneCast()
{
	PreparedElectricZoneArcIndex = INDEX_NONE;
	PreparedElectricZoneTargetYaw = 0.0f;
}

bool ALutalis::BeginScytheWarning(float WarningDuration)
{
	if (!HasAuthority() || !ScytheZoneClass)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveScytheZones.Reset();

	for (TActorIterator<ADaeva> It(World); It; ++It)
	{
		ADaeva* Target = *It;
		if (!IsValid(Target) || Target->IsDead())
		{
			continue;
		}

		FVector SpawnLocation = Target->GetActorLocation();

		FHitResult GroundHit;
		const FVector TraceStart = SpawnLocation + FVector(0.f, 0.f, 300.f);
		const FVector TraceEnd = SpawnLocation - FVector(0.f, 0.f, 3000.f);

		if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility))
		{
			SpawnLocation = GroundHit.ImpactPoint;
		}

		SpawnLocation += FVector(0.f, 0.f, 50.0f);

		const FRotator SpawnRotation(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

		ALutalisScytheZone* ScytheZone = World->SpawnActor<ALutalisScytheZone>(
			ScytheZoneClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		if (!ScytheZone)
		{
			continue;
		}

		ScytheZone->InitZone(this, ScytheDamageData, ScytheZoneLength, ScytheZoneWidth);
		ScytheZone->StartWarning(WarningDuration);
		ActiveScytheZones.Add(ScytheZone);
	}

	return ActiveScytheZones.Num() > 0;
}

bool ALutalis::ActivateScytheSweep()
{
	if (!HasAuthority() || ActiveScytheZones.IsEmpty())
	{
		return false;
	}

	bool bActivatedAny = false;
	for (ALutalisScytheZone* ScytheZone : ActiveScytheZones)
	{
		if (!IsValid(ScytheZone))
		{
			continue;
		}

		ScytheZone->ActivateSweep();
		bActivatedAny = true;
	}

	ActiveScytheZones.Reset();

	return bActivatedAny;
}

bool ALutalis::ResolveElectricZoneActor()
{
	if (ElectricZoneActor)
	{
		return true;
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ALutalisElectricZone> It(World); It; ++It)
		{
			ElectricZoneActor = *It;
			break;
		}
	}

	if (!ElectricZoneActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Lutalis] ElectricZoneActor is null."));
		return false;
	}

	return true;
}
