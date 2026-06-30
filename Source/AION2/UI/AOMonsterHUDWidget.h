// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "AbilitySystemComponent.h"

#include "AOMonsterHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class AION2_API UAOMonsterHUDWidget : public UAOUserWidgetBase
{
	GENERATED_BODY()
	
	
public:
    virtual void BindToASC(UAbilitySystemComponent* InASC) override;
    virtual void UnbindFromASC() override;

protected:
    // UI 소멸자.
    virtual void NativeDestruct() override;

public:
    // 상위에서 Bind호출할 때 이것도 호출해줘야 함.
    void SetMonsterIndex(int32 InMonsterIndex);

protected:
    void HandleHealthChanged(const FOnAttributeChangeData& Data);
    void HandleStaminaChanged(const FOnAttributeChangeData& Data);

private:
    void BindASCDelegates();
    void UnbindASCDelegates();

    void BroadcastInitialAttributes();

private:
    // 내부 UI 업데이트
    void UpdateHpBar(float CurrentValue, float MaxValue);
    void UpdateStaminaBar(float CurrentValue, float MaxValue);

private:
    ///
    // 중복 Binding/해제 문제에 대한 대안.
    // 기존 Delegate을 저장하고, 재Binding마다 기존 Delegate 제거 후 bind.
    ///
    FDelegateHandle HealthChangedHandle;

    // 여기서 Stamina는 그로기 게이지.
    FDelegateHandle StaminaChangedHandle;

protected:
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UProgressBar> Pb_HpBar;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UProgressBar> Pb_StaminaBar;


    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UTextBlock> TB_MonsterName;
	
private:
    int32 MonsterIndex = 0;
};
