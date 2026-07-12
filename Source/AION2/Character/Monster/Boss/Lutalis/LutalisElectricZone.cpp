// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Monster/Boss/Lutalis/LutalisElectricZone.h"

#include "Character/AOCharacter.h"
#include "Character/Daeva/Daeva.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"

namespace
{
	constexpr int32 ElectricZoneArcCount = 4;
}

ALutalisElectricZone::ALutalisElectricZone()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	bReplicates = true;
	SetReplicateMovement(false);
}

void ALutalisElectricZone::BeginPlay()
{
	Super::BeginPlay();

	ResolveDamageCauser();
	RefreshArcReferenceCache();
	EnsureArcStateArrays();
	CreateArcMaterialInstance();

	for (int32 ArcIndex = 0; ArcIndex < GetArcCount(); ++ArcIndex)
	{
		SetArcWarningVisible(ArcIndex, false);
		SetArcActiveVisible(ArcIndex, false);
		SetArcFillAmount(ArcIndex, WarningStartFillAmount);
	}
}

void ALutalisElectricZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsAnyArcWarningInProgress())
	{
		UpdateWarningFill();
	}

	if (IsAnyArcActive())
	{
		UpdateActiveDamage(DeltaTime);
	}

	const bool bNeedTick = IsAnyArcWarningInProgress() || IsAnyArcActive();
	SetActorTickEnabled(bNeedTick);
}

void ALutalisElectricZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ALutalisElectricZone::InitZone(AAOCharacter* InDamageCauser, const FAttackData& InTickDamageData, float InRadius, float InSectorAngle)
{
	DamageCauser = InDamageCauser;
	TickDamageData = InTickDamageData;
	Radius = InRadius;
	SectorAngle = InSectorAngle;
}

int32 ALutalisElectricZone::StartRandomArcWarning(float InWarningDuration)
{
	if (!HasAuthority())
	{
		return INDEX_NONE;
	}

	RefreshArcReferenceCache();
	EnsureArcStateArrays();
	CreateArcMaterialInstance();

	const int32 ArcIndex = PickRandomUnusedArcIndex();

	if (ArcIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ElectricZone] No unused ArcIndicator left."));
		return INDEX_NONE;
	}

	MulticastStartArcWarning(ArcIndex, InWarningDuration);
	return ArcIndex;
}

bool ALutalisElectricZone::StartArcWarning(int32 ArcIndex, float InWarningDuration)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!IsValidArcIndex(ArcIndex))
	{
		return false;
	}

	MulticastStartArcWarning(ArcIndex, InWarningDuration);
	return true;
}

void ALutalisElectricZone::MulticastStartArcWarning_Implementation(int32 ArcIndex, float InWarningDuration)
{
	StartArcWarningInternal(ArcIndex, InWarningDuration);
}

void ALutalisElectricZone::StartArcWarningInternal(int32 ArcIndex, float InWarningDuration)
{
	RefreshArcReferenceCache();
	EnsureArcStateArrays();
	CreateArcMaterialInstance();

	if (!IsValidArcIndex(ArcIndex))
	{
		return;
	}

	if (bArcWarnings[ArcIndex] || bArcActives[ArcIndex])
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float FinalWarningDuration = InWarningDuration > 0.0f ? InWarningDuration : WarningDuration;

	bArcUsed[ArcIndex] = true;
	bArcWarnings[ArcIndex] = true;
	bArcWarningFilled[ArcIndex] = false;
	bArcActives[ArcIndex] = false;

	ArcWarningStartTimes[ArcIndex] = World->GetTimeSeconds();
	ArcWarningDurations[ArcIndex] = FinalWarningDuration;

	PendingArcIndex = ArcIndex;

	SetArcFillAmount(ArcIndex, WarningStartFillAmount);
	SetArcWarningVisible(ArcIndex, true);
	SetArcActiveVisible(ArcIndex, false);

	SetActorTickEnabled(true);

	UE_LOG(LogTemp, Log, TEXT("[ElectricZone] Start Warning. ArcIndex=%d Duration=%f"), ArcIndex, FinalWarningDuration);
}

bool ALutalisElectricZone::ActivatePendingArc()
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!IsValidArcIndex(PendingArcIndex))
	{
		return false;
	}

	const int32 ArcIndexToActivate = PendingArcIndex;
	MulticastActivateArc(ArcIndexToActivate);

	return true;
}

void ALutalisElectricZone::MulticastActivateArc_Implementation(int32 ArcIndex)
{
	ActivateArcInternal(ArcIndex);
}

