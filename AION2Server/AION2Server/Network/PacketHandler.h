#pragma once

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[256];

class PacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < 256; i++)
			GPacketHandler[i] = [](PacketSessionRef& s, BYTE* b, int32 l) { return false; };

		Register<C_LoginPacket>(EPacketType::C_Login, Handle_C_Login);
		Register<C_SignUpPacket>(EPacketType::C_SignUp, Handle_C_SignUp);
	}

	static bool Handle_C_SignUp(PacketSessionRef& session, C_SignUpPacket& pkt);
	static bool Handle_C_Login(PacketSessionRef& session, C_LoginPacket& pkt);

	template<typename PacketTypeStruct, typename ProcessFunc>
	static void Register(EPacketType id, ProcessFunc func)
	{
		GPacketHandler[static_cast<uint8>(id)] = [func](PacketSessionRef& session, BYTE* buffer, int32 len)
		{
			return HandlePacket<PacketTypeStruct>(func, session, buffer, len);
		};
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[static_cast<uint8>(header->packetType)](session, buffer, len);
	}

private:
	template<typename PacketTypeStruct, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc Func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		if (len < sizeof(PacketTypeStruct))
			return false;

		PacketTypeStruct* packet = reinterpret_cast<PacketTypeStruct*>(buffer);
		return Func(session, *packet);
	}

public:
	template<typename T>
	static SendBufferRef MakeSendBuffer(T& packet, EPacketType packetId)
	{
		const uint16 packetSize = sizeof(T);
		SendBufferRef sendBuffer = make_shared<SendBuffer>(packetSize);

		packet.header.packetSize = packetSize;
		packet.header.packetType = packetId;

		sendBuffer->CopyData(&packet, packetSize);
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};
