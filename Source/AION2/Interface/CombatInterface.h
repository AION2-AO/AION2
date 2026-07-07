#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Abilities/GameplayAbility.h"
#include "CombatInterface.generated.h"

UINTERFACE(MinimalAPI)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FTraceData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float Range = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float Radius = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	FVector StartOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly)
	FVector Direction = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly)
	float AvailableRange = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag HitGameplayCueTag;

	UPROPERTY(EditDefaultsOnly)
	FTraceData TraceData;

	UPROPERTY(EditDefaultsOnly)
	bool bRestoreManaOnHit = false;
};

class AION2_API ICombatInterface
{
	GENERATED_BODY()

public:
	virtual void CheckAttackHit(const FAttackData& AttackData) = 0;
};
