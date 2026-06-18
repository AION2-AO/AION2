// Fill out your copyright notice in the Description page of Project Settings.


#include "TalythraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"



// Sets default values
ATalythraProjectile::ATalythraProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

	RootComponent = Collision;



	bReplicates = true;
	SetReplicateMovement(true);


}

// Called when the game starts or when spawned
void ATalythraProjectile::BeginPlay()
{
	Super::BeginPlay();


	if (HasAuthority() == false)
	{
		if (ProjectileMovement)
		{
			ProjectileMovement->StopMovementImmediately();
			ProjectileMovement->SetComponentTickEnabled(false);
		}
	}

	
}

// Called every frame
void ATalythraProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATalythraProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{

}

void ATalythraProjectile::InitVelocityAndDirection(const FVector Direction)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity =
			Direction.GetSafeNormal() * ProjectileMovement->InitialSpeed;
	}
}

