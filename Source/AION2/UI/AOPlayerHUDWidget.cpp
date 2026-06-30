// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOPlayerHUDWidget.h"

#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Player/AOPlayerState.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AOMonsterHUDWidget.h"


void UAOPlayerHUDWidget::BindToASC(UAbilitySystemComponent* InASC)
{
    Super::BindToASC(InASC);

    if (!BoundASC)
    {
        return;
    }

    BindASCDelegates();
    BroadcastInitialAttributes();
}

void UAOPlayerHUDWidget::UnbindFromASC()
{
    UnbindASCDelegates();
    Super::UnbindFromASC();
}

void UAOPlayerHUDWidget::NativeDestruct()
{
    UnbindASCDelegates();
    Super::NativeDestruct();
}

void UAOPlayerHUDWidget::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    const UAOAttributeSet* AttributeSet = BoundASC->GetSet<UAOAttributeSet>();
    if (!AttributeSet)
    {
        return;
    }

    UpdateHpBar(Data.NewValue, AttributeSet->GetMaxHealth());
}

void UAOPlayerHUDWidget::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
    const UAOAttributeSet* AttributeSet = BoundASC->GetSet<UAOAttributeSet>();
    if (!AttributeSet)
    {
        return;
    }

    UpdateHpBar(AttributeSet->GetHealth(), Data.NewValue);
}

void UAOPlayerHUDWidget::HandleManaChanged(const FOnAttributeChangeData& Data)
{
    const UAOAttributeSet* AttributeSet = BoundASC->GetSet<UAOAttributeSet>();
    if (!AttributeSet)
    {
        return;
    }

    UpdateManaBar(Data.NewValue, AttributeSet->GetMaxMana());
}

void UAOPlayerHUDWidget::HandleMaxManaChanged(const FOnAttributeChangeData& Data)
{
    const UAOAttributeSet* AttributeSet = BoundASC->GetSet<UAOAttributeSet>();
    if (!AttributeSet)
    {
        return;
    }

    UpdateManaBar(AttributeSet->GetMana(), Data.NewValue);
}

void UAOPlayerHUDWidget::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
    const UAOAttributeSet* AttributeSet = BoundASC->GetSet<UAOAttributeSet>();
    if (!AttributeSet)
    {
        return;
    }

    UpdateStaminaBar(Data.NewValue, AttributeSet->GetMaxStamina());
}

void UAOPlayerHUDWidget::HandleMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
    const UAOAttributeSet* AttributeSet = BoundASC->GetSet<UAOAttributeSet>();
    if (!AttributeSet)
    {
        return;
    }

    UpdateStaminaBar(AttributeSet->GetStamina(), Data.NewValue);
}

void UAOPlayerHUDWidget::BindASCDelegates()
{
    // Health Bind
    HealthChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
        UAOAttributeSet::GetHealthAttribute()
    ).AddUObject(this, &UAOPlayerHUDWidget::HandleHealthChanged);

    MaxHealthChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
        UAOAttributeSet::GetMaxHealthAttribute()
    ).AddUObject(this, &UAOPlayerHUDWidget::HandleMaxHealthChanged);

    // Mana Bind
    ManaChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
        UAOAttributeSet::GetManaAttribute()
    ).AddUObject(this, &UAOPlayerHUDWidget::HandleManaChanged);

    MaxManaChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
        UAOAttributeSet::GetMaxManaAttribute()
    ).AddUObject(this, &UAOPlayerHUDWidget::HandleMaxManaChanged);

    // Stamina Bind
    StaminaChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
        UAOAttributeSet::GetStaminaAttribute()
    ).AddUObject(this, &UAOPlayerHUDWidget::HandleStaminaChanged);

    MaxStaminaChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
        UAOAttributeSet::GetMaxStaminaAttribute()
    ).AddUObject(this, &UAOPlayerHUDWidget::HandleMaxStaminaChanged);
}

void UAOPlayerHUDWidget::UnbindASCDelegates()
{
    if (!BoundASC)
    {
        return;
    }

    // Health ����
    if (HealthChangedHandle.IsValid())
    {
        BoundASC->GetGameplayAttributeValueChangeDelegate(
            UAOAttributeSet::GetHealthAttribute()
        ).Remove(HealthChangedHandle);

        HealthChangedHandle.Reset();
    }

    if (MaxHealthChangedHandle.IsValid())
    {
        BoundASC->GetGameplayAttributeValueChangeDelegate(
            UAOAttributeSet::GetMaxHealthAttribute()
        ).Remove(MaxHealthChangedHandle);

        MaxHealthChangedHandle.Reset();
    }

    // Mana ����
    if (ManaChangedHandle.IsValid())
    {
        BoundASC->GetGameplayAttributeValueChangeDelegate(
            UAOAttributeSet::GetManaAttribute()
        ).Remove(ManaChangedHandle);

        ManaChangedHandle.Reset();
    }

    if (MaxManaChangedHandle.IsValid())
    {
        BoundASC->GetGameplayAttributeValueChangeDelegate(
            UAOAttributeSet::GetMaxManaAttribute()
        ).Remove(MaxManaChangedHandle);

        MaxManaChangedHandle.Reset();
    }

    // Stamina ����
    if (StaminaChangedHandle.IsValid())
    {
        BoundASC->GetGameplayAttributeValueChangeDelegate(
            UAOAttributeSet::GetStaminaAttribute()
        ).Remove(StaminaChangedHandle);

        StaminaChangedHandle.Reset();
    }

    if (MaxStaminaChangedHandle.IsValid())
    {
        BoundASC->GetGameplayAttributeValueChangeDelegate(
            UAOAttributeSet::GetMaxStaminaAttribute()
        ).Remove(MaxStaminaChangedHandle);

        MaxStaminaChangedHandle.Reset();
    }
}

void UAOPlayerHUDWidget::BroadcastInitialAttributes()
{
    if (!BoundASC)
    {
        return;
    }

    const UAOAttributeSet* AttributeSet = BoundASC->GetSet<UAOAttributeSet>();
    if (!AttributeSet)
    {
        return;
    }

    UpdateHpBar(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
    UpdateManaBar(AttributeSet->GetMana(), AttributeSet->GetMaxMana());
    UpdateStaminaBar(AttributeSet->GetStamina(), AttributeSet->GetMaxStamina());
}

void UAOPlayerHUDWidget::UpdateHpBar(float CurrentValue, float MaxValue)
{
    if (Pb_HpBar)
    {
        Pb_HpBar->SetPercent(MaxValue > 0.0f ? CurrentValue / MaxValue : 0.0f);
    }

    if (TB_HpText)
    {
        TB_HpText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), static_cast<int>(CurrentValue), static_cast<int>(MaxValue))));
    }
}

void UAOPlayerHUDWidget::UpdateManaBar(float CurrentValue, float MaxValue)
{
    if (Pb_MpBar)
    {
        Pb_MpBar->SetPercent(MaxValue > 0.0f ? CurrentValue / MaxValue : 0.0f);
    }


    if (TB_MpText)
    {
        TB_MpText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), static_cast<int>(CurrentValue), static_cast<int>(MaxValue))));
    }
}

void UAOPlayerHUDWidget::UpdateStaminaBar(float CurrentValue, float MaxValue)
{
    if (Pb_StaminaBar)
    {
        Pb_StaminaBar->SetPercent(MaxValue > 0.0f ? CurrentValue / MaxValue : 0.0f);
        if (Pb_StaminaBar->GetPercent() == 1.0f)
        {
            Pb_StaminaBar->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            Pb_StaminaBar->SetVisibility(ESlateVisibility::Visible);
        }
    }
}
