#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AOTrapZone.generated.h"

class USphereComponent;
class UGameplayEffect;

UCLASS()
class AION2_API AAOTrapZone : public AActor
{
	GENERATED_BODY()
	
public:	
	AAOTrapZone();

protected:
	virtual void BeginPlay() override;
	void ApplyTrapDamage();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	TObjectPtr<USphereComponent> DamageCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
	TSubclassOf<UGameplayEffect> TrapDamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
	float TrapRadius = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
	float TrapDuration = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
	float DamageInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
	float DamageAmount = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
	FGameplayTag DamageSetByCallerTag;

private :
	FTimerHandle DamageTimerHandle;
};