void ALutalisElectricZone::ActivateArcInternal(int32 ArcIndex)
{
	EnsureArcStateArrays();

	if (!IsValidArcIndex(ArcIndex))
	{
		return;
	}

	bArcWarnings[ArcIndex] = false;
	bArcWarningFilled[ArcIndex] = false;
	bArcActives[ArcIndex] = true;

	bArcFirstDamageApplied[ArcIndex] = false;
	ArcDamageElapsedTimes[ArcIndex] = 0.0f;
	ArcActiveElapsedTimes[ArcIndex] = 0.0f;

	SetArcFillAmount(ArcIndex, WarningEndFillAmount);
	SetArcWarningVisible(ArcIndex, false);
	SetArcActiveVisible(ArcIndex, true);

	if (PendingArcIndex == ArcIndex)
	{
		PendingArcIndex = INDEX_NONE;
	}

	SetActorTickEnabled(true);

	UE_LOG(LogTemp, Log, TEXT("[ElectricZone] Activate Arc. ArcIndex=%d"), ArcIndex);
}

void ALutalisElectricZone::FinishAllArcs()
{
	if (HasAuthority())
	{
		MulticastFinishAllArcs();
	}
}

void ALutalisElectricZone::MulticastFinishAllArcs_Implementation()
{
	for (int32 ArcIndex = 0; ArcIndex < GetArcCount(); ++ArcIndex)
	{
		FinishArcInternal(ArcIndex);
	}

	SetActorTickEnabled(false);
}

void ALutalisElectricZone::ResetElectricZone()
{
	EnsureArcStateArrays();

	for (int32 ArcIndex = 0; ArcIndex < GetArcCount(); ++ArcIndex)
	{
		bArcUsed[ArcIndex] = false;
		FinishArcInternal(ArcIndex);
		SetArcFillAmount(ArcIndex, WarningStartFillAmount);
	}

	PendingArcIndex = INDEX_NONE;
	SetActorTickEnabled(false);
}

void ALutalisElectricZone::RefreshArcReferenceCache()
{
	ArcIndicatorActors =
	{
		ArcIndicator1,
		ArcIndicator2,
		ArcIndicator3,
		ArcIndicator4
	};

	ElectricZoneNS =
	{
		ElectricZoneNS1,
		ElectricZoneNS2,
		ElectricZoneNS3,
		ElectricZoneNS4
	};

	ArcIndicatorMeshes.SetNum(GetArcCount());

	for (int32 ArcIndex = 0; ArcIndex < GetArcCount(); ++ArcIndex)
	{
		ArcIndicatorMeshes[ArcIndex] = nullptr;

		AActor* IndicatorActor = ArcIndicatorActors.IsValidIndex(ArcIndex)
			? ArcIndicatorActors[ArcIndex]
			: nullptr;

		if (!IndicatorActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ElectricZone] ArcIndicator actor is null. Index=%d"), ArcIndex);
			continue;
		}

		UStaticMeshComponent* MeshComponent = IndicatorActor->FindComponentByClass<UStaticMeshComponent>();

		if (!MeshComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ElectricZone] ArcIndicator has no StaticMeshComponent. Index=%d Actor=%s"),
				ArcIndex,
				*IndicatorActor->GetName());
			continue;
		}

		ArcIndicatorMeshes[ArcIndex] = MeshComponent;
	}
}

void ALutalisElectricZone::EnsureArcStateArrays()
{
	const int32 Count = GetArcCount();

	if (bArcUsed.Num() != Count)
	{
		bArcUsed.Init(false, Count);
	}

	if (bArcWarnings.Num() != Count)
	{
		bArcWarnings.Init(false, Count);
	}

	if (bArcWarningFilled.Num() != Count)
	{
		bArcWarningFilled.Init(false, Count);
	}

	if (bArcActives.Num() != Count)
	{
		bArcActives.Init(false, Count);
	}

	if (bArcFirstDamageApplied.Num() != Count)
	{
		bArcFirstDamageApplied.Init(false, Count);
	}

	if (ArcWarningStartTimes.Num() != Count)
	{
		ArcWarningStartTimes.Init(0.0f, Count);
	}

	if (ArcWarningDurations.Num() != Count)
	{
		ArcWarningDurations.Init(WarningDuration, Count);
	}

	if (ArcActiveElapsedTimes.Num() != Count)
	{
		ArcActiveElapsedTimes.Init(0.0f, Count);
	}

	if (ArcDamageElapsedTimes.Num() != Count)
	{
		ArcDamageElapsedTimes.Init(0.0f, Count);
	}

	if (ArcMIDs.Num() != Count)
	{
		ArcMIDs.SetNum(Count);
	}
}

