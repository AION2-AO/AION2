#include "pch.h"
#include "PacketHandler.h"

PacketHandlerFunc GPacketHandler[256];

bool PacketHandler::Handle_C_SignUp(PacketSessionRef& session, C_SignUpPacket& pkt)
{
	cout << "SignUp Request: ID(" << pkt.id << ") PW(" << pkt.password << ") Class(" << static_cast<int32>(pkt.classType)<< ")" << endl;

	return false;
}

bool PacketHandler::Handle_C_Login(PacketSessionRef& session, C_LoginPacket& pkt)
{
	cout << "Login Request: ID(" << pkt.id << ") PW(" << pkt.password << ")" << endl;


	return true;
}