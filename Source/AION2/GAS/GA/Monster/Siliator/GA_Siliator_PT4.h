#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Monster/GA_Monster_Attack.h"
#include "GA_Siliator_PT4.generated.h"

UCLASS()
class AION2_API UGA_Siliator_PT4 : public UGA_Monster_Attack
{
	GENERATED_BODY()
	
protected:
	virtual void OnCheckAttackHitEvent(FGameplayEventData Payload);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Siliator|Shuriken")
	TSubclassOf<class ASiliatorShuriken> ShurikenClass;

	UPROPERTY(EditDefaultsOnly, Category = "Siliator|Shuriken")
	float ShurikenForwardOffset = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Siliator|Shuriken")
	float ShurikenSideOffset = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "Siliator|Shuriken")
	float ShurikenHeightOffset = 150.f;
};
