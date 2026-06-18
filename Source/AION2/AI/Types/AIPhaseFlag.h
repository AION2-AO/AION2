
#pragma once 

#include "CoreMinimal.h"
#include "AIPhaseFlag.generated.h"


UENUM(BlueprintType, meta = (Bitflags))
enum class ETalythraState : uint8
{
	None UMETA(Hidden),
	Spawn,
	Idle,
	Chase,
	Ranged_Shot,
	Melee_Attack_Skill_05, 
	Dead,

};

ENUM_CLASS_FLAGS(ETalythraState);



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