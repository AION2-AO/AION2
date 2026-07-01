#pragma once
#include <mutex>
#include <vector>

class RedisConnection;
class RedisConnectionPool
{
public:
	RedisConnectionPool() = default;
	~RedisConnectionPool() {}

	bool							Connect(int32 connectionCount, const std::string& ip, int32 port);
	void							Clear();
	RedisConnection*				Pop();
	void							Push(RedisConnection* connection);

private:
	std::mutex						_redisLock;
	std::vector<RedisConnection*>	_connections;
};

