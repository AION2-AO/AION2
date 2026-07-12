// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/Boss/Lutalis/LutalisScytheZone.h"

#include "Animation/AnimSequenceBase.h"
#include "Character/Daeva/Daeva.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ALutalisScytheZone::ALutalisScytheZone()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 30.f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	WarningRectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WarningRectMesh"));
	WarningRectMesh->SetupAttachment(SceneRoot);

	ScytheMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScytheMesh"));
	ScytheMesh->SetupAttachment(SceneRoot);
	ScytheMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ScytheSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ScytheSkeletalMesh"));
	ScytheSkeletalMesh->SetupAttachment(SceneRoot);
	ScytheSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScytheSkeletalMesh->VisibilityBasedAnimTickOption =
		EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetupAttachment(SceneRoot);
	DamageBox->SetGenerateOverlapEvents(true);
	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageBox->SetCollisionObjectType(ECC_WorldDynamic);
	DamageBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	WarningRectMesh->SetHiddenInGame(true);
	ScytheMesh->SetHiddenInGame(true);
	ScytheSkeletalMesh->SetHiddenInGame(true);
}

void ALutalisScytheZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALutalisScytheZone, Length);
	DOREPLIFETIME(ALutalisScytheZone, Width);
	DOREPLIFETIME(ALutalisScytheZone, ReplicatedWarningDuration);
	DOREPLIFETIME(ALutalisScytheZone, VisualState);
}

// Called when the game starts or when spawned
void ALutalisScytheZone::BeginPlay()
{
	Super::BeginPlay();

	OnRep_VisualState();
}

void ALutalisScytheZone::InitZone(AAOCharacter* InDamageCauser, const FAttackData& InDamageData, float InLength, float InWidth)
{
	DamageCauser = InDamageCauser;
	DamageData = InDamageData;
	Length = InLength;
	Width = InWidth;

	ApplyZoneDimensions();
	ForceNetUpdate();
}

void ALutalisScytheZone::OnRep_ZoneDimensions()
{
	ApplyZoneDimensions();
}

void ALutalisScytheZone::ApplyZoneDimensions()
{
	WarningRectMesh->SetWorldScale3D(FVector(Length / 100.f, Width / 100.f, 1.f));
	DamageBox->SetBoxExtent(FVector(Length * 0.5f, Width * 0.2f, DamageHeight * 0.5f));
	DamageBox->SetRelativeLocation(FVector(0.f, 0.f, DamageHeight * 0.5f));
	ScytheMesh->SetRelativeLocation(FVector(0.f, 0.f, ScytheHeight));
	ScytheSkeletalMesh->SetRelativeLocation(FVector(0.f, 0.f, ScytheHeight));
	AlignScytheToWarningRotation();
}

void ALutalisScytheZone::StartWarning(float InWarningDuration)
{
	if (!HasAuthority())
	{
		return;
	}

	ReplicatedWarningDuration = FMath::Max(InWarningDuration, 0.1f);
	VisualState = ELutalisScytheZoneVisualState::Warning;
	StartWarningInternal(ReplicatedWarningDuration);
	ForceNetUpdate();
}

void ALutalisScytheZone::ActivateSweep()
{
	if (!HasAuthority())
	{
		return;
	}

	VisualState = ELutalisScytheZoneVisualState::Scythe;
	ActivateSweepInternal();
	ForceNetUpdate();
}

void ALutalisScytheZone::OnRep_VisualState()
{
	switch (VisualState)
	{
	case ELutalisScytheZoneVisualState::Warning:
		StartWarningInternal(ReplicatedWarningDuration);
		break;
	case ELutalisScytheZoneVisualState::Scythe:
		ActivateSweepInternal();
		break;
	default:
		WarningRectMesh->SetHiddenInGame(true);
		ScytheMesh->SetHiddenInGame(true);
		ScytheSkeletalMesh->SetHiddenInGame(true);
		DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		bWarning = false;
		bDamageWindowActive = false;
		RefreshTickEnabled();
		break;
	}
}

void ALutalisScytheZone::StartWarningInternal(float InWarningDuration)
{
	WarningDuration = FMath::Max(InWarningDuration, 0.1f);
	WarningElapsed = 0.f;
	bWarning = true;
	bDamageWindowActive = false;
	ApplyZoneDimensions();

	WarningMID = WarningRectMesh->CreateDynamicMaterialInstance(0);
	if (WarningMID)
	{
		WarningMID->SetScalarParameterValue(FillParameterName, 0.f);
	}

	WarningRectMesh->SetHiddenInGame(false);
	ScytheMesh->SetHiddenInGame(true);
	ScytheSkeletalMesh->SetHiddenInGame(true);
	RefreshTickEnabled();
}

