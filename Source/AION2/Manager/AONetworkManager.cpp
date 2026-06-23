// Fill out your copyright notice in the Description page of Project Settings.


#include "AONetworkManager.h"
#include "Game/AOGameInstance.h"
#include "UI/AOLoginUserWidget.h"
#include "Network/PacketHeader.h"
#include "Manager/AOPlayerManager.h"

constexpr int BUFSIZE = 4096;

void UAONetworkManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAONetworkManager::Deinitialize()
{
	Super::Deinitialize();
}

void UAONetworkManager::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues)
{
	if (World && World->IsGameWorld())
	{
		FScopeLock Lock(&QueueLock); // 큐와 버퍼 접근을 동시에 보호
		ResetBuffer();
		ReceiveQueue.Empty(); // 대기 중인 패킷 큐도 반드시 비워야 함
		UE_LOG(LogTemp, Warning, TEXT("Level Changed: Network Buffer and Queue Flushed."));
	}
}

void UAONetworkManager::SetSocket(FSocket* Socket)
{
	ClientSocket = Socket;
	SetPlayerManager();
}

void UAONetworkManager::ResetBuffer()
{
	ReceiverBuffer.Empty();
	UE_LOG(LogTemp, Warning, TEXT("Network Buffer Cleared for New Level"));
}

void UAONetworkManager::SetPlayerManager()
{
	GameInstance = Cast<UAOGameInstance>(GetWorld()->GetGameInstance());
	if (GameInstance)
	{
		PlayerMng = GameInstance->GetSubsystem<UAOPlayerManager>();
	}
}

void UAONetworkManager::ReceiveData()
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
			ReceiverBuffer.Append(TempData, BytesRead);
			int32 ProcessedOffset = 0;

			//받은 크기가 헤더 사이즈를 넘을 때까지
			while (ReceiverBuffer.Num() - ProcessedOffset >= sizeof(FPacketHeader))
			{
				uint8* HeaderPtr = ReceiverBuffer.GetData() + ProcessedOffset;
				FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(HeaderPtr);

				UE_LOG(LogTemp, Warning, TEXT("[ReceiveData] 버퍼남은크기: %d, 패킷Id: %d, 예고된PacketSize: %d"),
					ReceiverBuffer.Num() - ProcessedOffset, Header->PacketId, Header->PacketSize);

				if (Header->PacketSize > MAX_PACKET_SIZE || Header->PacketSize < sizeof(FPacketHeader))
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid Packet Size: %d"), Header->PacketSize);
					// 연결 끊기 등 예외 처리 필요
					return;
				}

				if (ReceiverBuffer.Num() - ProcessedOffset < Header->PacketSize)
				{
					// [로그 추가] 데이터가 아직 다 안 와서 짤린 경우
					UE_LOG(LogTemp, Warning, TEXT("[ReceiveData] 패킷 미완성 데이터 대기 기동 (요구: %d, 남은버퍼: %d)"),
						Header->PacketSize, ReceiverBuffer.Num() - ProcessedOffset);
					break;
				}

				//정돈된 패킷 하나를 큐에 넣기
				TArray<uint8> OnePacket;
				OnePacket.Append(HeaderPtr, Header->PacketSize);
				{
					FScopeLock Lock(&QueueLock); // 이 큐가 멀티스레드에서 쓰인다면 Lock 필요
					ReceiveQueue.Enqueue(MoveTemp(OnePacket));
				}
				ProcessedOffset += Header->PacketSize;
			}

			//버퍼를 처음부터 해석한 패킥 사이즈만큼 지우기
			if (ProcessedOffset > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("[ReceiveData] 버퍼 정리 전 전체크기: %d, 지울 크기(ProcessedOffset): %d"),
					ReceiverBuffer.Num(), ProcessedOffset);
				ReceiverBuffer.RemoveAt(0, ProcessedOffset, EAllowShrinking::No);
			}
		}
		else
		{
			break;
		}
	}
}

