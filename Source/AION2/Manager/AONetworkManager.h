// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <Sockets.h>
#include "Network/AONetworkReceiverWorker.h"
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

public:
	class UAOGameInstance* GameInstance; 
	class UAOPlayerManager* PlayerMng;

private:
	FSocket* ClientSocket;

	// 리시버 워커 스레드를 소유할 고유 포인터 선언함
	TUniquePtr<AONetworkReceiverWorker> ReceiverWorker;

	const int32 MAX_PACKET_SIZE = 65535;	
};
