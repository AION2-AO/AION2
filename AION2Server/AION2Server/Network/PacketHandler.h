#pragma once
#include "SendBuffer.h"
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

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


};

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);

class PacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 1000; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;

		GPacketHandler[PKT_C_SIGNUP] = [](PacketSessionRef& session, BYTE* buffer, int len) { return HandlePacket<Protocol::C_SignUpPacket>(HandleSignUp, session, buffer, len); };
		GPacketHandler[PKT_C_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_LoginPacket>(HandleLogin, session, buffer, len); };
		GPacketHandler[PKT_C_MAPLOADCOMPLETE] = [](PacketSessionRef& session, BYTE* buffer, int len) {return HandlePacket<Protocol::C_MapLoadCompletePacket>(HandleMapComplete, session, buffer, len); };
	}

	static bool HandleSignUp(PacketSessionRef& session, Protocol::C_SignUpPacket& pkt);
	static bool HandleLogin(PacketSessionRef& session, Protocol::C_LoginPacket& pkt);
	static bool HandleMapComplete(PacketSessionRef& session, Protocol::C_MapLoadCompletePacket& pkt);

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
