// AOPacketHandler.h
#pragma once

#include "CoreMinimal.h"
#include "Network/PacketHeader.h"

class UAONetworkManager;
class UAOGameInstance;
class UAOPlayerManager;
class UAOLoginUserWidget;

class FPacketHandler
{
public:
	FPacketHandler(UAONetworkManager* InMng);

	bool Handle_S_SIGNUP(Protocol::S_SignUpResultPacket& Pkt);
	bool Handle_S_SLOGIN(Protocol::S_LoginSuccessPacket& Pkt);
	bool Handle_S_FLOGIN(Protocol::S_LoginFailPacket& Pkt);
	bool Handle_S_ITEM(Protocol::S_ItemDataPacket& Pkt);
	bool Handle_S_SPAWN(Protocol::S_SpawnPacket& Pkt);
	bool Handle_S_MOVE(Protocol::S_MovePacket& Pkt);
	bool Handle_S_CREATE(Protocol::S_DungeonCreatePacket& Pkt);
	bool Handle_S_ENTER(Protocol::S_DungeonEnterPacket& Pkt);
	bool Handle_S_READY(Protocol::S_DungeonReadyPacket& Pkt);
	bool Handle_S_START(Protocol::S_DungeonStartPacket& Pkt);
	bool Handle_S_CHAT(Protocol::S_ChatPacket& Pkt);
	bool Handle_S_STORE(Protocol::S_StorePurchasePacket& Pkt);

private:
	UAOLoginUserWidget* GetLoginWidget() const;

private:
	UAONetworkManager* NetworkMng;
	UAOGameInstance* GameInstance;
	UAOPlayerManager* PlayerMng;
};

// 함수 포인터 타입
typedef bool (*PacketHandlerFunc)(UAONetworkManager*, uint8*, int32);
extern PacketHandlerFunc GAOPacketHandler[UINT16_MAX];

void InitPacketHandler();

bool Handle_INVALID(UAONetworkManager* NetworkMng, uint8* Buffer, int32 Len);

// 패킷 변환 및 파싱 템플릿 함수
template<typename T, typename MemberFunc>
bool HandlePacketPolicy(MemberFunc Handler, UAONetworkManager* NetworkMng, uint8* Buffer, int32 Len)
{
	T Pkt;
	// 버퍼에서 패킷 파싱
	if (Pkt.ParseFromArray(Buffer, Len) == false)
		return false;

	// 핸들러 호출
	FPacketHandler Helper(NetworkMng);
	return (Helper.*Handler)(Pkt);
}