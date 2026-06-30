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
class AMMODaeva;
UCLASS()
class AION2_API UAOPlayerManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
private:
	UAOPlayerManager();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;	

public:
	void HandleLogin(uint64 PlayerId, uint8 ClassType);
	void HandleSpawn(uint64 PlayerId, FString PlayerName, uint8 ClassType, FVector SpawnLocation, FRotator SpawnRotation);
	void HandleItem(Protocol::S_ItemDataPacket Items);
	void HnadleMove(uint64 PlayerId, FVector NewLocation, FRotator NewRotation, FVector NewVel);
	void HandleDungeonCreate(int32 DungeonId);
	void HandleDungeonEnter(int32 DungeonId);
	void HandleDungeonStart(FString ServerURL);

private:
	FVector PlayerStartLocation = FVector(442.0f,-147.0f, 90.0f );

private:	
	UPROPERTY()
	TObjectPtr<AMMODaeva> MyPlayer;

	UPROPERTY()
	TMap<uint64, TObjectPtr<AMMODaeva>> Players;

	UPROPERTY()
	TMap<uint64, FPlayerInfo> PlayerInfos;
	
	TSubclassOf <AMMODaeva> PlayerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	TMap<uint8, TSubclassOf<APawn>> JobClassMap;
	
	TMap<int32, Protocol::ItemData> MyItems;

private:
	class UAOGameInstance* GameInstance;
	UWorld* World;
};