int32 ALutalisElectricZone::SelectRandomUnusedArcIndex()
{
	if (!HasAuthority())
	{
		return INDEX_NONE;
	}

	RefreshArcReferenceCache();
	EnsureArcStateArrays();
	CreateArcMaterialInstance();

	return PickRandomUnusedArcIndex();
}

void ALutalisElectricZone::CreateArcMaterialInstance()
{
	EnsureArcStateArrays();

	for (int32 ArcIndex = 0; ArcIndex < GetArcCount(); ++ArcIndex)
	{
		UStaticMeshComponent* Mesh = ArcIndicatorMeshes.IsValidIndex(ArcIndex)
			? ArcIndicatorMeshes[ArcIndex]
			: nullptr;

		if (!Mesh)
		{
			continue;
		}

		if (ArcMIDs[ArcIndex])
		{
			continue;
		}

		ArcMIDs[ArcIndex] = Mesh->CreateDynamicMaterialInstance(0);
	}
}

int32 ALutalisElectricZone::PickRandomUnusedArcIndex() const
{
	TArray<int32> Candidates;

	for (int32 ArcIndex = 0; ArcIndex < GetArcCount(); ++ArcIndex)
	{
		if (!bArcUsed.IsValidIndex(ArcIndex))
		{
			continue;
		}

		if (bArcUsed[ArcIndex])
		{
			continue;
		}

		if (bArcWarnings.IsValidIndex(ArcIndex) && bArcWarnings[ArcIndex])
		{
			continue;
		}

		if (bArcActives.IsValidIndex(ArcIndex) && bArcActives[ArcIndex])
		{
			continue;
		}

		Candidates.Add(ArcIndex);
	}

	if (Candidates.Num() <= 0)
	{
		return INDEX_NONE;
	}

	const int32 RandomArrayIndex = FMath::RandRange(0, Candidates.Num() - 1);
	return Candidates[RandomArrayIndex];
}

bool ALutalisElectricZone::GetArcIndicatorForwardYaw(int32 ArcIndex, float& OutYaw)
{
	RefreshArcReferenceCache();

	if (!ArcIndicatorMeshes.IsValidIndex(ArcIndex) || !ArcIndicatorMeshes[ArcIndex])
	{
		return false;
	}

	OutYaw = ArcIndicatorMeshes[ArcIndex]->GetForwardVector().Rotation().Yaw;
	return true;
}


void ALutalisElectricZone::UpdateWarningFill()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (int32 ArcIndex = 0; ArcIndex < GetArcCount(); ++ArcIndex)
	{
		if (!bArcWarnings.IsValidIndex(ArcIndex) || !bArcWarnings[ArcIndex])
		{
			continue;
		}

		const float Duration = FMath::Max(ArcWarningDurations[ArcIndex], KINDA_SMALL_NUMBER);
		const float Elapsed = World->GetTimeSeconds() - ArcWarningStartTimes[ArcIndex];

		const float Alpha = FMath::Clamp(Elapsed / Duration, 0.0f, 1.0f);
		const float FillAmount = FMath::Lerp(WarningStartFillAmount, WarningEndFillAmount, Alpha);

		SetArcFillAmount(ArcIndex, FillAmount);

		if (Alpha >= 1.0f)
		{
			SetArcFillAmount(ArcIndex, WarningEndFillAmount);

			bArcWarnings[ArcIndex] = false;
			bArcWarningFilled[ArcIndex] = true;
		}
	}
}

void ALutalisElectricZone::UpdateActiveDamage(float DeltaTime)
{
	for (int32 ArcIndex = 0; ArcIndex < GetArcCount(); ++ArcIndex)
	{
		if (!bArcActives.IsValidIndex(ArcIndex) || !bArcActives[ArcIndex])
		{
			continue;
		}

		ArcActiveElapsedTimes[ArcIndex] += DeltaTime;

		if (ActiveDuration > 0.0f && ArcActiveElapsedTimes[ArcIndex] >= ActiveDuration)
		{
			FinishArcInternal(ArcIndex);
			continue;
		}

		if (!HasAuthority())
		{
			continue;
		}

		if (!bArcFirstDamageApplied[ArcIndex])
		{
			ArcDamageElapsedTimes[ArcIndex] += DeltaTime;

			if (ArcDamageElapsedTimes[ArcIndex] < FMath::Max(FirstDamageDelay, 0.0f))
			{
				continue;
			}

			ApplyDamageTick(ArcIndex);

			bArcFirstDamageApplied[ArcIndex] = true;
			ArcDamageElapsedTimes[ArcIndex] = 0.0f;
			continue;
		}

		if (DamageInterval <= 0.0f)
		{
			ApplyDamageTick(ArcIndex);
			continue;
		}

		ArcDamageElapsedTimes[ArcIndex] += DeltaTime;

		if (ArcDamageElapsedTimes[ArcIndex] >= DamageInterval)
		{
			ApplyDamageTick(ArcIndex);
			ArcDamageElapsedTimes[ArcIndex] = 0.0f;
		}
	}
}

