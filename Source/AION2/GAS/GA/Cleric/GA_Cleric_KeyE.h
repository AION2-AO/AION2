#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/GA_Attack.h"
#include "GA_Cleric_KeyE.generated.h"

UCLASS()
class AION2_API UGA_Cleric_KeyE : public UGA_Attack
{
	GENERATED_BODY()

protected:
	virtual void OnCheckAttackHitEvent(FGameplayEventData Payload) override;
};
