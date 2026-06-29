#include "GAS/GC/GC_GhostTrail.h"
#include "Character/AOCharacter.h"
#include "Actor/Effect/GhostTrailSpawner.h"
#include "Actor/Effect/GhostTrailEffect.h"

bool UGC_GhostTrail::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget)
	{
		return false;
	}

	UWorld* World = MyTarget->GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return false;
	}

	if (!SpawnerClass || !GhostTrailClass)
	{
		return false;
	}

	AGhostTrailSpawner* Spawner = World->SpawnActor<AGhostTrailSpawner>(SpawnerClass, MyTarget->GetActorTransform());
	if (!Spawner)
	{
		return false;
	}

	Spawner->Initialize(MyTarget, GhostTrailClass, GhostMaterial, GhostLifeTime, SpawnInterval, EffectDuration);

	return true;
}
