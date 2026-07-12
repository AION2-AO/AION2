#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AOFallDeathVolume.generated.h"

class UBoxComponent;

UCLASS()
class AION2_API AAOFallDeathVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	AAOFallDeathVolume();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);
};