void ALutalisScytheZone::ActivateSweepInternal()
{
	bWarning = false;
	bDamageWindowActive = false;
	DamagedActors.Reset();

	WarningRectMesh->SetHiddenInGame(true);
	ApplyZoneDimensions();
	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (ScytheAnimation)
	{
		ScytheSkeletalMesh->PlayAnimation(ScytheAnimation, false);
		if (HasAuthority())
		{
			SetLifeSpan(FMath::Max(ScytheAnimation->GetPlayLength() + ExtraLifeAfterAnimation, 0.1f));
		}
	}
	else
	{
		if (HasAuthority())
		{
			SetLifeSpan(FMath::Max(FallbackLifeTime, 0.1f));
		}
	}

	ScytheMesh->SetHiddenInGame(ScytheAnimation != nullptr);
	ScytheSkeletalMesh->SetHiddenInGame(ScytheAnimation == nullptr);
	RefreshTickEnabled();
}

void ALutalisScytheZone::BeginDamageWindow()
{
	if (!HasAuthority())
	{
		return;
	}

	bDamageWindowActive = true;
	DamageBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageBox->UpdateOverlaps();
	ApplyDamageToOverlaps();
	RefreshTickEnabled();
}

void ALutalisScytheZone::EndDamageWindow()
{
	if (!HasAuthority())
	{
		return;
	}

	bDamageWindowActive = false;
	DamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RefreshTickEnabled();
}

void ALutalisScytheZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bWarning)
	{
		UpdateWarning();
	}

	if (bDamageWindowActive)
	{
		ApplyDamageToOverlaps();
	}

	//DrawDamageBoxDebug();
}

void ALutalisScytheZone::UpdateWarning()
{
	WarningElapsed += GetWorld()->GetDeltaSeconds();

	const float Alpha = FMath::Clamp(WarningElapsed / WarningDuration, 0.f, 1.f);
	if (WarningMID)
	{
		WarningMID->SetScalarParameterValue(FillParameterName, Alpha);
	}
}

void ALutalisScytheZone::ApplyDamageToOverlaps()
{
	if (!HasAuthority() || !DamageCauser || !bDamageWindowActive)
	{
		return;
	}

	TArray<AActor*> Overlaps;
	DamageBox->GetOverlappingActors(Overlaps, ADaeva::StaticClass());

	for (AActor* Actor : Overlaps)
	{
		ADaeva* Target = Cast<ADaeva>(Actor);
		if (!Target || Target->IsDead() || DamagedActors.Contains(Target))
		{
			continue;
		}

		FHitResult Hit;
		Hit.HitObjectHandle = FActorInstanceHandle(Target);
		Hit.Location = Target->GetActorLocation();
		Hit.ImpactPoint = Hit.Location;
		Hit.bBlockingHit = true;

		AAOCharacter* TargetCharacter = Target;
		TargetCharacter->TakeDamageAO(DamageData, Hit, DamageCauser);
		DamagedActors.Add(Target);
	}
}

void ALutalisScytheZone::AlignScytheToWarningRotation()
{
	const FRotator WarningRotation = WarningRectMesh->GetComponentRotation();
	const FRotator ScytheRotation =
		(WarningRotation.Quaternion() * ScytheRotationOffset.Quaternion()).Rotator();

	DamageBox->SetWorldRotation(WarningRotation);
	ScytheMesh->SetWorldRotation(ScytheRotation);
	ScytheSkeletalMesh->SetWorldRotation(ScytheRotation);
}

void ALutalisScytheZone::DrawDamageBoxDebug() const
{
#if !UE_BUILD_SHIPPING
	if (!bDrawDamageBoxDebug || !DamageBox || !GetWorld())
	{
		return;
	}

	const FColor DebugColor = bDamageWindowActive ? FColor::Red : FColor::Yellow;
	DrawDebugBox(
		GetWorld(),
		DamageBox->GetComponentLocation(),
		DamageBox->GetScaledBoxExtent(),
		DamageBox->GetComponentQuat(),
		DebugColor,
		false,
		DamageBoxDebugDrawDuration,
		0,
		DamageBoxDebugThickness
	);
#endif
}

void ALutalisScytheZone::RefreshTickEnabled()
{
	const bool bNeedsDebugTick =
		bDrawDamageBoxDebug && VisualState != ELutalisScytheZoneVisualState::Hidden;

	SetActorTickEnabled(bWarning || bDamageWindowActive || bNeedsDebugTick);
}
