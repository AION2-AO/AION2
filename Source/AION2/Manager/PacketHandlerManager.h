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

	// 회원가입 및 로그인
	bool Handle_S_SIGNUP(Protocol::S_SignUpResultPacket& Pkt);
	bool Handle_S_SET_NICKNAME(Protocol::S_SetNicknamePacket& Pkt);
	bool Handle_S_LOGIN_SUCCEED(Protocol::S_LoginSuccessPacket& Pkt);
	bool Handle_S_LOGIN_FAIL(Protocol::S_LoginFailPacket& Pkt);

	// 아이템 및 상점
	bool Handle_S_ITEM(Protocol::S_ItemDataPacket& Pkt);
	bool Handle_S_STORE(Protocol::S_StorePurchasePacket& Pkt);
	bool Handle_S_USE_ITEM (Protocol::S_UseItemPacket& Pkt);

	// 맵 이동 및 스폰
	bool Handle_S_SPAWN(Protocol::S_SpawnPacket& Pkt);
	bool Handle_S_MOVE(Protocol::S_MovePacket& Pkt);
	bool Handle_S_DASH(Protocol::S_DashPacket& Pkt);
	bool Handle_S_PVP_STATE(Protocol::S_PvpStatePacket& Pkt);
	bool Handle_S_ATTACK_RESULT(Protocol::S_AttackResultPacket& Pkt);
	bool Handle_S_JUMP(Protocol::S_JumpPacket& Pkt);

	// 던전
	bool Handle_S_ENTER_WAITING(Protocol::S_DungeonWaitingRoomEnterPacket& Pkt);
	bool Handle_S_CREATE(Protocol::S_DungeonCreatePacket& Pkt);
	bool Handle_S_DEDI_CREATE(Protocol::S_DungeonCreatePacket& Pkt); 
	bool Handle_S_ENTER(Protocol::S_DungeonEnterPacket& Pkt);
	bool Handle_S_READY(Protocol::S_DungeonReadyPacket& Pkt);
	bool Handle_S_START(Protocol::S_DungeonStartPacket& Pkt);
	bool Handle_S_EXIT(Protocol::S_DungeonExitPacket& Pkt);
	bool Handle_S_DUNGEON_END(Protocol::S_RequestDungeonCompletePacket& Pkt);
	bool Handle_S_DUNGEON_FAIL(Protocol::S_DungeonFailPacket& Pkt);

	// 캐릭터 상태 및 상호작용
	bool Handle_S_CHAT(Protocol::S_ChatPacket& Pkt);

	// 메일
	bool Handle_S_MAIL_SEND(Protocol::S_MailSendPacket& Pkt);
	bool Handle_S_MAIL_LIST(Protocol::S_MailListPacket& Pkt);
	bool Handle_S_MAIL_CONTENT(Protocol::S_MailContentPacket& Pkt);

	// 연결 종료
	bool Handle_S_DISCONNECT(Protocol::S_DisconnectPacket& Pkt);

	// 데디케이트 서버
	bool Handle_S_DUNGEON_SET_PLAYER(Protocol::S_DungeonStartDediPacket Pkt);

private:
	UAOLoginUserWidget* GetLoginWidget() const;
	class UAOUIManager* GetUIManager() const;
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