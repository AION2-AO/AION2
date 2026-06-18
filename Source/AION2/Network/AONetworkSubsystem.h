// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <Sockets.h>
#include "AONetworkSubsystem.generated.h"

USTRUCT()
struct FPacketHeader
{
	GENERATED_BODY()
	
	uint16 PacketSize;
	uint16 PacketId;
};

UCLASS()
class AION2_API UAONetworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues);

	void SetSocket(FSocket* Socket) { ClientSocket = Socket; }

public:
	void ReceiveData();
	void ResetBuffer();
	void ProcessQueuePackets();
	
private:
	FSocket* ClientSocket;

	TQueue<TArray<uint8>, EQueueMode::Spsc> ReceiveQueue;
	FCriticalSection QueueLock;

	TArray<uint8> ReceiverBuffer;

	class UAOGameInstance* GameInst;
	const int32 MAX_PACKET_SIZE = 65535;

};