void UAONetworkManager::ProcessQueuePackets()
{
	ReceiveData();


	if (!GameInstance) return;

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

		uint8* PayloadPtr = PacketData.GetData() + sizeof(FPacketHeader);
		int32 PayloadSize = PacketData.Num() - sizeof(FPacketHeader);

		UE_LOG(LogTemp, Error, TEXT("[ProcessQueue] 패킷Id: %d, 꺼낸 고유 패킷 전체 크기: %d, 계산된 PayloadSize: %d"),
			Header->PacketId, PacketData.Num(), PayloadSize);

		switch (Header->PacketId)
		{
		case PKT_S_SIGNUP:
		{
			Protocol::S_SignUpResultPacket Pkt;
			if (Pkt.ParseFromArray(PayloadPtr, PayloadSize))
			{
				bool bSuccess = Pkt.success();
				UE_LOG(LogTemp, Log, TEXT("SignUp Result Received: %d"), bSuccess);
				if (!bSuccess)
				{
					if (GameInstance && GameInstance->LoginWidget)
					{
						GameInstance->LoginWidget->HandleRegisterError();
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("LoginWidget is not Valid."));
					}
				}
				else
				{
					GameInstance->LoginWidget->HandleRegisterResult();
				}
			}
			break;
		}

		case PKT_S_FLOGIN:
		{
			Protocol::S_LoginFailPacket Pkt;
			UE_LOG(LogTemp, Error, TEXT("Login Failed!"));
			GameInstance->LoginWidget->HandleLoginResult();
			break;
		}

		case PKT_S_SLOGIN:
		{
			uint64 PlayerId = 0;
			uint8 ClassType = 0;
			bool bSuccess = false;

			{
				Protocol::S_LoginSuccessPacket Pkt;
				if (Pkt.ParseFromArray(PayloadPtr, PayloadSize))
				{
					UE_LOG(LogTemp, Log, TEXT("Login Success!"));
					if (PlayerMng)
					{
						PlayerId = Pkt.playerinfo().playerid();
						ClassType = static_cast<uint8>(Pkt.playerinfo().playerclass());
						bSuccess = true;
					}
				}
			}

			if(bSuccess && PlayerMng)
			{
				PlayerMng->HandleLogin(PlayerId, ClassType);
				GameInstance->OnReadyoOpenLevel();

			}
		break;
		}

		case PKT_S_ITEM:
		{
			Protocol::S_ItemDataPacket Pkt;
			if (Pkt.ParseFromArray(PayloadPtr, PayloadSize))
			{
				int32 ItemCount = Pkt.playeritems_size();

				UE_LOG(LogTemp, Log, TEXT("Received Item Count: %d"), ItemCount);
				if (ItemCount == 0) break;
				for (int i = 0; i < ItemCount; i++)
				{
					const Protocol::ItemData& Item = Pkt.playeritems(i);

					int32 InstanceId = Item.iteminstancedid();
					int32 TemplateId = Item.itemtemplateid();
					int32 SlotIndex = Item.slotindex();
					int32 Count = Item.count();
				}
			}
			break;
		}

		case PKT_S_SPAWN:
		{
			Protocol::S_SpawnPacket Pkt;
			if (Pkt.ParseFromArray(PayloadPtr, PayloadSize))
			{
				int32 SpawnCount = Pkt.playerstates_size();

				UE_LOG(LogTemp, Log, TEXT("Received Players Count: %d"), SpawnCount);
				for (int i = 0; i < SpawnCount; ++i)
				{
					const Protocol::PlayerState& Info = Pkt.playerstates(i);
					uint64 PlayerId = Info.playerinfo().playerid();
					FVector Location = FVector(Info.playerpos().x(), Info.playerpos().y(), Info.playerpos().z());
					uint8 CalssType = static_cast<uint8>(Info.playerinfo().playerclass());
					PlayerMng->HandleSpawn(PlayerId, CalssType, Location);
				}
			}
			break;
		}

		default:
			break;
	}
}
}
