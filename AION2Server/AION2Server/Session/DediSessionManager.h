#pragma once
#include "DedicatedSession.h"

using DediSessionRef = std::shared_ptr<DedicatedSession>;

class DediSessionManager
{
public:
	void Add(DediSessionRef session);
	void Remove(DediSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);

	DediSessionRef GetFreeDediSession();

private:
	std::mutex _dediSessionMngLock;
	std::set<DediSessionRef> _dediSessions;
};

extern DediSessionManager GDediSessionManager;
