
#pragma once 

#include "CoreMinimal.h"
#include "Interface/CombatInterface.h"
#include "TalythraTypes.generated.h"



UENUM(BlueprintType, meta = (Bitflags))
enum class ETalythraPhase : uint8
{
	None UMETA(Hidden),
	Combat,
	OutOfCombat,
	PreCombat,
	DEAD,
};

ENUM_CLASS_FLAGS(ETalythraPhase);


UENUM(BlueprintType, meta = (Bitflags))
enum class ETalythraState : uint8
{
	None UMETA(Hidden),
	Spawn,
	Idle,
	Chase,
	Ranged_Shot,
	Melee_Attack_Skill_05, 
	Charge_Attack,
	Teleport_Attack,
	Dead,

};

ENUM_CLASS_FLAGS(ETalythraState);



UENUM(BlueprintType )
enum class ETalythraMontageID : uint8
{
	None UMETA(Hidden),
	Montage_ChargeAttack,
	Montage_CloseAttack, 
	Montage_RangedShot,
	Montage_TeleportAttack,
	Montage_WaveAttack, 
};

ENUM_CLASS_FLAGS(ETalythraMontageID);



UENUM(BlueprintType)
enum class ETalythraAbilityID : uint8
{
	None UMETA(Hidden),
	Ability_CloseAttack,
	Ability_RangedShot,
	Ability_TeleportAttack,
	Ability_WaveAttack,
};

ENUM_CLASS_FLAGS(ETalythraAbilityID);



USTRUCT(BlueprintType)
struct FMonsterAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly)
	float AvailableRange = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	FTraceData TraceData;
};

