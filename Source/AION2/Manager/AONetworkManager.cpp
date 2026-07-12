// Fill out your copyright notice in the Description page of Project Settings.


#include "AONetworkManager.h"
#include "Game/AOGameInstance.h"
#include "Network/PacketHeader.h"
#include "Manager/AOPlayerManager.h"
#include "Manager/PacketHandlerManager.h"


void UAONetworkManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	static bool bInitialized = false;
	if (!bInitialized)
	{
		InitPacketHandler();
		bInitialized = true;
	}
}

void UAONetworkManager::Deinitialize()
{
	if (ReceiverWorker.IsValid())
	{
		ReceiverWorker->Stop();
		ReceiverWorker.Reset();
	}
	Super::Deinitialize();
}

void UAONetworkManager::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues)
{
}

void UAONetworkManager::SetSocket(FSocket* Socket)
{
	ClientSocket = Socket;
	SetPlayerManager();
	if (ClientSocket)
	{
		if (ReceiverWorker.IsValid())
		{
			ReceiverWorker->Stop();
			ReceiverWorker.Reset();
		}
		ReceiverWorker = MakeUnique<AONetworkReceiverWorker>(ClientSocket);
		FRunnableThread* WorkerThread = FRunnableThread::Create(ReceiverWorker.Get(), TEXT("AONetworkReceiverWorkerThread"), 0, TPri_BelowNormal);
		ReceiverWorker->SetThread(WorkerThread);
	}
}

void UAONetworkManager::ResetBuffer()
{
	UE_LOG(LogTemp, Warning, TEXT("Network Buffer Cleared for New Level"));
}

void UAONetworkManager::SetPlayerManager()
{
	if (GetWorld() == nullptr)
	{
		GameInstance = Cast<UAOGameInstance>(GetOuter());
	}

	else
	{
		GameInstance = Cast<UAOGameInstance>(GetWorld()->GetGameInstance());
	}

	if (GameInstance)
	{
		PlayerMng = GameInstance->GetSubsystem<UAOPlayerManager>();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetPlayerManager: GameInstance is null"));
	}
}

void UAONetworkManager::ReceiveData()
{
}

void UAONetworkManager::ProcessQueuePackets()
{
	if (!GameInstance || !PlayerMng || !ReceiverWorker.IsValid())
		return;

	FPacket Packet;
	if (!ReceiverWorker.IsValid())return;

	while (ReceiverWorker->PopPacket(Packet))
	{
		if (Packet.RawPayload.Num() >= sizeof(FPacketHeader))
		{
			uint8* PayloadPtr = Packet.RawPayload.GetData() + sizeof(FPacketHeader);
			int32 PayloadSize = Packet.RawPayload.Num() - sizeof(FPacketHeader);

			PacketHandlerFunc Handler = GAOPacketHandler[Packet.PacketId];
			if (Handler)
			{
				Handler(this, PayloadPtr, PayloadSize);
			}
		}
	}
}

void UAONetworkManager::Tick(float DeltaTime)
{
	ProcessQueuePackets();
}

bool UAONetworkManager::IsTickable() const
{
	return !IsTemplate();
}

TStatId UAONetworkManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAONetworkManager, STATGROUP_Tickables);
}

