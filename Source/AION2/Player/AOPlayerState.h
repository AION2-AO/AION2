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
	Assassin,
	Cleric,
	Ranger,
	Templar,
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

	void SetMyId(uint64 PlayerId);
	void SetMyClass(EDaevaClassType InClassType);
	void SetMyName(FString InName);

	// H.Y.
	void SetPlayerInfo(uint64 InPalyerId, const FString& InPlayerName, uint8 InCalssType);

	// GetClass function.
	FORCEINLINE EDaevaClassType GetMyClass() const{ return MyClassType; }

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
	// H.Y
	UPROPERTY(Replicated)
	uint64 MyId = 0;
	// 
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Info", meta = (AllowPrivateAccess = "true"))
	EDaevaClassType MyClassType = EDaevaClassType::None;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Info", meta = (AllowPrivateAccess = "true"))
	FString MyName;
};
