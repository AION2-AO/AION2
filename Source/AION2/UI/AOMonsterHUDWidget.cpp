// Fill out your copyright notice in the Description page of Project Settings.

#include "AOMonsterHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Player/AOPlayerState.h"
#include "GAS/AttributeSet/AOAttributeSet.h"


void UAOMonsterHUDWidget::BindToASC(UAbilitySystemComponent* InASC)
{
    if (!InASC || BoundASC == InASC)
    {
        return;
    }

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
    // TODO(suyeon): 몬스터 이름 HardCoding. 나중에 바꾸기.
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

    if (TB_MonsterName)
    {
        TB_MonsterName->SetText(FText::FromString(MonsterName));
    }
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

void UAOMonsterHUDWidget::HandleGroggyChanged(const FOnAttributeChangeData& Data)
{
    const UAOAttributeSet* AttributeSet = BoundASC->GetSet<UAOAttributeSet>();
    if (!AttributeSet)
    {
        return;
    }

    UpdateGroggyBar(Data.NewValue, AttributeSet->GetMaxGroggy());
}

void UAOMonsterHUDWidget::BindASCDelegates()
{
    HealthChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
        UAOAttributeSet::GetHealthAttribute()
    ).AddUObject(this, &UAOMonsterHUDWidget::HandleHealthChanged);

    // Groggy Bind
    GroggyChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(
        UAOAttributeSet::GetGroggyAttribute()
    ).AddUObject(this, &UAOMonsterHUDWidget::HandleGroggyChanged);
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

    if (GroggyChangedHandle.IsValid())
    {
        BoundASC->GetGameplayAttributeValueChangeDelegate(
            UAOAttributeSet::GetGroggyAttribute()
        ).Remove(GroggyChangedHandle);

        GroggyChangedHandle.Reset();
    }
}

void UAOMonsterHUDWidget::UpdateHpBar(float CurrentValue, float MaxValue)
{
    if (Pb_HpBar)
    {
        Pb_HpBar->SetPercent(MaxValue > 0.0f ? CurrentValue / MaxValue : 0.0f);
    }
}

void UAOMonsterHUDWidget::UpdateGroggyBar(float CurrentValue, float MaxValue)
{
    if (Pb_GroggyBar)
    {
        Pb_GroggyBar->SetPercent(MaxValue > 0.0f ? CurrentValue / MaxValue : 0.0f);
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
    UpdateGroggyBar(AttributeSet->GetGroggy(), AttributeSet->GetMaxGroggy());
}