#pragma once
#include <unordered_map>
#include "RedisConnectionPool.h"

class RedisManager
{
public:
	RedisManager() : _pool(new RedisConnectionPool()) {}
	~RedisManager() { delete _pool; }

	bool Init(int32 connectionCount, const std::string& ip, int32 port);

	void UpdatePlayerHp(const std::string& playerId, int32 hp);

	std::unordered_map<std::string, int32>GetPendingHpUpdate();
	void ClearPendingHpUpdate(const std::vector<std::string>& ids);

private:
	RedisConnectionPool* _pool;
};
extern RedisManager GRedisManager;
