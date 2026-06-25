// Fill out your copyright notice in the Description page of Project Settings.


#include "AONetworkManager.h"
#include "Game/AOGameInstance.h"
#include "UI/AOLoginUserWidget.h"
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
	/*{
		if (!Packet.PacketMessage) continue;
		switch (Packet.PacketId)
		{
		case PKT_S_SIGNUP:
		{
			auto Pkt = std::static_pointer_cast<Protocol::S_SignUpResultPacket>(Packet.PacketMessage);
			if (Pkt)
			{
				bool bSuccess = Pkt->success();
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
			auto Pkt = std::static_pointer_cast<Protocol::S_LoginFailPacket>(Packet.PacketMessage);
			if(Pkt)
			{
				UE_LOG(LogTemp, Error, TEXT("Login Failed!"));
				GameInstance->LoginWidget->HandleLoginResult();
			}
			break;
		}

		case PKT_S_SLOGIN:
		{
			uint64 PlayerId = 0;
			uint8 ClassType = 0;
			bool bSuccess = false;

			auto Pkt = std::static_pointer_cast<Protocol::S_LoginSuccessPacket>(Packet.PacketMessage);
			if(Pkt&&Pkt->has_playerinfo())
			{
				UE_LOG(LogTemp, Log, TEXT("Login Success!"));		
				if (PlayerMng)
				{
					PlayerId = Pkt->playerinfo().playerid();;
					ClassType = static_cast<uint8>(Pkt->playerinfo().playerclass());
					bSuccess = true;
				}
			}

			if (bSuccess && PlayerMng)
			{
				PlayerMng->HandleLogin(PlayerId, ClassType);
				GameInstance->OnReadyoOpenLevel();

			}
			break;
		}

		case PKT_S_ITEM:
		{
			auto Pkt = std::static_pointer_cast<Protocol::S_ItemDataPacket>(Packet.PacketMessage);

			if (Pkt)
			{
				int32 ItemCount = Pkt->playeritems_size();

				UE_LOG(LogTemp, Log, TEXT("Received Item Count: %d"), ItemCount);
				if (ItemCount == 0) break;
				for (int i = 0; i < ItemCount; i++)
				{
					const Protocol::ItemData& Item = Pkt->playeritems(i);

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
			auto Pkt = std::static_pointer_cast<Protocol::S_SpawnPacket>(Packet.PacketMessage);
			if (Pkt)
			{
				int32 SpawnCount = Pkt->playerstates_size();

				UE_LOG(LogTemp, Log, TEXT("Received Players Count: %d"), SpawnCount);
				for (int i = 0; i < SpawnCount; ++i)
				{
					const Protocol::PlayerState& Info = Pkt->playerstates(i);
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
	}*/
}
