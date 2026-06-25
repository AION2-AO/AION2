// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"
#include "Network/PacketHeader.h"


/**
 * 
 */
class AION2_API AONetworkReceiverWorker : public FRunnable
{
public:
	AONetworkReceiverWorker();
	AONetworkReceiverWorker(FSocket* InSocket);
	virtual ~AONetworkReceiverWorker();

	virtual uint32 Run() override;
	virtual void Stop() override;

	void SetThread(FRunnableThread* InThread) { Thread = InThread; }

	bool PopPacket(FPacket& OutPacket)
	{
		FScopeLock Lock(&QueueLock);
		return PacketQueue.Dequeue(OutPacket);
	}

private:
	FSocket* ClientSocket;
	FRunnableThread* Thread;
	bool bRunThread;

	FCriticalSection QueueLock;
	TQueue< FPacket, EQueueMode::Mpsc> PacketQueue;
};
