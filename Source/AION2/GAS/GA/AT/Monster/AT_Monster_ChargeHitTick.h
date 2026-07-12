// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_Monster_ChargeHitTick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChargeHitTickDelegate);
/**
 * 
 */
UCLASS()
class AION2_API UAT_Monster_ChargeHitTick : public UAbilityTask
{
	GENERATED_BODY()
	
public:
    UAT_Monster_ChargeHitTick();

    // 매 틱 브로드캐스트
    UPROPERTY(BlueprintAssignable)
    FChargeHitTickDelegate OnTick;

    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
        meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
    static UAT_Monster_ChargeHitTick* CreateChargeHitTick(UGameplayAbility* OwningAbility);

    virtual void Activate() override;
    virtual void TickTask(float DeltaTime) override;

protected:
    virtual void OnDestroy(bool bInOwnerFinished) override;

};