void ALutalisElectricZone::SetArcFillAmount(int32 ArcIndex, float FillAmount)
{
	UMaterialInstanceDynamic* ArcMID = ArcMIDs.IsValidIndex(ArcIndex)
		? ArcMIDs[ArcIndex]
		: nullptr;

	if (ArcMID)
	{
		ArcMID->SetScalarParameterValue(FillAmountParameterName, FillAmount);
	}
}

void ALutalisElectricZone::SetArcWarningVisible(int32 ArcIndex, bool bVisible)
{
	if (!ArcIndicatorMeshes.IsValidIndex(ArcIndex))
	{
		return;
	}

	UStaticMeshComponent* Mesh = ArcIndicatorMeshes[ArcIndex];
	if (!Mesh)
	{
		return;
	}

	Mesh->SetVisibility(bVisible, true);
	Mesh->SetHiddenInGame(!bVisible, true);
}

void ALutalisElectricZone::SetArcActiveVisible(int32 ArcIndex, bool bVisible)
{
	if (!ElectricZoneNS.IsValidIndex(ArcIndex))
	{
		return;
	}

	AActor* ElectricZoneActor = ElectricZoneNS[ArcIndex];
	if (!ElectricZoneActor)
	{
		return;
	}

	ElectricZoneActor->SetActorHiddenInGame(!bVisible);
	ElectricZoneActor->SetActorTickEnabled(bVisible);

	UNiagaraComponent* NiagaraComponent = ElectricZoneActor->FindComponentByClass<UNiagaraComponent>();
	if (!NiagaraComponent)
	{
		return;
	}

	if (ElectricZoneNiagaraSystem)
	{
		NiagaraComponent->SetAsset(ElectricZoneNiagaraSystem);
	}

	NiagaraComponent->SetVisibility(bVisible, true);
	NiagaraComponent->SetHiddenInGame(!bVisible, true);

	if (bVisible)
	{
		NiagaraComponent->Activate(true);
	}
	else
	{
		NiagaraComponent->DeactivateImmediate();
	}
}

void ALutalisElectricZone::ApplyDamageTick(int32 ArcIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	ResolveDamageCauser();

	if (!DamageCauser || !DamageCauser->GetAbilitySystemComponent())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (bDrawDebug)
	{
		DrawDebugZone();
	}

	for (TActorIterator<ADaeva> It(World); It; ++It)
	{
		ADaeva* Target = *It;

		if (!IsValid(Target))
		{
			continue;
		}

		if (Target == DamageCauser)
		{
			continue;
		}

		if (Target->IsDead())
		{
			continue;
		}

		if (!Target->GetAbilitySystemComponent())
		{
			continue;
		}

		if (!IsInsideElectricZone(Target, ArcIndex))
		{
			continue;
		}

		FHitResult HitResult;
		BuildDamageHitResult(Target, HitResult);

		AAOCharacter* TargetCharacter = Target;
		TargetCharacter->TakeDamageAO(TickDamageData, HitResult, DamageCauser);
	}
}

void ALutalisElectricZone::BuildDamageHitResult(ADaeva* Target, FHitResult& OutHitResult) const
{
	if (!Target)
	{
		return;
	}

	const FVector TargetLocation = Target->GetActorLocation();

	OutHitResult.HitObjectHandle = FActorInstanceHandle(Target);
	OutHitResult.Location = TargetLocation;
	OutHitResult.ImpactPoint = TargetLocation;
	OutHitResult.Component = Target->GetCapsuleComponent();
	OutHitResult.bBlockingHit = true;

	FVector ImpactNormal = TargetLocation - GetActorLocation();
	ImpactNormal.Z = 0.0f;
	ImpactNormal = ImpactNormal.GetSafeNormal();
	OutHitResult.ImpactNormal = ImpactNormal;
	OutHitResult.Normal = ImpactNormal;
}

