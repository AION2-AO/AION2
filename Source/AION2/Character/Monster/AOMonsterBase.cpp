// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/AOMonsterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"

// Sets default values
AAOMonsterBase::AAOMonsterBase(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
}

// Called when the game starts or when spawned
void AAOMonsterBase::BeginPlay()
{
    Super::BeginPlay();

}


// Called every frame
void AAOMonsterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAOMonsterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

}


bool AAOMonsterBase::CanMoveOnNavMesh(const FVector Direction, float Distance)
{
    const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

    if (!NavSys)
        return false;

    FVector Dir2D = Direction;
    Dir2D.Z = 0.f;

    if (!Dir2D.Normalize())
        return false;

    // Start를 발바닥 높이로 수정  
    const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    FVector Start = GetActorLocation();
    Start.Z -= HalfHeight;

    const FVector End = Start + Dir2D * Distance;

    FVector HitLocation;
    const bool bHit = NavSys->NavigationRaycast(GetWorld(), Start, End, HitLocation);


    return !bHit;
}