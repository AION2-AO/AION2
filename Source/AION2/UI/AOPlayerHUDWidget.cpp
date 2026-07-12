// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOPlayerHUDWidget.h"

#include "UI/AOMonsterHUDWidget.h"
#include "UI/AOQuickSkillHUD.h"
#include "UI/AOClassSwitcherWidget.h"

#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Player/AOPlayerState.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"



// TODO(SuYeon): Delegate마다, 만약 다른 ASC와 연동되어있다면 로그를 출력하거나 하는 방어적 코드 추가 +Monster의 것에도 추가할 것.


void UAOPlayerHUDWidget::BindToASC(UAbilitySystemComponent* InASC)
{
    // Validation Check
    if (!InASC)
    {
        return;
    }

	// 기존 ASC에 묵인 delegate 먼제 제거.
	// BoundASC가 새 ASC로 바뀐 뒤에 제거하면 ASC delegate가 남을 수 있기 때문입니다.
	UnbindASCDelegates();

	Super::BindToASC(InASC);

 //   const bool bASCChanged = BoundASC != InASC;

	//if (bASCChanged)
	//{
	//	// ASC가 바뀌었을 때만 재호출
	//	Super::BindToASC(InASC);
	//}
	//else
	//{
	//	UnbindASCDelegates();
	//}

	/*
	* ASC가 바뀜 => 상위 Bind 로직 호출 => 여전히 null이면,
	* 애초에 InASC가 null로 들어온 것.
	* => early return.
	*/
	if (!BoundASC)
	{
		return;
	}

    if (QuickSkillHUD)
    {
		//UE_LOG(LogTemp, Warning, TEXT("PlayerHUD BindToASC. ASC=%s, QuickSkillHUD=%s"),
		//	*GetNameSafe(InASC),
		//	*GetNameSafe(QuickSkillHUD));

        QuickSkillHUD->BindToASC(BoundASC);
    }

    BindASCDelegates();
    BroadcastInitialAttributes();
}

void UAOPlayerHUDWidget::UnbindFromASC()
{
	if (QuickSkillHUD)
	{
		QuickSkillHUD->UnbindFromASC();
	}

	UnbindASCDelegates();
	Super::UnbindFromASC();
}

void UAOPlayerHUDWidget::NativeDestruct()
{
	ClearBinding();
	Super::NativeDestruct();
}

void UAOPlayerHUDWidget::ChangeClassIcon(EDaevaClassType InClassType)
{
	if (!PlayerClassSwitcher)
	{
		return;
	}

	// 나중에 ClassSwitcher 변경되면 ClassType 자체로 넣어보기.
	PlayerClassSwitcher->SetClassWidget(InClassType);
	PlayerClassSwitcher->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UAOPlayerHUDWidget::SetPlayerName(const FText PlayerName)
{
	if (TB_PlayerName)
	{
		TB_PlayerName->SetText(PlayerName);
	}
}

void UAOPlayerHUDWidget::HandleHealthChanged(const FOnAttributeChangeData& Data)
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

	UpdateHpBar(Data.NewValue, AttributeSet->GetMaxHealth());
}

void UAOPlayerHUDWidget::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
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

	UpdateHpBar(AttributeSet->GetHealth(), Data.NewValue);
}

void UAOPlayerHUDWidget::HandleManaChanged(const FOnAttributeChangeData& Data)
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

	UpdateManaBar(Data.NewValue, AttributeSet->GetMaxMana());
}

void UAOPlayerHUDWidget::HandleMaxManaChanged(const FOnAttributeChangeData& Data)
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

	UpdateManaBar(AttributeSet->GetMana(), Data.NewValue);
}

void UAOPlayerHUDWidget::HandleStaminaChanged(const FOnAttributeChangeData& Data)
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

	UpdateStaminaBar(Data.NewValue, AttributeSet->GetMaxStamina());
}

void UAOPlayerHUDWidget::HandleMaxStaminaChanged(const FOnAttributeChangeData& Data)
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

	UpdateStaminaBar(AttributeSet->GetStamina(), Data.NewValue);
}

void UAOPlayerHUDWidget::BindASCDelegates()
{
    // 방어적으로 시작.
    UnbindASCDelegates();

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

    // Health Bind.
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

	// Mana Bind.
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

	// Stamina Bind.
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

void UAOPlayerHUDWidget::PlaySkillPressedFeedback(int32 InputId)
{
	if (QuickSkillHUD)
	{
		QuickSkillHUD->PlaySkillPressedFeedback(InputId);
	}
}
