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
	PKT_C_SIGNUP = 1000,
	PKT_S_SIGNUP = 1001,

	PKT_C_LOGIN = 1002,
	PKT_S_SLOGIN = 1003,
	PKT_S_FLOGIN = 1004,
	PKT_S_ITEM = 1005,

	PKT_C_MAPLOADCOMPLETE = 1006,
	PKT_S_SPAWN = 1007,

	PKT_C_MOVE = 1008,
	PKT_S_MOVE = 1009,

	PKT_C_DUNGEONWAITINTROOM = 1010,
	PKT_S_DUNGEONWAITINTROOM = 1011,

	PKT_C_DUNGEONCREATE = 1012,
	PKT_S_DUNGEONCREATE = 1013,

	PKT_C_DUNGEONENTER = 1014,
	PKT_S_DUNGEONENTER = 1015,

	PKT_C_DUNGEONREADY = 1016,
	PKT_S_DUNGEONREADY = 1017,

	PKT_C_DUNGEONSTART = 1018,
	PKT_S_DUNGEONSTART = 1019,

	PKT_C_STOREPURCHASE = 1020,
	PKT_S_STOREPURCHASE = 1021,

	PKT_C_CHANGEHP = 1022,

	PKT_DS_DEDICATED = 1100,
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

	FPlayerInfo() {}
	FPlayerInfo(uint64 InId, FString InName, uint8 InClass)
		:PlayerId(InId), PlayerName(InName), ClassType(InClass) 
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
