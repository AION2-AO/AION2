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
		GPacketHandler[PKT_C_SET_NICNNAME] = [](PacketSessionRef& session, BYTE* buffer, int len) { return HandlePacket<Protocol::C_SetNicknamePacket>(HandleSetNickname, session, buffer, len); };
		GPacketHandler[PKT_C_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_LoginPacket>(HandleLogin, session, buffer, len); };

		// 아이템 및 상점
		GPacketHandler[PKT_C_STORE_PURCHASE] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_StorePurchasePacket>(HandleStorePurchase, session, buffer, len); };
		GPacketHandler[PKT_C_USE_ITEM] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_UseItemPacket>(HandleUseItem, session, buffer, len); };

		// 맵 이동 및 스폰
		GPacketHandler[PKT_C_MAP_LOAD_COMPLETE] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_MapLoadCompletePacket>(HandleMapComplete, session, buffer, len); };
		GPacketHandler[PKT_C_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_MovePacket>(HandleMove, session, buffer, len); };
		GPacketHandler[PKT_C_DASH] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DashPacket>(HandleDash, session, buffer, len); };
		GPacketHandler[PKT_C_JUMP] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_JumpPacket>(HandleJump, session, buffer, len); };

		// 던전
		GPacketHandler[PKT_C_DUNGEON_ENTER_WAITING_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonWaitingRoomEnterPacket>(HandleDungeonWaitingRoom, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEON_CREATE] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonCreatePacket>(HandleDungeonCreate, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEON_ENTER] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonEnterPacket>(HandleDungeonEnter, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEON_READY] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonReadyPacket>(HandleDungeonReady, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEON_START] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonStartPacket>(HandleDungeonStart, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEON_EXIT] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DungeonExitPacket>(HandleDungeonExit, session, buffer, len); };
		GPacketHandler[PKT_C_DUNGEON_COMPLETE_REQUEST] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_RequestDungeonCompletePacket>(HandleDungeonEnd, session, buffer, len); };

		// 캐릭터 상태 및 상호작용
		GPacketHandler[PKT_C_CHANGE_HP] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_ChangeHpPacket>(HandleChangeHp, session, buffer, len); };
		GPacketHandler[PKT_C_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_ChatPacket>(HandleChat, session, buffer, len); };
		GPacketHandler[PKT_C_ATTACK] = [](PacketSessionRef& session, BYTE* buffer, int len) { return HandlePacket<Protocol::C_AttackPacket>(HandleAttack, session, buffer, len); };

		// 메일
		GPacketHandler[PKT_C_MAIL_SEND] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_SendMailPacket>(HandleMailSend, session, buffer, len); };
		GPacketHandler[PKT_C_MAIL_LIST] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_MailListPacket>(HandleMailList, session, buffer, len); };
		GPacketHandler[PKT_C_MAIL_CONTENT] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_MailContentPacket>(HandleMailContent, session, buffer, len); };


		// 기타 (데디케이트 서버)
		GPacketHandler[PKT_DS_DEDICATED] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_DedicatedPacket>(HandleDedicated, session, buffer, len); };
	}

	static bool HandleSignUp(PacketSessionRef& session, Protocol::C_SignUpPacket& pkt);
	static bool HandleSetNickname(PacketSessionRef& session, Protocol::C_SetNicknamePacket& pkt);
	static bool HandleLogin(PacketSessionRef& session, Protocol::C_LoginPacket& pkt);
	static bool HandleMapComplete(PacketSessionRef& session, Protocol::C_MapLoadCompletePacket& pkt);
	static bool HandleMove(PacketSessionRef& session, Protocol::C_MovePacket& pkt);
	static bool HandleDash(PacketSessionRef& session, Protocol::C_DashPacket& pkt);
	static bool HandleJump(PacketSessionRef& session, Protocol::C_JumpPacket& pkt);
	static bool HandleAttack(PacketSessionRef& session, Protocol::C_AttackPacket& pkt);

	static bool HandleChangeHp(PacketSessionRef& session, Protocol::C_ChangeHpPacket& pkt);

	static bool HandleDedicated(PacketSessionRef& session, Protocol::C_DedicatedPacket& pkt);
	static bool HandleDungeonWaitingRoom(PacketSessionRef& session, Protocol::C_DungeonWaitingRoomEnterPacket& pkt);
	static bool HandleDungeonCreate(PacketSessionRef& session, Protocol::C_DungeonCreatePacket& pkt);
	static bool HandleDungeonEnter(PacketSessionRef& session, Protocol::C_DungeonEnterPacket& pkt);
	static bool HandleDungeonExit(PacketSessionRef& session, Protocol::C_DungeonExitPacket& pkt);
	static bool HandleDungeonReady(PacketSessionRef& session, Protocol::C_DungeonReadyPacket& pkt);
	static bool HandleDungeonStart(PacketSessionRef& session, Protocol::C_DungeonStartPacket& pkt);
	static bool HandleDungeonEnd(PacketSessionRef& session, Protocol::C_RequestDungeonCompletePacket& pkt);


	static bool HandleStorePurchase(PacketSessionRef& session, Protocol::C_StorePurchasePacket& pkt);
	static bool HandleUseItem(PacketSessionRef& session, Protocol::C_UseItemPacket& pkt);
	static bool HandleChat(PacketSessionRef& session, Protocol::C_ChatPacket& pkt);


	// 메일
	static bool HandleMailSend(PacketSessionRef& session, Protocol::C_SendMailPacket& pkt);
	static bool HandleMailList(PacketSessionRef& session, Protocol::C_MailListPacket& pkt);
	static bool HandleMailContent(PacketSessionRef& session, Protocol::C_MailContentPacket& pkt);

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
	static SendBufferRef MakeSendBuffer(Protocol::S_SetNicknamePacket& pkt) { return MakeSendBuffer(pkt, PKT_S_SET_NICNNAME); };
	static SendBufferRef MakeSendBuffer(Protocol::S_LoginSuccessPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_LOGIN_SUCCEED); };
	static SendBufferRef MakeSendBuffer(Protocol::S_LoginFailPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_LOGIN_FAIL); };

	static SendBufferRef MakeSendBuffer(Protocol::S_ItemDataPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_ITEM); };
	static SendBufferRef MakeSendBuffer(Protocol::S_SpawnPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_SPAWN); };
	static SendBufferRef MakeSendBuffer(Protocol::S_MovePacket& pkt) { return MakeSendBuffer(pkt, PKT_S_MOVE); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DashPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DASH); };
	static SendBufferRef MakeSendBuffer(Protocol::S_JumpPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_JUMP); };
	static SendBufferRef MakeSendBuffer(Protocol::S_AttackResultPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_ATTACK_RESULT); };
	static SendBufferRef MakeSendBuffer(Protocol::S_PvpStatePacket& pkt) { return MakeSendBuffer(pkt, PKT_S_PVP_STATE); };

	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonWaitingRoomEnterPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEON_ENTER_WAITING_ROOM); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonCreatePacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEON_CREATE); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonEnterPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEON_ENTER); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonStartPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEON_START); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonExitPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEON_EXIT); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonReadyPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEON_READY); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonFailPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEON_FAIL); };
	static SendBufferRef MakeSendBuffer(Protocol::S_SetDungeonPlayerPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEON_SET_PLAYER); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DungeonStartDediPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEON_DEDI_START); };
	static SendBufferRef MakeSendBuffer(Protocol::S_RequestDungeonCompletePacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DUNGEON_COMPLETE_REQUEST); };

	static SendBufferRef MakeSendBuffer(Protocol::S_StorePurchasePacket& pkt) { return MakeSendBuffer(pkt, PKT_S_STORE_PURCHASE); };
	static SendBufferRef MakeSendBuffer(Protocol::S_UseItemPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_USE_ITEM); };

	static SendBufferRef MakeSendBuffer(Protocol::S_ChatPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_CHAT); };
	static SendBufferRef MakeSendBuffer(Protocol::S_DisconnectPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_DISCONNECT); };

	static SendBufferRef MakeSendBuffer(Protocol::S_MailListPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_MAIL_LIST); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MailContentPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_MAIL_CONTENT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MailSendPacket& pkt) { return MakeSendBuffer(pkt, PKT_S_MAIL_SEND); }

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
