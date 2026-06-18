#pragma once
class GameSession;

using GameSessionRef = shared_ptr<GameSession>;

class GameSessionManager
{
public:
	void Add(GameSessionRef session);
	void Remove(GameSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);

private:
	std::mutex _sessionMngLock;
	set<GameSessionRef> _sessions;
};

extern GameSessionManager GSessionManager;

