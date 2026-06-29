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

	// 네트워크 보간 설정
	ProjectileMovement->bInterpMovement = true;          // 이동 보간 설정 
	ProjectileMovement->bInterpRotation = true;          // 회전 보간 설정
	ProjectileMovement->InterpLocationTime = 0.05f;      // Min Net Update(30Hz) 기준 안전값
	ProjectileMovement->InterpRotationTime = 0.025f;     // 회전 보간 시간 설정
	ProjectileMovement->InterpLocationMaxLagDistance = 300.f; // 일정이상 거리 멀어지면 보간 포기하고 스냅하는 설정 


	// Replication 설정 
	bReplicates = true;					// 이 Actor를 클라이언트에도 만들어지도록 설정
	SetReplicateMovement(true);			// 이 Actor의 위치/회전/속도를 클라이언트에 동기화 설정
	SetNetUpdateFrequency(100.f);		// 초당 최대 100번까지 클라에게 패킷을 보내라.
	SetMinNetUpdateFrequency(30.f);     // 변화가 없거나 우선순위가 낮아도 초당 최소 30번은 체크 보장해라. 

}

// Called when the game starts or when spawned
void ATalythraProjectile::BeginPlay()
{
	Super::BeginPlay();



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

