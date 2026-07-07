
#pragma once 

#include "CoreMinimal.h"
#include "Interface/CombatInterface.h"
#include "WaterEleTypes.generated.h"

UENUM(BlueprintType, meta = (Bitflags))
enum class EWaterElePhase : uint8
{
	None UMETA(Hidden),

	DEAD,
};
ENUM_CLASS_FLAGS(EWaterElePhase);

UENUM(BlueprintType, meta = (Bitflags))
enum class EWaterEleState : uint8
{
	None UMETA(Hidden),
	Spawn,
	Idle,
	Chase,
	Cannon,
	Reload,
	Dead,
};
ENUM_CLASS_FLAGS(EWaterEleState);

UENUM(BlueprintType)
enum class EWaterEleMontageID : uint8
{
	None UMETA(Hidden),
	Montage_Cannon,
	Montage_ReloadStart,
	Montage_ReloadEnd,
	Montage_Dead,
};

ENUM_CLASS_FLAGS(EWaterEleMontageID);

UENUM(BlueprintType)
enum class EWaterEleAbilityID : uint8
{
	None UMETA(Hidden),
	Ability_Cannon,

};
ENUM_CLASS_FLAGS(EWaterEleAbilityID);

USTRUCT(BlueprintType)
struct FWEMonsterAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float AvailableRange = 200.0f;
};

