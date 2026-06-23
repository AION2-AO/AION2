#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "AOPlayerState.generated.h"

class UAbilitySystemComponent;
class UDA_AbilitySet;
class UAOAttributeSet;

UENUM(BlueprintType)
enum class EDaevaClassType : uint8
{
	None,
	Templar,
	Assassin,
	Ranger,
	Cleric
};

UCLASS()
class AION2_API AAOPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AAOPlayerState();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

private:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	EDaevaClassType ClassType = EDaevaClassType::None;
};
