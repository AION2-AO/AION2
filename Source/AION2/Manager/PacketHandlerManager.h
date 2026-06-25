// AOPacketHandler.h
#pragma once

#include "CoreMinimal.h"
#include "Network/PacketHeader.h"

class UAONetworkManager;

// 함수 포인터 타입 정의함
typedef bool (*PacketHandlerFunc)(UAONetworkManager*, uint8*, int32);
extern PacketHandlerFunc GAOPacketHandler[UINT16_MAX];

void InitPacketHandler();

// 패킷별 실제 처리 함수 전방 선언함
bool Handle_INVALID(UAONetworkManager* NetworkMng, uint8* Buffer, int32 Len);
bool Handle_S_SIGNUP(UAONetworkManager* NetworkMng, Protocol::S_SignUpResultPacket& Pkt);
bool Handle_S_SLOGIN(UAONetworkManager* NetworkMng, Protocol::S_LoginSuccessPacket& Pkt);
bool Handle_S_FLOGIN(UAONetworkManager* NetworkMng, Protocol::S_LoginFailPacket& Pkt);
bool Handle_S_ITEM(UAONetworkManager* NetworkMng, Protocol::S_ItemDataPacket& Pkt);
bool Handle_S_SPAWN(UAONetworkManager* NetworkMng, Protocol::S_SpawnPacket& Pkt);
bool Handle_S_MOVE(UAONetworkManager* NetworkMng, Protocol::S_MovePacket& Pkt);

// 패킷 변환 및 파싱을 대행해줄 템플릿 헬퍼 함수임
template<typename T, typename HandlerFunc>
bool HandlePacketPolicy(HandlerFunc Handler, UAONetworkManager* NetworkMng, uint8* Buffer, int32 Len)
{
	T Pkt;
	// 메인 스레드 스택 메모리 안에서 파싱 수행함
	if (Pkt.ParseFromArray(Buffer, Len) == false)
		return false;

	// 성공 시 실제 컨텐츠 로직 함수 호출함
	return Handler(NetworkMng, Pkt);
}