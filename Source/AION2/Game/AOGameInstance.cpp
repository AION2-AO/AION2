// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameInstance.h"
#include <Sockets.h>
#include "Common/TcpSocketBuilder.h"
#include "SocketSubsystem.h"
#include "Manager/AONetworkManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UAOGameInstance::Init()
{
	Super::Init();

#if UE_BUILD_DEVELOPMENT
	TryAsyncConnect("127.0.0.1", 7777);
#endif
}

void UAOGameInstance::TryAsyncConnect(const FString& Ip, int32 Port)
{
	TWeakObjectPtr<UAOGameInstance>WeakInstPtr(this);
	if (!WeakInstPtr.IsValid())return;
	Async(EAsyncExecution::Thread, [WeakInstPtr, Ip, Port]()
		{
			if (WeakInstPtr->ConnectToServer(Ip, Port))
			{
				AsyncTask(ENamedThreads::GameThread, [WeakInstPtr]()
					{
						if (!WeakInstPtr.IsValid()) return;
						WeakInstPtr->UNetworkManager = WeakInstPtr->GetSubsystem<UAONetworkManager>();
						if (WeakInstPtr->UNetworkManager)
						{
							WeakInstPtr->UNetworkManager->SetSocket(WeakInstPtr->ClientSocket);
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("NetWorkManager is null"));
						}
					});
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread, []()
					{
					});
			}
		});
}

bool UAOGameInstance::ConnectToServer(const FString& Ip, int32 Port)
{
	if (ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		UE_LOG(LogTemp, Error, TEXT("Already Connect To Server"));
		return true;
	}

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("GameClientSocket"));

	if (!ClientSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Socket Creation Failed"));
		return false;
	}

	FIPv4Address IpAddress;
	if (!FIPv4Address::Parse(Ip, IpAddress))
	{
		UE_LOG(LogTemp, Error, TEXT("Invaild Ip Address format: %s"), *Ip);
		return false;
	}
	TSharedRef<FInternetAddr>Addr = SocketSubsystem->CreateInternetAddr();
	Addr->SetIp(IpAddress.Value);
	Addr->SetPort(Port);

	if (!ClientSocket->Connect(*Addr))
	{
		int32 ErrorCode = SocketSubsystem->GetLastErrorCode();
		FString ErrorMessage = SocketSubsystem->GetSocketError(static_cast<ESocketErrors>(ErrorCode));
		UE_LOG(LogTemp, Error, TEXT("Failed to connect: %s (Code %d)"), *ErrorMessage, ErrorCode);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Connected to server: %s:%d"), *Ip, Port);
	return true;
}

bool UAOGameInstance::IsServerConnected()
{
	if (ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		return true;
	}
	else
		return false;
}

void UAOGameInstance::SendSignUpPacket(const FString& Id, const FString& Password, int32 ClassType)
{
	UE_LOG(LogTemp, Display, TEXT("Send SignUp Packet: %s, %s, %d"), *Id, *Password, ClassType);
	Protocol::C_SignUpPacket Pkt;
	Pkt.set_id(TCHAR_TO_UTF8(*Id));
	Pkt.set_password(TCHAR_TO_UTF8(*Password));
	Pkt.set_classtype(static_cast<Protocol::ClassType>(ClassType));

	SendPacket(Pkt, PKT_C_SIGNUP);
} 

void UAOGameInstance::SendLoginPacket(const FString& Id, const FString& Password)
{
	Protocol::C_LoginPacket Pkt;
	Pkt.set_id(TCHAR_TO_UTF8(*Id));
	Pkt.set_password(TCHAR_TO_UTF8(*Password));

	SendPacket(Pkt, PKT_C_LOGIN);
}

void UAOGameInstance::SendMapLoadCompletePacket()
{
	Protocol::C_MapLoadCompletePacket Pkt;
	SendPacket(Pkt, PKT_C_MAPLOADCOMPLETE);
}

void UAOGameInstance::OpenVillageLevel()
{
	FString VillagePath = TEXT("/Game/Map/Village");
	UGameplayStatics::OpenLevel(GetWorld(), *VillagePath, true);
}

void UAOGameInstance::OnReadyoOpenLevel()
{
	if (UWorld* World = GetWorld())
	{
		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, this, &UAOGameInstance::OpenVillageLevel, 1.0f, false);
	}
}

void UAOGameInstance::SendPacket(void* Packet, int32 PacketSize)
{
	int32 bytesSent = 0;

	if (!ClientSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Invaild Socket"));
		return;
	}
	if (!ClientSocket->Send(reinterpret_cast<uint8*>(Packet), PacketSize, bytesSent))
	{
		UE_LOG(LogTemp, Error, TEXT("Packet Send Fail"));
	}
	else
	{

	}
}

void UAOGameInstance::SendPacket(google::protobuf::Message& Pkt, uint16 PacketId)
{
	const uint16 DataSize = static_cast<uint16>(Pkt.ByteSizeLong());
	const uint16 PacketSize = DataSize + sizeof(FPacketHeader);

	TArray<uint8> Buffer; 
	Buffer.AddUninitialized(PacketSize);
	
	FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(Buffer.GetData());
	Header->PacketSize = PacketSize;
	Header->PacketId = PacketId;

	Pkt.SerializeToArray(Buffer.GetData() + sizeof(FPacketHeader), DataSize);

	SendPacket(Buffer.GetData(), PacketSize);
}