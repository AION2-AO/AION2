// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "AbilitySystemComponent.h"
#include "Item/AOItemDataBase.h"
#include "Player/AOPlayerState.h"
#include "AOPlayerHUDWidget.generated.h"

/**
 * 
 */

class UProgressBar;
class UTextBlock;
class UAOQuickSkillHUD;
class UAOClassSwitcherWidget;


UCLASS()
class AION2_API UAOPlayerHUDWidget : public UAOUserWidgetBase
{
    GENERATED_BODY()

public:
    virtual void BindToASC(UAbilitySystemComponent* InASC) override;
    virtual void UnbindFromASC() override;
    
private:
    void BindASCDelegates();
    void UnbindASCDelegates();


protected:
    // UMG 처음 만들어질 때만 호출됨
    //virtual void NativeOnInitialized() override;

    // UI 소멸자.
    virtual void NativeDestruct() override;

public:
    // => SkillHUD.
    void PlaySkillPressedFeedback(int32 InputId);


    void BroadcastInitialAttributes();
public:
    // === Class Widget by PlayerInfo ===
    void ChangeClassIcon(EDaevaClassType InClassType);

    // === Set Player Nickname ===
    void SetPlayerName(const FText PlayerName);

protected:
    void HandleHealthChanged(const FOnAttributeChangeData& Data);
    void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);

    void HandleManaChanged(const FOnAttributeChangeData& Data);
    void HandleMaxManaChanged(const FOnAttributeChangeData& Data);

    void HandleStaminaChanged(const FOnAttributeChangeData& Data);
    void HandleMaxStaminaChanged(const FOnAttributeChangeData& Data);


private:
    // 내부 UI 업데이트
    void UpdateHpBar(float CurrentValue, float MaxValue);
    void UpdateManaBar(float CurrentValue, float MaxValue);
    void UpdateStaminaBar(float CurrentValue, float MaxValue);

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void UpdateItemQuickSlot(int32 SlotIndex, const FAOSlotData& SlotData, const FItemData& TemplateData);

private:
    ///
    // 중복 Binding/해제 문제에 대한 대안.
    // 기존 Delegate을 저장하고, 재Binding마다 기존 Delegate 제거 후 bind.
    ///
    FDelegateHandle HealthChangedHandle;
    FDelegateHandle MaxHealthChangedHandle;

    FDelegateHandle ManaChangedHandle;
    FDelegateHandle MaxManaChangedHandle;

    FDelegateHandle StaminaChangedHandle;
    FDelegateHandle MaxStaminaChangedHandle;
    
protected:
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
    TObjectPtr<UAOClassSwitcherWidget> PlayerClassSwitcher;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
    TObjectPtr<UTextBlock> TB_PlayerName;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
    TObjectPtr<UProgressBar> Pb_HpBar;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
    TObjectPtr<UTextBlock> TB_HpText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
    TObjectPtr<UProgressBar> Pb_MpBar;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
    TObjectPtr<UTextBlock> TB_MpText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
    TObjectPtr<UProgressBar> Pb_StaminaBar;

    // For PlayerHUD on the MainPanel (Optional for the player head-up status bar, etc.).
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional,AllowPrivateAccess = "true"))
    TObjectPtr<UAOQuickSkillHUD> QuickSkillHUD;

};
