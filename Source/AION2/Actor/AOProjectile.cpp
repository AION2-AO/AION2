#include "Actor/AOProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Character/AOCharacter.h"

AAOProjectile::AAOProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(true);

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);

    Collision->InitSphereRadius(20.f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
}

void AAOProjectile::BeginPlay()
{
    Super::BeginPlay();

    SetLifeSpan(LifeSeconds);

    ProjectileMovement->InitialSpeed = ProjectileSpeed;
    ProjectileMovement->MaxSpeed = ProjectileSpeed;

    if (HasAuthority())
    {
        Collision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnProjectileOverlap);
    }
}

void AAOProjectile::InitProjectile(const FAttackData& InAttackData, AAOCharacter* InDamageCauser, AAOCharacter* InTarget, const FVector& InDirection)
{
    AttackData = InAttackData;
    DamageCauser = InDamageCauser;
    Target = InTarget;

    const FVector Direction = InDirection.GetSafeNormal();

    ProjectileMovement->InitialSpeed = ProjectileSpeed;
    ProjectileMovement->MaxSpeed = ProjectileSpeed;
    ProjectileMovement->Velocity = Direction * ProjectileSpeed;

    if (bHoming && IsValid(Target))
    {
        ProjectileMovement->bIsHomingProjectile = true;
        ProjectileMovement->HomingTargetComponent = Target->GetRootComponent();
        ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
    }
    else
    {
        ProjectileMovement->bIsHomingProjectile = false;
    }
}

void AAOProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }

    AAOCharacter* HitCharacter = Cast<AAOCharacter>(OtherActor);
    if (!HitCharacter || HitCharacter != Target)
    {
        return;
    }

    if (HitCharacter->IsDead())
    {
        return;
    }

    if (!DamageCauser->IsEnemy(HitCharacter))
    {
        return;
    }

    bool bDidCameraShake = false;
    DamageCauser->OnAttackSucceeded(AttackData, Target, SweepResult, bDidCameraShake);

    Destroy();
}