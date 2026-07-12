// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Network/PacketHeader.h"
#include "AOGameInstance.generated.h"

/**
 *
 */

class UAOUIManager;
class UAONetworkManager;

UCLASS()
class AION2_API UAOGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	void TryAsyncConnect(const FString& Ip, int32 Port);
	bool ConnectToServer(const FString& Ip, int32 Port);
	bool IsServerConnected();

	FString  GetLocalIPAddress();
	int32 GetLocalPort();
	//void SendDediIpPort(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SendDediIpPort();

	UFUNCTION(BlueprintCallable)
	void SendSignUpPacket(const FString& Id, const FString& Password, int32 ClassType);

	UFUNCTION(BlueprintCallable)
	void SendLoginPacket(const FString& Id, const FString& Password);

	UFUNCTION(BlueprintCallable)
	void SendMapLoadCompletePacket();

public:
	UFUNCTION(BlueprintCallable)
	void OpenVillageLevel();

	void OnReadyoOpenLevel();

	void SendPacket(void* Packet, int32 PacketSize);
	void SendPacket(google::protobuf::Message& Pkt, uint16 PacketId);

	class UAONetworkManager* GetNetworkManager() { return UNetworkManager; }
	UAOUIManager* GetUIManager() const { return UIManager; }

public:
	void SetMyPlayerId(uint64 playerId) { MyPlayerId = playerId; }
	uint64 GetMyPlayerId() const { return MyPlayerId; }
	
	void SetMyPlayerClass(uint8 ClassType) { MyPlayerClass = ClassType; }
	int32 GetMyPlayerClass() { return MyPlayerClass; }

public:
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<class UAOLoginUserWidget> LoginWidget;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<class UAOLoginUserWidget> RegisterWidget;

	FSocket* ClientSocket;
	class UAONetworkManager* UNetworkManager;

private:
	uint64 MyPlayerId = 0;
	uint8 MyPlayerClass = 0;

	FString MyPlayerName;

	UPROPERTY()
	TObjectPtr<UAOUIManager> UIManager;
};
