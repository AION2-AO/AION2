// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Network/PacketHeader.h"
#include <unordered_map>
#include "AOPlayerManager.generated.h"

/**
 * 
 */
class ADaeva;
UCLASS()
class AION2_API UAOPlayerManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
private:
	UAOPlayerManager();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;	

public:
	void HandleLogin(uint64 PlayerId, uint8 ClassType);
	void HandleSpawn(uint64 PlayerId, uint8 ClassType, FVector SpawnLocation, FRotator SpawnRotation);
	void HnadleMove(uint64 PlayerId, FVector NewLocation, FRotator NewRotation, FVector NewVel);
	
private:
	FVector PlayerStartLocation = FVector(442.0f,-147.0f, 90.0f );

private:	
	UPROPERTY()
	TObjectPtr<ADaeva> MyPlayer;

	UPROPERTY()
	TMap<uint64, TObjectPtr<ADaeva>> PlayerInfos;
	
	TSubclassOf <ADaeva> PlayerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	TMap<uint8, TSubclassOf<APawn>> JobClassMap;
	
	class UAOGameInstance* GameInstance;
	UWorld* World;
};
