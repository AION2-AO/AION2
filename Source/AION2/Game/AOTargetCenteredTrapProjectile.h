#pragma once

#include "CoreMinimal.h"
#include "Character/Daeva/AOTrapProjectile.h"
#include "AOTargetCenteredTrapProjectile.generated.h"

UCLASS()
class AION2_API AAOTargetCenteredTrapProjectile : public AAOTrapProjectile
{
	GENERATED_BODY()
	
protected :
	virtual FVector GetTrapSpawnLocation(AAOCharacter* HitCharacter, const FHitResult& SweepResult) const override;
};
