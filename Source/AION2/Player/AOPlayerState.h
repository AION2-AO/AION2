#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "AOPlayerState.generated.h"

class UAbilitySystemComponent;
class UDA_AbilitySet;
class UAOAttributeSet;

UCLASS()
class AION2_API AAOPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AAOPlayerState();

public:
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAOAttributeSet* GetAttributeSet() const;

	void GiveCommonAbilities();

private:
	UPROPERTY(EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAbilitySystemComponent> ASC;

	UPROPERTY(EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAOAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDA_AbilitySet> CommonAbilitySet;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> CommonAbilityHandles;

	bool bCommonAbilitiesGiven = false;
};
