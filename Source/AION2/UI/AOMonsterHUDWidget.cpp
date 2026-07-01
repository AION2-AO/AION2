// Fill out your copyright notice in the Description page of Project Settings.

#include "AOMonsterHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Player/AOPlayerState.h"
#include "GAS/AttributeSet/AOAttributeSet.h"


void UAOMonsterHUDWidget::BindToASC(UAbilitySystemComponent* InASC)
{
    Super::BindToASC(InASC);

    if (!BoundASC)
    {
        return;
    }

    BindASCDelegates();
    BroadcastInitialAttributes();
}

void UAOMonsterHUDWidget::UnbindFromASC()
{
    UnbindASCDelegates();
    Super::UnbindFromASC();
}

void UAOMonsterHUDWidget::NativeDestruct()
{
    UnbindASCDelegates();
    Super::NativeDestruct();
}

void UAOMonsterHUDWidget::SetMonsterIndex(int32 InMonsterIndex)
{
    // TODO: 몬스터 이름 HardCoding. 나중에 바꾸기.
    static FString MonsterName = TEXT("");
    switch (InMonsterIndex)
    {
    case 0:
        MonsterName = TEXT("물의 정령");
        break;

    case 1:
        MonsterName = TEXT("거짓의 실리아토르");
        break;

    case 2:
        MonsterName = TEXT("고통의 루탈리스");
        break;

    case 3:
        MonsterName = TEXT("공허의 탈리스라");
        break;
    }

    TB_MonsterName->SetText(FText::FromString(MonsterName));
}

void UAOMonsterHUDWidget::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    const UAOAttributeSet* AttributeSet = BoundASC->GetSet<UAOAttributeSet>();
    if (!AttributeSet)
    {
        return;
    }

    UpdateHpBar(Data.NewValue, AttributeSet->GetMaxHealth());
}

void UAOMonsterHUDWidget::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
    const UAOAttributeSet* AttributeSet = BoundASC->GetSet<UAOAttributeSet>();
    if (!AttributeSet)
    {
        return;
    }

    UpdateStaminaBar(Data.NewValue, AttributeSet->GetMaxStamina());
}

void UAOMonsterHUDWidget::BindASCDelegates()
{
    HealthChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
        UAOAttributeSet::GetHealthAttribute()
    ).AddUObject(this, &UAOMonsterHUDWidget::HandleHealthChanged);

    // Stamina Bind
    StaminaChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
        UAOAttributeSet::GetStaminaAttribute()
    ).AddUObject(this, &UAOMonsterHUDWidget::HandleStaminaChanged);
}

void UAOMonsterHUDWidget::UnbindASCDelegates()
{
    if (!BoundASC)
    {
        return;
    }

    if (HealthChangedHandle.IsValid())
    {
        BoundASC->GetGameplayAttributeValueChangeDelegate(
            UAOAttributeSet::GetHealthAttribute()
        ).Remove(HealthChangedHandle);

        HealthChangedHandle.Reset();
    }

    if (StaminaChangedHandle.IsValid())
    {
        BoundASC->GetGameplayAttributeValueChangeDelegate(
            UAOAttributeSet::GetStaminaAttribute()
        ).Remove(StaminaChangedHandle);

        StaminaChangedHandle.Reset();
    }
}

void UAOMonsterHUDWidget::UpdateHpBar(float CurrentValue, float MaxValue)
{
    if (Pb_HpBar)
    {
        Pb_HpBar->SetPercent(MaxValue > 0.0f ? CurrentValue / MaxValue : 0.0f);
    }
}

void UAOMonsterHUDWidget::UpdateStaminaBar(float CurrentValue, float MaxValue)
{
    if (Pb_StaminaBar)
    {
        Pb_StaminaBar->SetPercent(MaxValue > 0.0f ? CurrentValue / MaxValue : 0.0f);
    }
}

void UAOMonsterHUDWidget::BroadcastInitialAttributes()
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
    UpdateStaminaBar(AttributeSet->GetStamina(), AttributeSet->GetMaxStamina());
}