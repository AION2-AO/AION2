#pragma once
#include "CoreMinimal.h"

#pragma warning(push)
#pragma warning(disable: 4668) 
#pragma warning(disable: 4458) 
#pragma warning(disable: 4800)

#pragma push_macro("check")
#undef check
#pragma push_macro("verify")
#undef verify

#ifndef PROTOBUF_USE_DLLS
#define PROTOBUF_USE_DLLS 0
#endif

#include "Network/Protocol.pb.h"
#include "Network/Struct.pb.h"
#include "Network/Enum.pb.h"

#pragma pop_macro("verify")
#pragma pop_macro("check")

#pragma warning(pop)

enum : uint16
{
	// 회원가입 및 로그인
	PKT_C_SIGNUP = 1000,
	PKT_S_SIGNUP = 1001,
	PKT_C_SET_NICNNAME = 1046,
	PKT_S_SET_NICNNAME = 1047,

	PKT_C_LOGIN = 1002,
	PKT_S_LOGIN_SUCCEED = 1003,
	PKT_S_LOGIN_FAIL = 1004,

	// 아이템 및 상점
	PKT_S_ITEM = 1005,
	PKT_C_USE_ITEM = 1006,
	PKT_S_USE_ITEM = 1007,
	PKT_C_STORE_PURCHASE = 1008,
	PKT_S_STORE_PURCHASE = 1009,

	// 맵 이동 및 스폰
	PKT_C_MAP_LOAD_COMPLETE = 1010,
	PKT_S_SPAWN = 1011,
	PKT_C_MOVE = 1012,
	PKT_S_MOVE = 1013,
	PKT_C_DASH = 1044,
	PKT_S_DASH = 1045,

	// 던전
	PKT_C_DUNGEON_ENTER_WAITING_ROOM = 1014,
	PKT_S_DUNGEON_ENTER_WAITING_ROOM = 1015,
	PKT_C_DUNGEON_CREATE = 1016,
	PKT_S_DUNGEON_CREATE = 1017,
	PKT_C_DUNGEON_ENTER = 1018,
	PKT_S_DUNGEON_ENTER = 1019,
	PKT_C_DUNGEON_EXIT = 1032,
	PKT_S_DUNGEON_EXIT = 1033,
	PKT_C_DUNGEON_READY = 1020,
	PKT_S_DUNGEON_READY = 1021,
	PKT_C_DUNGEON_START = 1022,
	PKT_S_DUNGEON_START = 1023,
	PKT_S_DUNGEON_FAIL = 1028,
	PKT_C_DUNGEON_MAP_COMPLETE = 1029,
	PKT_S_DUNGEON_SET_PLAYER = 1030,
	PKT_S_DUNGEON_DEDI_START = 1031,

	// 던전 종료
	PKT_C_DUNGEON_COMPLETE_REQUEST = 1040,
	PKT_S_DUNGEON_COMPLETE_REQUEST = 1041,

	// 캐릭터 상태 및 상호작용
	PKT_C_CHANGE_HP = 1024,
	PKT_C_CHAT = 1025,
	PKT_S_CHAT = 1026,

	// 메일
	PKT_C_MAIL_LIST = 1034,
	PKT_S_MAIL_LIST = 1035,
	PKT_C_MAIL_CONTENT = 1036,
	PKT_S_MAIL_CONTENT = 1037,
	PKT_C_MAIL_SEND = 1038,
	PKT_S_MAIL_SEND = 1039,

	// 연결 종료
	PKT_S_DISCONNECT = 1027,

	PKT_DS_DEDICATED = 1100,

	// PVP 및 공격 관련 패킷
	PKT_S_PVP_STATE = 1048,
	PKT_C_ATTACK = 1049,
	PKT_S_ATTACK_RESULT = 1050,
	PKT_C_JUMP = 1051,
	PKT_S_JUMP = 1052,
};

#include "PacketHeader.generated.h"

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY()
	uint64 PlayerId = 0;

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	uint8 ClassType = 0;

	UPROPERTY()
	float PlayerHp = 0;

	FPlayerInfo() {}
	FPlayerInfo(uint64 InId, FString InName, uint8 InClass, float InHp)
		:PlayerId(InId), PlayerName(InName), ClassType(InClass), PlayerHp(InHp)
	{}
};

struct FPacket
{
	uint16 PacketId = 0;
	TArray<uint8> RawPayload;
};

#pragma pack(push, 1)
struct FPacketHeader
{
	uint16 PacketSize;
	uint16 PacketId;
}; 
#pragma pack(pop)
