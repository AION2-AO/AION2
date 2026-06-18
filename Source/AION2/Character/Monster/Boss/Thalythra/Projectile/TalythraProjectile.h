// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TalythraProjectile.generated.h"

UCLASS()
class AION2_API ATalythraProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATalythraProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


    UFUNCTION()
    void OnProjectileHit(
        UPrimitiveComponent* HitComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit
    );


    void InitVelocityAndDirection(const FVector Direction);



private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

    


};
