#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GhostTrailEffect.generated.h"

UCLASS()
class AION2_API AGhostTrailEffect : public AActor
{
	GENERATED_BODY()

public:
	AGhostTrailEffect();

public:
	void Initialize(USkeletalMeshComponent* RootMesh, const TArray<USkeletalMeshComponent*>& SourceMeshes, UMaterialInterface* InGhostMaterial, float InLifeTime = 0.4f);

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	TArray<TObjectPtr<class UPoseableMeshComponent>> GhostMeshes;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	float LifeTime = 0.4f;
	float ElapsedTime = 0.f;
};
