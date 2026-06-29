#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GC_GhostTrail.generated.h"

UCLASS()
class AION2_API UGC_GhostTrail : public UGameplayCueNotify_Static
{
    GENERATED_BODY()

public:
    virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Ghost")
    TSubclassOf<class AGhostTrailSpawner> SpawnerClass;

    UPROPERTY(EditDefaultsOnly, Category = "Ghost")
    TSubclassOf<class AGhostTrailEffect> GhostTrailClass;

    UPROPERTY(EditDefaultsOnly, Category = "Ghost")
    TObjectPtr<class UMaterialInterface> GhostMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "Ghost")
    float GhostLifeTime = 0.4f;

    UPROPERTY(EditDefaultsOnly, Category = "Ghost")
    float SpawnInterval = 0.05f;

    UPROPERTY(EditDefaultsOnly, Category = "Ghost")
    float EffectDuration = 0.4f;
};
