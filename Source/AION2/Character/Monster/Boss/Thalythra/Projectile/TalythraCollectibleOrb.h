// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/CombatInterface.h"
#include "Types/AOTypes.h"
#include "TalythraCollectibleOrb.generated.h"



UCLASS()
class AION2_API ATalythraCollectibleOrb : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATalythraCollectibleOrb();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


    UFUNCTION()
    void OnProjectileOverlapEvent(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );


    void InitVelocityAndDirection(const FVector Direction);
    FORCEINLINE void Set_OrbColor(EOrbColor _EOrbColor) { OrbColor = _EOrbColor; }



private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class USphereComponent> Collision; 

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(EditAnywhere, Category = "AttackData", meta = (AllowPrivateAccess = "true"))
    FAttackData AttackData;



private:
    UPROPERTY()
    EOrbColor OrbColor = EOrbColor::None;


};
