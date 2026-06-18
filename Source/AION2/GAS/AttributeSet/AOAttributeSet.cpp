#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UAOAttributeSet::UAOAttributeSet()
{
    InitHealth(100.f);
    InitMaxHealth(100.f);

    InitMana(100.f);
    InitMaxMana(100.f);

    InitStamina(100.f);
    InitMaxStamina(100.f);

    InitAttackPower(10.f);
    InitDefense(5.f);
    InitMoveSpeed(600.f);

    InitLevel(1.f);
    InitExp(0.f);
}

void UAOAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Mana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Defense, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Level, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAOAttributeSet, Exp, COND_None, REPNOTIFY_Always);
}

void UAOAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }

    if (Attribute == GetManaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
    }

    if (Attribute == GetStaminaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
    }


}

void UAOAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
    }

    if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
    }

    if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
    }
}

void UAOAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Health, OldHealth);
}

void UAOAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, MaxHealth, OldMaxHealth);
}

void UAOAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Mana, OldMana);
}

void UAOAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, MaxMana, OldMaxMana);
}

void UAOAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Stamina, OldStamina);
}

void UAOAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, MaxStamina, OldMaxStamina);
}

void UAOAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, AttackPower, OldAttackPower);
}

void UAOAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Defense, OldDefense);
}

void UAOAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, MoveSpeed, OldMoveSpeed);
}

void UAOAttributeSet::OnRep_Level(const FGameplayAttributeData& OldLevel)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Level, OldLevel);
}

void UAOAttributeSet::OnRep_Exp(const FGameplayAttributeData& OldExp)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAOAttributeSet, Exp, OldExp);
}
