#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"

GameSessionManager GSessionManager;
\
void GameSessionManager::Add(GameSessionRef session)
{
	lock_guard<mutex>lock(_sessionMngLock);
	_sessions.insert(session);
}

void GameSessionManager::Remove(GameSessionRef session)
{
	lock_guard<mutex>lock(_sessionMngLock);
	_sessions.erase(session);
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	lock_guard<mutex>lock(_sessionMngLock);
	for (GameSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}

}
