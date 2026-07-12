#include "Actor/SiliatorShuriken.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AION2/Character/AOCharacter.h"
#include "Net/UnrealNetwork.h"

#include "AbilitySystemComponent.h"

ASiliatorShuriken::ASiliatorShuriken()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	SetRootComponent(BoxCollision);

	BoxCollision->SetBoxExtent(FVector(120.f, 120.f, 20.f));
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxCollision->SetCollisionObjectType(ECC_WorldDynamic);
	BoxCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoxCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ASiliatorShuriken::OnBoxBeginOverlap);
	BoxCollision->OnComponentEndOverlap.AddDynamic(this, &ASiliatorShuriken::OnBoxEndOverlap);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(BoxCollision);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AOEPlaneComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AOEPlaneComp"));
	AOEPlaneComp->SetupAttachment(RootComponent);
	AOEPlaneComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InitialLifeSpan = LifeTime;
}

void ASiliatorShuriken::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle Timer;
	GetWorldTimerManager().SetTimer(Timer, this, &ASiliatorShuriken::StartMove, 3.5f, false);
}

void ASiliatorShuriken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASiliatorShuriken, TargetCharacter);
}

void ASiliatorShuriken::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateShuriken(DeltaTime);

	UpdateAOEPlane();

	if (!HasAuthority())
	{
		return;
	}

	MoveToTarget(DeltaTime);

	DamageElapsedTime += DeltaTime;
	if (DamageElapsedTime >= DamageInterval)
	{
		DamageElapsedTime = 0.f;
		ProcessTickDamage();
	}
}

void ASiliatorShuriken::Destroyed()
{
	Super::Destroyed();

	if (!HasAuthority())
	{
		return;
	}

	if (AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner()))
	{
		if (UAbilitySystemComponent* OwnerASC = OwnerCharacter->GetAbilitySystemComponent())
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = GetActorLocation();
			CueParams.Instigator = this;
			CueParams.EffectCauser = this;
			OwnerASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Hit.Monster.Siliator.Explosion")), CueParams);
		}
	}
}

void ASiliatorShuriken::SetTarget(AAOCharacter* InTarget, const FAttackData& InAttackData)
{
	TargetCharacter = InTarget;
	AttackData = InAttackData;

	OnRep_TargetCharacter();
}

void ASiliatorShuriken::StartMove()
{
	AOEPlaneComp->SetVisibility(false);
	bHasMoveStarted = true;
}

void ASiliatorShuriken::MoveToTarget(float DeltaTime)
{
	if (!bHasMoveStarted || !TargetCharacter)
	{
		return;
	}

	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = TargetCharacter->GetActorLocation();
	TargetLocation.Z = CurrentLocation.Z;

	const FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);

	SetActorLocation(NewLocation);
}

void ASiliatorShuriken::RotateShuriken(float DeltaTime)
{
	FRotator NewRotation = MeshComp->GetRelativeRotation();
	NewRotation.Yaw += RotationSpeed * DeltaTime;
	MeshComp->SetRelativeRotation(NewRotation);
}

void ASiliatorShuriken::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(OtherActor) || OtherActor == this)
	{
		return;
	}

	if (ASiliatorShuriken* OtherShuriken = Cast<ASiliatorShuriken>(OtherActor))
	{
		OtherShuriken->Destroy();
		Destroy();

		return;
	}

	AAOCharacter* HitCharacter = Cast<AAOCharacter>(OtherActor);
	if (!IsValid(HitCharacter))
	{
		return;
	}

	AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	if (!OwnerCharacter->IsEnemy(HitCharacter))
	{
		return;
	}

	OverlappingCharacters.Add(HitCharacter);
}

void ASiliatorShuriken::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	AAOCharacter* HitCharacter = Cast<AAOCharacter>(OtherActor);
	if (!IsValid(HitCharacter))
	{
		return;
	}

	AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	if (!OwnerCharacter->IsEnemy(HitCharacter))
	{
		return;
	}

	OverlappingCharacters.Remove(HitCharacter);
}

void ASiliatorShuriken::ProcessTickDamage()
{
	for (auto It = OverlappingCharacters.CreateIterator(); It; ++It)
	{
		AAOCharacter* HitCharacter = *It;

		if (!IsValid(HitCharacter))
		{
			It.RemoveCurrent();
			continue;
		}

		if (HitCharacter->IsDead())
		{
			It.RemoveCurrent();
			continue;
		}

		ApplyTickDamage(HitCharacter);
	}
}

void ASiliatorShuriken::ApplyTickDamage(AAOCharacter* HitCharacter)
{
	AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	FHitResult HitResult;
	bool bCameraShake = false;
	OwnerCharacter->OnAttackSucceeded(AttackData, HitCharacter, HitResult, bCameraShake);
}

void ASiliatorShuriken::UpdateAOEPlane()
{
	if (!AOEPlaneComp || !TargetCharacter || bHasMoveStarted)
	{
		return;
	}

	const FVector StartLocation = GetActorLocation();
	FVector TargetLocation = TargetCharacter->GetActorLocation();
	TargetLocation.Z = StartLocation.Z;

	const FVector ToTarget = TargetLocation - StartLocation;
	const float Distance = ToTarget.Size();

	if (Distance <= KINDA_SMALL_NUMBER)
	{
		AOEPlaneComp->SetVisibility(false);
		return;
	}

	AOEPlaneComp->SetVisibility(true);

	const FVector Direction = ToTarget / Distance;

	// Plane 중심은 표창과 타깃의 중간
	const FVector CenterLocation = StartLocation + Direction * (Distance * 0.5f) + FVector(0.f, 0.f, AOEPlaneZOffset);

	// X축이 타깃 방향을 바라보게 회전
	const FRotator PlaneRotation = Direction.Rotation();

	// UE 기본 Plane은 XY 평면
	// 길이: X축, 폭: Y축 기준
	const FVector PlaneScale(Distance / PlaneBaseSizeX, AOEPlaneWidth / PlaneBaseSizeY, 1.f);
	AOEPlaneComp->SetWorldLocation(CenterLocation);
	AOEPlaneComp->SetWorldRotation(PlaneRotation);
	AOEPlaneComp->SetWorldScale3D(PlaneScale);
}

void ASiliatorShuriken::OnRep_TargetCharacter()
{
	UpdateAOEPlane();
}
