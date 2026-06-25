// Fill out your copyright notice in the Description page of Project Settings.

#include "Network/AONetworkReceiverWorker.h"
#include "Common/TcpSocketBuilder.h"
#include "Common/TcpSocketBuilder.h"
#include "Protocol.pb.h"
constexpr int BUFSIZE = 4096;

AONetworkReceiverWorker::AONetworkReceiverWorker()
{
}

AONetworkReceiverWorker::AONetworkReceiverWorker(FSocket* InSocket)
	:ClientSocket(InSocket), bRunThread(true)
{
}

AONetworkReceiverWorker::~AONetworkReceiverWorker()
{
	if (Thread)
	{
		Thread->Kill(true);
		delete Thread;
	}
}

uint32 AONetworkReceiverWorker::Run()
{
	if (!ClientSocket)
		return 0;

	TArray<uint8> ReceiverBuffer;
	uint8 TempData[BUFSIZE];

	// 소켓은 기본적으로 Non-Blocking으로 한 번만 세팅함임
	ClientSocket->SetNonBlocking(true);

	// 스레드 생명주기 제어 루프 추가함
	while (bRunThread)
	{
		uint32 DataSize = 0;
		bool bHasData = ClientSocket->HasPendingData(DataSize);

		if (bHasData && DataSize > 0)
		{
			int32 BytesRead = 0;
			// 대기 중인 데이터 크기와 BUFSIZE 중 작은 값만큼만 안전하게 읽음임
			int32 ReadSize = FMath::Min(DataSize, static_cast<uint32>(BUFSIZE));

			if (ClientSocket->Recv(TempData, ReadSize, BytesRead) && BytesRead > 0)
			{
				ReceiverBuffer.Append(TempData, BytesRead);
				int32 ProcessedOffset = 0;

				while (ReceiverBuffer.Num() - ProcessedOffset >= sizeof(FPacketHeader))
				{
					uint8* HeaderPtr = ReceiverBuffer.GetData() + ProcessedOffset;
					FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(HeaderPtr);

					if (Header->PacketSize > MAX_PACKET_SIZE || Header->PacketSize < sizeof(FPacketHeader))
					{
						UE_LOG(LogTemp, Error, TEXT("Invalid Packet Size: %d"), Header->PacketSize);
						bRunThread = false; // 루프 종료 유도함
						break;
					}

					if (ReceiverBuffer.Num() - ProcessedOffset < Header->PacketSize)
					{
						break; // 데이터 미완성이므로 다음 수신을 기다림
					}

					FPacket OnePacket;
					OnePacket.PacketId = Header->PacketId;

					OnePacket.RawPayload.Append(HeaderPtr, Header->PacketSize);
					{
						FScopeLock Lock(&QueueLock);
						PacketQueue.Enqueue(MoveTemp(OnePacket));
					}
					ProcessedOffset += Header->PacketSize;
				}

				if (ProcessedOffset > 0)
				{
					ReceiverBuffer.RemoveAt(0, ProcessedOffset, EAllowShrinking::No);
				}
			}
		}
		else
		{
			// 처리할 데이터가 없으면 무조건 스레드를 쉬게 하여 CPU 과점을 방지함임
			FPlatformProcess::Sleep(0.01f);
		}
	}

	return 0;
}

void AONetworkReceiverWorker::Stop()
{
	bRunThread = false;
}
