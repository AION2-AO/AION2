#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/GA_Attack.h"
#include "GA_Cleric_Key4.generated.h"

UCLASS()
class AION2_API UGA_Cleric_Key4 : public UGA_Attack
{
	GENERATED_BODY()

protected:
	virtual void OnCheckAttackHitEvent(FGameplayEventData Payload) override;
};
