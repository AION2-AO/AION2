// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AOCharacter.h"
#include "AI/Types/AIPhaseFlag.h"
#include "AOMonsterBase.generated.h"

UCLASS()
class AION2_API AAOMonsterBase : public AAOCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAOMonsterBase(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	// Navigation Mesh의 범위 밖에 이동인지 아닌지를 판단하는 함수 
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	bool CanMoveOnNavMesh(const FVector Direction, float Distance);


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;





};
