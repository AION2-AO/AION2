// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Network/PacketHeader.h"
#include "Types/DungeonRoomTypes.h"
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
	void HandleLogin(const uint64 PlayerId, const uint8 ClassType);
	void HandleSpawn(const uint64 PlayerId, const FString PlayerName, uint8 ClassType, FVector SpawnLocation, FRotator SpawnRotation);
	void HandleItem(Protocol::S_ItemDataPacket Items);
	void HnadleMove(const uint64 PlayerId, FVector NewLocation, FRotator NewRotation, FVector NewVel);

	void HandleDungeonCreate(int32 DungeonId);
	void HandleDungeonEnter(int32 DungeonId);
	void HandleDungeonStart(FString ServerURL);

	void HandleChatting(FString SenderName, FString SendMessage);
	void HandleStorePurchase(Protocol::ItemData ItemInfo);
	void HandleUseItem(const Protocol::S_UseItemPacket& Pkt);
	
	void HandleDungeonSetPlayerInfo(const Protocol::S_DungeonStartDediPacket& Info);

	void HandleDisconnect(uint64 RemovePlayerId);

#pragma region Dungeon State
public:
	void ClearMyDungeonRoomState();

	bool TryUpdateMyDungeonRoomState(const Protocol::DungeonInfo& DungeonInfo);

	// 방 목록 전체용 Update 함수
	void UpdateMyDungeonRoomStateFromList(const google::protobuf::RepeatedPtrField<Protocol::DungeonInfo>& DungeonInfos);

	void UpdateMyDungeonEnterState(int32 DungeonId, const Protocol::DungeonPlayerInfo& EnterPlayer);
	void UpdateMyDungeonReadyState(int32 DungeonId, uint64 PlayerId);

	const FPlayerDungeonRoomState& GetMyDungeonRoomState() const { return MyDungeonRoomState; }

private:
	UPROPERTY()
	FPlayerDungeonRoomState MyDungeonRoomState;

#pragma endregion

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
