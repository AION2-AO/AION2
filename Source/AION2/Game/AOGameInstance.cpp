// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameInstance.h"

#include <Networking.h>
#include <Sockets.h>
#include "Common/TcpSocketBuilder.h"
#include "Serialization/ArrayWriter.h"
#include "SocketSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Network/AONetworkSubsystem.h"
#include "../Common/Protocol.h"

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
						WeakInstPtr->UNetworkManager = WeakInstPtr->GetSubsystem<UAONetworkSubsystem>();
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

	C_SignUpPacket SignUpPacket;
	SignUpPacket.header.packetType = EPacketType::C_SignUp;
	SignUpPacket.header.packetSize = sizeof(C_SignUpPacket);

	FTCHARToUTF8 ConvertId(*Id);
	FCStringAnsi::Strncpy(SignUpPacket.id, (const char*)ConvertId.Get(), sizeof(SignUpPacket.id) - 1);
	SignUpPacket.id[sizeof(SignUpPacket.id) - 1] = '\0';

	FTCHARToUTF8 ConvertPassword(*Password);
	FCStringAnsi::Strncpy(SignUpPacket.password, (const char*)ConvertPassword.Get(), sizeof(SignUpPacket.password) - 1);
	SignUpPacket.password[sizeof(SignUpPacket.password) - 1] = '\0';

	SignUpPacket.classType = static_cast<EClassType>(ClassType);

	SendPacket(&SignUpPacket, sizeof(C_SignUpPacket));
}

void UAOGameInstance::SendLoginPacket(const FString& Id, const FString& Password)
{
	UE_LOG(LogTemp, Display, TEXT("Send Login Packet: %s, %s"), *Id, *Password);
	C_LoginPacket LoginPacket;
	LoginPacket.header.packetType = EPacketType::C_Login;
	LoginPacket.header.packetSize = sizeof(LoginPacket);

	FTCHARToUTF8 ConvertId(*Id);
	FCStringAnsi::Strncpy(LoginPacket.id, (const char*)ConvertId.Get(), sizeof(LoginPacket.id) - 1);
	LoginPacket.id[sizeof(LoginPacket.id) - 1] = '\0';

	FTCHARToUTF8 ConvertPassword(*Password);
	FCStringAnsi::Strncpy(LoginPacket.password, (const char*)ConvertPassword.Get(), sizeof(LoginPacket.password) - 1);
	LoginPacket.password[sizeof(LoginPacket.password) - 1] = '\0';

	SendPacket(&LoginPacket, sizeof(C_LoginPacket));
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