#pragma once
#include "SendBuffer.h"
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	// 회원가입 및 로그인
	PKT_C_SIGNUP = 1000,
	PKT_S_SIGNUP = 1001,
	PKT_C_LOGIN = 1002,
	PKT_S_SLOGIN = 1003,
	PKT_S_FLOGIN = 1004,

	// 아이템 및 상점
	PKT_S_ITEM = 1005,
	PKT_C_USEITEM = 1006,
	PKT_S_USEITEM = 1007,
	PKT_C_STOREPURCHASE = 1008,
	PKT_S_STOREPURCHASE = 1009,

	// 맵 이동 및 스폰
	PKT_C_MAPLOADCOMPLETE = 1010,
	PKT_S_SPAWN = 1011,
	PKT_C_MOVE = 1012,
	PKT_S_MOVE = 1013,

	// 던전
	PKT_C_DUNGEONWAITINTROOM = 1014,
	PKT_S_DUNGEONWAITINTROOM = 1015,
	PKT_C_DUNGEONCREATE = 1016,
	PKT_S_DUNGEONCREATE = 1017,
	PKT_C_DUNGEONENTER = 1018,
	PKT_S_DUNGEONENTER = 1019,
	PKT_C_DUNGEONEXIT = 1032,
	PKT_S_DUNGEONEXIT = 1033,
	PKT_C_DUNGEONREADY = 1020,
	PKT_S_DUNGEONREADY = 1021,
	PKT_C_DUNGEONSTART = 1022,
	PKT_S_DUNGEONSTART = 1023,
	PKT_S_DUNGEONFAIL = 1028,
	PKT_C_DUNGEOMMAPCOMPLETE = 1029,
	PKT_S_DUNGEONSETPLAYER = 1030,
	PKT_S_DUNGEONDEDISTART = 1031,

	// 캐릭터 상태 및 상호작용
	PKT_C_CHANGEHP = 1024,
	PKT_C_CHAT = 1025,
	PKT_S_CHAT = 1026,

	// 연결 종료
	PKT_S_DISCONNECT = 1027,

	PKT_DS_DEDICATED = 1100,
};

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);

class PacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 1000; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;

		// 회원가입 및 로그인
		GPacketHandler[PKT_C_SIGNUP] = [](PacketSessionRef& session, BYTE* buffer, int len) { return HandlePacket<Protocol::C_SignUpPacket>(HandleSignUp, session, buffer, len); };
		GPacketHandler[PKT_C_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_LoginPacket>(HandleLogin, session, buffer, len); };

		// 아이템 및 상점
		GPacketHandler[PKT_C_STOREPURCHASE] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_StorePurchasePacket>(HandleStorePurchase, session, buffer, len); };
		GPacketHandler[PKT_C_USEITEM] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_UseItemPacket>(HandleUseItem, session, buffer, len); };

		// 맵 이동 및 스폰
		GPacketHandler[PKT_C_MAPLOADCOMPLETE] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_MapLoadCompletePacket>(HandleMapComplete, session, buffer, len); };
		GPacketHandler[PKT_C_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_MovePacket>(HandleMove, session, buffer, len); };

		// 던전
		GPacketHandler[PKT_C_DUNGEONWAITINTROOM] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonWaitingRoomEnterPacket>(HandleDungeonWaitingRoom, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEONCREATE] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonCreatePacket>(HandleDungeonCreate, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEONENTER] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonEnterPacket>(HandleDungeonEnter, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEONREADY] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonReadyPacket>(HandleDungeonReady, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEONSTART] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonStartPacket>(HandleDungeonStart, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEONEXIT] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonExitPacket>(HandleDungeonExit, session, buffer, len); };

		// 캐릭터 상태 및 상호작용
		GPacketHandler[PKT_C_CHANGEHP] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_ChangeHpPacket>(HandleChangeHp, session, buffer, len); };
		GPacketHandler[PKT_C_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_ChatPacket>(HandleChat, session, buffer, len); };

		// 기타 (데디케이트 서버)
		GPacketHandler[PKT_DS_DEDICATED] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DedicatedPacket>(HandleDedicated, session, buffer, len); };
	}

	static bool HandleSignUp(PacketSessionRef& session, Protocol::C_SignUpPacket& pkt);
	static bool HandleLogin(PacketSessionRef& session, Protocol::C_LoginPacket& pkt);
	static bool HandleMapComplete(PacketSessionRef& session, Protocol::C_MapLoadCompletePacket& pkt);
	static bool HandleMove(PacketSessionRef& session, Protocol::C_MovePacket& pkt);
	static bool HandleChangeHp(PacketSessionRef& session, Protocol::C_ChangeHpPacket& pkt);

	static bool HandleDedicated(PacketSessionRef& session, Protocol::C_DedicatedPacket& pkt);
	static bool HandleDungeonWaitingRoom(PacketSessionRef& session, Protocol::C_DungeonWaitingRoomEnterPacket& pkt);
	static bool HandleDungeonCreate(PacketSessionRef& session, Protocol::C_DungeonCreatePacket& pkt);
	static bool HandleDungeonEnter(PacketSessionRef& session, Protocol::C_DungeonEnterPacket& pkt);
	static bool HandleDungeonExit(PacketSessionRef& session, Protocol::C_DungeonExitPacket& pkt);
	static bool HandleDungeonReady(PacketSessionRef& session, Protocol::C_DungeonReadyPacket& pkt);
	static bool HandleDungeonStart(PacketSessionRef& session, Protocol::C_DungeonStartPacket& pkt);

	static bool HandleStorePurchase(PacketSessionRef& session, Protocol::C_StorePurchasePacket& pkt);
	static bool HandleUseItem(PacketSessionRef& session, Protocol::C_UseItemPacket& pkt);
	static bool HandleChat(PacketSessionRef& session, Protocol::C_ChatPacket& pkt);


	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[static_cast<uint16>(header->id)](session, buffer, len);
	}

	//template<typename PacketTypeStruct, typename ProcessFunc>
	//static void Register(EPacketType id, ProcessFunc func)
	//{
	//	GPacketHandler[static_cast<uint8>(id)] = [func](PacketSessionRef& session, BYTE* buffer, int32 len)
	//		{
	//			PacketTypeStruct pkt;
	//			if (pkt.ParseFromArray(&buffer[sizeof(PacketHeader)], len - sizeof(PacketHeader)) == false)
	//				return false;
	//			return func(session, pkt);
	//		};
	//}

	static SendBufferRef MakeSendBuffer(Protocol::S_SignUpResultPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_SIGNUP); };
	static SendBufferRef MakeSendBuffer(Protocol::S_LoginSuccessPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_SLOGIN); };
	static SendBufferRef MakeSendBuffer(Protocol::S_LoginFailPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_FLOGIN); };
	static SendBufferRef MakeSendBuffer(Protocol::S_ItemDataPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_ITEM); };
	static SendBufferRef MakeSendBuffer(Protocol::S_SpawnPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_SPAWN); };
	static SendBufferRef MakeSendBuffer(Protocol::S_MovePacket& pkt) { return MakeSendBuffer(pkt, PKT_S_MOVE); };

	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonWaitingRoomEnterPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEONWAITINTROOM); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonCreatePacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEONCREATE); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonEnterPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEONENTER); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonStartPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEONSTART); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonExitPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEONEXIT); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonReadyPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEONREADY); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonFailPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEONFAIL); };

	static SendBufferRef MakeSendBuffer(Protocol::S_StorePurchasePacket& pkt) { return MakeSendBuffer(pkt, PKT_S_STOREPURCHASE); };
	static SendBufferRef MakeSendBuffer(Protocol::S_UseItemPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_USEITEM); };

	static SendBufferRef MakeSendBuffer(Protocol::S_ChatPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_CHAT); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DisconnectPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DISCONNECT); };

	static SendBufferRef MakeSendBuffer(Protocol::S_SetDungeonPlayerPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEONSETPLAYER); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonStartDediPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEONDEDISTART); };

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& packet, uint16 packetId)
	{
		const uint16 dataSize = static_cast<uint16>(packet.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = std::make_shared<SendBuffer>(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());

		header->size = packetSize;
		header->id = packetId;

		char* bufferPtr = reinterpret_cast<char*>(header);
		packet.SerializeToArray(bufferPtr + sizeof(PacketHeader), dataSize);
		//	ASSERT_CRASH(packet.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};
