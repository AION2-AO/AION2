// GA_Monster_Chase.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AITypes.h"
#include "GA_Monster_Chase.generated.h"

class AAIMonsterControllerBase;
class UAT_RotateToTarget;

UCLASS()
class AION2_API UGA_Monster_Chase : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_Monster_Chase();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
    void OnChaseTimeout();
    void FinishChase(bool bSuccess);

private:
    UPROPERTY()
    TObjectPtr<UAT_RotateToTarget> RotateTask = nullptr;

    UPROPERTY()
    TObjectPtr<AAIMonsterControllerBase> CachedAIController = nullptr;

    UPROPERTY()
    TObjectPtr<AActor> CachedTarget = nullptr;

    FTimerHandle CheckTimerHandle;
    FTimerHandle TimeoutTimerHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Chase")
    float AcceptableRadius = 200.f;

    UPROPERTY(EditDefaultsOnly, Category = "Chase")
    float ChaseTimeout = 2.f;

    UPROPERTY(EditDefaultsOnly, Category = "Chase")
    float RotateAvailableRange = 10000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Chase")
    float RotateInterpSpeed = 15.f;
};
