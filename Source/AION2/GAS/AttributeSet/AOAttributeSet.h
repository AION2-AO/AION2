#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AOAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName) \

UCLASS()
class AION2_API UAOAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public :
	UAOAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes | Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Health);


	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes | Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Attributes | Mana")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Mana);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Attributes | Mana")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, MaxMana);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Attributes | Stamina")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Stamina);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Attributes | Stamina")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, MaxStamina);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Attributes | Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, AttackPower);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defense, Category = "Attributes | Combat")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Defense);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "Attributes | Movement")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, MoveSpeed);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level, Category = "Attributes | Progression")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Level);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Exp, Category = "Attributes | Progression")
	FGameplayAttributeData Exp;
	ATTRIBUTE_ACCESSORS(UAOAttributeSet, Exp);

private : 
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);	

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldDefense);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldLevel);

	UFUNCTION()
	void OnRep_Exp(const FGameplayAttributeData& OldExp);
};
