#pragma once
class GameSession;

using GameSessionRef = std::shared_ptr<GameSession>;

class GameSessionManager
{
public:
	void Add(GameSessionRef session);
	void Remove(GameSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);
	GameSessionRef FindById(uint64 id);

private:
	std::mutex _sessionMngLock;
	std::set<GameSessionRef> _sessions;
};

extern GameSessionManager GSessionManager;

