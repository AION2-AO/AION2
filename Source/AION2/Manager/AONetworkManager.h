// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <Sockets.h>
#include "AONetworkManager.generated.h"

UCLASS()
class AION2_API UAONetworkManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues);

	void SetSocket(FSocket* Socket) ;
	void SetPlayerManager();
public:
	void ReceiveData();
	void ResetBuffer();
	void ProcessQueuePackets();
	
private:
	FSocket* ClientSocket;

	TQueue<TArray<uint8>, EQueueMode::Spsc> ReceiveQueue;
	FCriticalSection QueueLock;

	TArray<uint8> ReceiverBuffer;

	class UAOGameInstance* GameInstance;
	class UAOPlayerManager* PlayerMng;
	const int32 MAX_PACKET_SIZE = 65535;	
};
