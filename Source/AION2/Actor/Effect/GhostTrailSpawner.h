#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GhostTrailSpawner.generated.h"

class AGhostTrailEffect;
class UMaterialInterface;

UCLASS()
class AION2_API AGhostTrailSpawner : public AActor
{
	GENERATED_BODY()

public:
	AGhostTrailSpawner();

public:
	void Initialize(AActor* InTarget, TSubclassOf<AGhostTrailEffect> InGhostTrailClass, UMaterialInterface* InGhostMaterial, float InGhostLifeTime, float InSpawnInterval, float InEffectDuration);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	void SpawnGhostTrail();
	void StopSpawner();

private:
	UPROPERTY()
	TObjectPtr<AActor> Target;

	UPROPERTY()
	TSubclassOf<AGhostTrailEffect> GhostTrailClass;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> GhostMaterial;

	float GhostLifeTime = 0.4f;
	float SpawnInterval = 0.05f;
	float EffectDuration = 0.4f;

	FTimerHandle SpawnTimerHandle;
	FTimerHandle EndTimerHandle;
};
