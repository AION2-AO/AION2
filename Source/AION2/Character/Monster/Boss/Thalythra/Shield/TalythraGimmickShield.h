// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/AOTypes.h"
#include "TalythraGimmickShield.generated.h"

UCLASS()
class AION2_API ATalythraGimmickShield : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATalythraGimmickShield();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;



	void Set_ShieldOrbColor(EOrbColor _OrbColor) { OrbColor = _OrbColor;  }

	UFUNCTION()
	void OnShieldBeginOverlapEvent(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnShieldEndOverlapEvent(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);


	

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> Collision;


	EOrbColor OrbColor = EOrbColor::None;

};

