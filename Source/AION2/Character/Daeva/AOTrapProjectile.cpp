#include "Character/Daeva/AOTrapProjectile.h"
#include "AOTrapProjectile.h"
#include "Character/AOCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "TimerManager.h"

AAOTrapProjectile::AAOTrapProjectile()
{
	bReplicates = true;
}

void AAOTrapProjectile::BeginPlay()
{
	// 장판 생성 후 부모의 직격 처리 / Destroy를 실행하기 위해 먼저 등록
	if (HasAuthority())
	{
		Collision->OnComponentBeginOverlap.AddDynamic(this, &AAOTrapProjectile::SpawnTrapOnOverlap);
	}

	Super::BeginPlay();
}

void AAOTrapProjectile::SpawnTrapOnOverlap(UPrimitiveComponent* OverlappedComp,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	if (!HasAuthority() || bTrapSpawned || !TrapZoneClass)
	{
		return;
	}

	AAOCharacter* HitCharacter = Cast<AAOCharacter>(OtherActor);

	if (!HitCharacter)
	{
		return;
	}

	if (HitCharacter != Target)
	{
		return;
	}

	AAOCharacter* SourceCharacter = DamageCauser.Get();

	if (!SourceCharacter)
	{
		return;
	}

	if (HitCharacter->IsDead())
	{
		return;
	}

	if (!SourceCharacter->IsEnemy(HitCharacter))
	{
		return;
	}

	bTrapSpawned = true;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = SourceCharacter;
	SpawnParams.Instigator = SourceCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector TrapLocation =
		GetTrapSpawnLocation(HitCharacter, SweepResult);

	AAOTrapZone* SpawnedTrap = GetWorld()->SpawnActor<AAOTrapZone>(TrapZoneClass,TrapLocation,FRotator::ZeroRotator,SpawnParams);
}

FVector AAOTrapProjectile::GetTrapSpawnLocation(AAOCharacter* HitCharacter, const FHitResult& SweepResult) const
{
	return GetActorLocation();
}
