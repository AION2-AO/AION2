#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"
#include "DediSessionManager.h"
#include "Player.h"

GameSessionManager GSessionManager;

void GameSessionManager::Add(GameSessionRef session)
{
	std::lock_guard<std::mutex>lock(_sessionMngLock);
	_sessions.insert(session);
}

void GameSessionManager::Remove(GameSessionRef session)
{
	std::lock_guard<std::mutex>lock(_sessionMngLock);
	_sessions.erase(session);
}


void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	std::lock_guard<std::mutex>lock(_sessionMngLock);
	for (GameSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}

}

GameSessionRef GameSessionManager::FindById(uint64 id)
{
	std::lock_guard<std::mutex>lock(_sessionMngLock);
	for (GameSessionRef session : _sessions)
	{
		if (session->_player && session->_player->GetId() == id)
			return session;
	}
	return nullptr;
}
