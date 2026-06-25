#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_RotateToTarget.generated.h"

UCLASS()
class AION2_API UAT_RotateToTarget : public UAbilityTask
{
    GENERATED_BODY()

public:
    UAT_RotateToTarget();

    UFUNCTION(BlueprintCallable, Category = "Ability|Task", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility"))
    static UAT_RotateToTarget* RotateToTarget(UGameplayAbility* OwningAbility, float InAvailableRange = 0.f, float InInterpSpeed = 15.f);

    virtual void Activate() override;
    virtual void TickTask(float DeltaTime) override;
    virtual void OnDestroy(bool bInOwnerFinished) override;

private:
    float AvailableRange = 0.f;
    float InterpSpeed = 15.f;
};
