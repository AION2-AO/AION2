#include "pch.h"
#include "DedicatedSession.h"
#include "DediSessionManager.h"
#include "PacketHandler.h"


void DedicatedSession::OnConnected()
{
	GDediSessionManager.Add(static_pointer_cast<DedicatedSession>(shared_from_this()));
}

void DedicatedSession::OnDisconnected()
{
	GDediSessionManager.Remove(static_pointer_cast<DedicatedSession>(shared_from_this()));

}

void DedicatedSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHandler::HandlePacket(session, buffer, len);
}

void DedicatedSession::OnSend(int32 len)
{
}


void DedicatedSession::SetAddrInfo(const std::string& ip, const int32 port)
{
	_ip = ip;
	_port = port;
}
