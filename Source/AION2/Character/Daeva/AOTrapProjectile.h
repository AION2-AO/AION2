#pragma once

#include "CoreMinimal.h"
#include "AOTrapZone.h"
#include "Actor/AOProjectile.h"
#include "AOTrapProjectile.generated.h"

class UGameplayEffect;

UCLASS()
class AION2_API AAOTrapProjectile : public AAOProjectile
{
	GENERATED_BODY()
	
public :
	AAOTrapProjectile();

protected :
	virtual void BeginPlay() override;

	UFUNCTION()
	void SpawnTrapOnOverlap(UPrimitiveComponent* OverlappedComp,AActor* OtherActor,	UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);

public :
	UPROPERTY(EditDefaultsOnly, blueprintReadOnly, Category = "Trap")
	TSubclassOf<AAOTrapZone> TrapZoneClass;

private:
	bool bTrapSpawned = false;
};
