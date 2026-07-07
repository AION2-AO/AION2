// Fill out your copyright notice in the Description page of Project Settings.

#include "Network/AONetworkReceiverWorker.h"
#include "Common/TcpSocketBuilder.h"

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

	// ������ �⺻������ Non-Blocking���� �� ���� ������
	ClientSocket->SetNonBlocking(false);

	// ������ �����ֱ� ���� ���� �߰���
	while (bRunThread)
	{
		uint32 DataSize = 0;
		bool bHasData = ClientSocket->HasPendingData(DataSize);

		if (bHasData && DataSize > 0)
		{
			int32 BytesRead = 0;
			// ��� ���� ������ ũ��� BUFSIZE �� ���� ����ŭ�� �����ϰ� ������
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
						bRunThread = false; // ���� ���� ������
						break;
					}

					if (ReceiverBuffer.Num() - ProcessedOffset < Header->PacketSize)
					{
						break; // ������ �̿ϼ��̹Ƿ� ���� ������ ��ٸ�
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
			// ó���� �����Ͱ� ������ ������ �����带 ���� �Ͽ� CPU ������ ��������
			FPlatformProcess::Sleep(0.01f);
		}
	}

	return 0;
}

void AONetworkReceiverWorker::Stop()
{
	bRunThread = false;
}