bool ALutalisElectricZone::IsInsideElectricZone(AAOCharacter* Target, int32 ArcIndex) const
{
	if (!Target || !IsValidArcIndex(ArcIndex))
	{
		return false;
	}

	FVector TargetFeetLocation = Target->GetActorLocation();

	if (const UCapsuleComponent* Capsule = Target->GetCapsuleComponent())
	{
		TargetFeetLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();
	}

	const FVector ZoneLocation = GetActorLocation();

	if (FMath::Abs(TargetFeetLocation.Z - ZoneLocation.Z) > GroundTolerance)
	{
		return false;
	}

	FVector ToTarget = TargetFeetLocation - ZoneLocation;
	ToTarget.Z = 0.0f;

	if (ToTarget.SizeSquared() > FMath::Square(Radius))
	{
		return false;
	}

	const float TargetYaw = ToTarget.Rotation().Yaw;
	const UStaticMeshComponent* ArcMesh = ArcIndicatorMeshes.IsValidIndex(ArcIndex)
		? ArcIndicatorMeshes[ArcIndex]
		: nullptr;
	const float ArcYaw = ArcMesh ? ArcMesh->GetComponentRotation().Yaw : GetActorRotation().Yaw;
	const float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(ArcYaw, TargetYaw));
	const float ArcHalfAngle = SectorAngle * 0.5f;

	return DeltaYaw <= ArcHalfAngle;
}

bool ALutalisElectricZone::IsInsideAnyActiveElectricZone(AAOCharacter* Target) const
{
	for (int32 ArcIndex = 0; ArcIndex < GetArcCount(); ++ArcIndex)
	{
		if (!bArcActives.IsValidIndex(ArcIndex) || !bArcActives[ArcIndex])
		{
			continue;
		}

		if (IsInsideElectricZone(Target, ArcIndex))
		{
			return true;
		}
	}

	return false;
}

bool ALutalisElectricZone::IsValidArcIndex(int32 ArcIndex) const
{
	return ArcIndex >= 0 && ArcIndex < GetArcCount();
}

int32 ALutalisElectricZone::GetArcCount() const
{
	return ElectricZoneArcCount;
}

bool ALutalisElectricZone::IsAnyArcWarningInProgress() const
{
	for (bool bWarning : bArcWarnings)
	{
		if (bWarning)
		{
			return true;
		}
	}

	return false;
}

bool ALutalisElectricZone::IsAnyArcActive() const
{
	for (bool bActive : bArcActives)
	{
		if (bActive)
		{
			return true;
		}
	}

	return false;
}

void ALutalisElectricZone::ResolveDamageCauser()
{
	if (DamageCauser)
	{
		return;
	}

	DamageCauser = Cast<AAOCharacter>(GetOwner());

	if (!DamageCauser)
	{
		DamageCauser = Cast<AAOCharacter>(GetInstigator());
	}
}

void ALutalisElectricZone::FinishArcInternal(int32 ArcIndex)
{
	if (!IsValidArcIndex(ArcIndex))
	{
		return;
	}

	bArcWarnings[ArcIndex] = false;
	bArcWarningFilled[ArcIndex] = false;
	bArcActives[ArcIndex] = false;
	bArcFirstDamageApplied[ArcIndex] = false;

	ArcDamageElapsedTimes[ArcIndex] = 0.0f;
	ArcActiveElapsedTimes[ArcIndex] = 0.0f;

	SetArcWarningVisible(ArcIndex, false);
	SetArcActiveVisible(ArcIndex, false);
}

void ALutalisElectricZone::DrawDebugZone() const
{
#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Center = GetActorLocation();
	for (int32 ArcIndex = 0; ArcIndex < GetArcCount(); ++ArcIndex)
	{
		if (!bArcActives.IsValidIndex(ArcIndex) || !bArcActives[ArcIndex])
		{
			continue;
		}

		const UStaticMeshComponent* ArcMesh = ArcIndicatorMeshes.IsValidIndex(ArcIndex)
			? ArcIndicatorMeshes[ArcIndex]
			: nullptr;
		const float ArcYaw = ArcMesh ? ArcMesh->GetComponentRotation().Yaw : GetActorRotation().Yaw;
		const FVector Forward = FRotationMatrix(FRotator(0.0f, ArcYaw, 0.0f)).GetUnitAxis(EAxis::X);
		const float ArcHalfAngle = SectorAngle * 0.5f;
		const FVector LeftEdge = Forward.RotateAngleAxis(-ArcHalfAngle, FVector::UpVector) * Radius;
		const FVector RightEdge = Forward.RotateAngleAxis(ArcHalfAngle, FVector::UpVector) * Radius;

		DrawDebugLine(World, Center, Center + LeftEdge, FColor::Cyan, false, DebugDrawDuration, 0, 5.0f);
		DrawDebugLine(World, Center, Center + RightEdge, FColor::Cyan, false, DebugDrawDuration, 0, 5.0f);
	}

	DrawDebugSphere(World, Center, Radius, 32, FColor::Blue, false, DebugDrawDuration, 0, 1.0f);
#endif
}
