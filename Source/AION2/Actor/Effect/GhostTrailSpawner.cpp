#include "Actor/Effect/GhostTrailSpawner.h"
#include "Actor/Effect/GhostTrailEffect.h"
#include "Character/AOCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

AGhostTrailSpawner::AGhostTrailSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(false);
}

void AGhostTrailSpawner::Initialize(AActor* InTarget, TSubclassOf<AGhostTrailEffect> InGhostTrailClass, UMaterialInterface* InGhostMaterial, float InGhostLifeTime, float InSpawnInterval, float InEffectDuration)
{
	Target = InTarget;
	GhostTrailClass = InGhostTrailClass;
	GhostMaterial = InGhostMaterial;

	GhostLifeTime = InGhostLifeTime;
	SpawnInterval = InSpawnInterval;
	EffectDuration = InEffectDuration;

	if (!Target || !GhostTrailClass)
	{
		Destroy();
		return;
	}

	SpawnGhostTrail();

	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&ThisClass::SpawnGhostTrail,
		SpawnInterval,
		true
	);

	GetWorldTimerManager().SetTimer(
		EndTimerHandle,
		this,
		&ThisClass::StopSpawner,
		EffectDuration,
		false
	);
}

void AGhostTrailSpawner::SpawnGhostTrail()
{
	AAOCharacter* Character = Cast<AAOCharacter>(Target);
	if (!Character || !GhostTrailClass)
	{
		return;
	}

	UWorld* World = Character->GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return;
	}

	AGhostTrailEffect* Ghost = World->SpawnActor<AGhostTrailEffect>(GhostTrailClass, Mesh->GetComponentTransform());
	if (!Ghost)
	{
		return;
	}

	Ghost->Initialize(Character->GetMesh(), Character->GetAllMeshes(), GhostMaterial, GhostLifeTime);
}

void AGhostTrailSpawner::StopSpawner()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(EndTimerHandle);

	Destroy();
}

void AGhostTrailSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(EndTimerHandle);

	Super::EndPlay(EndPlayReason);
}
