// Fill out your copyright notice in the Description page of Project Settings.


#include "AONetworkSubsystem.h"
#include "Game/AOGameInstance.h"

constexpr int BUFSIZE = 4096;

void UAONetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAONetworkSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UAONetworkSubsystem::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues)
{
	if (World && World->IsGameWorld())
	{
		ResetBuffer();
		UE_LOG(LogTemp, Warning, TEXT("Level Changed: Network Buffer Flushed."));
	}
}

void UAONetworkSubsystem::ResetBuffer()
{
	ReceiverBuffer.Empty();
	UE_LOG(LogTemp, Warning, TEXT("Network Buffer Cleared for New Level"));
}

void UAONetworkSubsystem::ReceiveData()
{
	if (!ClientSocket)	
		return;
	
	ClientSocket->SetNonBlocking(true);
	uint32 DataSize;
	while (ClientSocket->HasPendingData(DataSize))
	{
		uint8 TempData[BUFSIZE];
		int32 BytesRead = 0;
		
		if (ClientSocket->Recv(TempData, FMath::Min(DataSize, (uint32)BUFSIZE), BytesRead))
		{
			//받은 데이터를 받은 사이즈만큼 버퍼에 넣음
			ReceiverBuffer.Append(TempData, DataSize);
			int32 ProcessedOffset = 0;
			
			//받은 크기가 헤더 사이즈를 넘을 때까지
			while (ReceiverBuffer.Num() - ProcessedOffset >= sizeof(FPacketHeader))
			{
				uint8* HeaderPtr = ReceiverBuffer.GetData() + ProcessedOffset;
				FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(HeaderPtr);
				if (Header->PacketSize > MAX_PACKET_SIZE || Header->PacketSize < sizeof(FPacketHeader))
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid Packet Size: %d"), Header->PacketSize);
					// 연결 끊기 등 예외 처리 필요
					return;
				}

				//받은 데이터가 패킷 사이즈보다 작으면 다시 Recv
				if (ReceiverBuffer.Num() - ProcessedOffset < Header->PacketSize)
					break;

				//정돈된 패킷 하나를 큐에 넣기
				TArray<uint8> OnePacket;
				OnePacket.Append(HeaderPtr, Header->PacketSize);
				{
					FScopeLock Lock(&QueueLock); // 이 큐가 멀티스레드에서 쓰인다면 Lock 필요
					ReceiveQueue.Enqueue(MoveTemp(OnePacket));
				}
				ProcessedOffset += Header->PacketSize;
			}
		}
		else
		{
			break;
		}
	}
}

void UAONetworkSubsystem::ProcessQueuePackets()
{
	ReceiveData();
	GameInst = Cast<UAOGameInstance>(GetWorld()->GetGameInstance());

	if (!GameInst) return;

	TArray<uint8> PacketData;

	while (true)
	{
		{
			FScopeLock Lock(&QueueLock);
			//패킷 데이터 큐 데이터 넣기
			if (!ReceiveQueue.Dequeue(PacketData))
				break;
		}
		if (PacketData.Num() < sizeof(FPacketHeader))
			continue;

		FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(PacketData.GetData());
	}
}
