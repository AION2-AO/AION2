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
	void SetMyHealth(float InHealth);
	void SetMyItem(FString InItem);

	// H.Y
	void SetPlayerInfo(uint64 InPalyerId, const FString& InPlayerName, uint8 InCalssType,float InHp/*, FString& InItem*/);

	// GetClass function.
	FORCEINLINE uint64 GetMyId() const { return MyId; }
	FORCEINLINE EDaevaClassType GetMyClass() const { return MyClassType; }
	FORCEINLINE float GetInitialHP() const { return InitialHP; }
	FORCEINLINE void SetInitialHP(float InHP) { InitialHP = InHP; }
	FORCEINLINE FString GetMyName() const { return MyName; }

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

private:
	float InitialHP = 0.0f;
};
