#include "pch.h"
#include "DediSessionManager.h"
#include "Session/DedicatedSession.h"

DediSessionManager GDediSessionManager;

void DediSessionManager::Add(DediSessionRef session)
{
	std::lock_guard<std::mutex>lock(_dediSessionMngLock);
	_dediSessions.insert(session);
}

void DediSessionManager::Remove(DediSessionRef session)
{
	std::lock_guard<std::mutex>lock(_dediSessionMngLock);
	_dediSessions.erase(session);
}

void DediSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	std::lock_guard<std::mutex>lock(_dediSessionMngLock);
	for (DediSessionRef session : _dediSessions)
	{
		session->Send(sendBuffer);
	}
}

DediSessionRef DediSessionManager::GetFreeDediSession()
{
	std::lock_guard<std::mutex>lock(_dediSessionMngLock);
	for (auto& session : _dediSessions)
	{
		if (!session->IsUsing())
			return session;
	}
		return nullptr;
}
