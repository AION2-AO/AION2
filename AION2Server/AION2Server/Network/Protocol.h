#pragma once

#pragma pack(push, 1)
struct Vector3
{
	float x;
	float y;
	float z;
};

enum class EPacketType : uint16
{
	C_Login = 1,
	S_LoginResult = 2,
	C_SignUp = 3,
	S_SignUpResult = 4,
};

struct PacketHeader
{
	uint16 packetSize;
	EPacketType packetType;
};

struct C_LoginPacket
{
	PacketHeader header;
	char id[20];
	char password[20];
};

struct S_LoginSuccese
{
	PacketHeader header;
	uint16 playerId;
};

struct S_LoginFail
{
	PacketHeader header;
};

#pragma pack(pop)
