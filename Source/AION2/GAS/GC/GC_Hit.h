#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GC_Hit.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS(Blueprintable)
class AION2_API UGC_Hit : public UGameplayCueNotify_Static
{
    GENERATED_BODY()

public:
    virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Hit Effect")
    TObjectPtr<UNiagaraSystem> HitEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Hit Effect")
    TObjectPtr<USoundBase> HitSound;

    UPROPERTY(EditDefaultsOnly, Category = "Hit Effect")
    float EffectScale = 1.f;
};
