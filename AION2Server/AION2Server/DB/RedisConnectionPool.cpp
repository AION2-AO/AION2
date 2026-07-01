#include "pch.h"
#include "RedisConnection.h"
#include "RedisConnectionPool.h"

bool RedisConnectionPool::Connect(int32 connectionCount, const std::string& ip, int32 port)
{
	std::lock_guard<std::mutex>lock(_redisLock);
	for (int32 i = 0; i < connectionCount; ++i)
	{
		RedisConnection* conn = new RedisConnection();
		if (conn->ConnectRedis(ip, port) == false)
		{
			delete conn;
			conn = nullptr;
			return false;
		}
		_connections.push_back(conn);
	}
	return true;
}

void RedisConnectionPool::Clear()
{
	std::lock_guard<std::mutex>lock(_redisLock);
	for (auto* conn : _connections)
	{
		delete conn;
	}
	_connections.clear();
}

RedisConnection* RedisConnectionPool::Pop()
{
	std::lock_guard<std::mutex>lock(_redisLock);
	if (_connections.empty()) return nullptr;
	RedisConnection* conn = _connections.back();
	_connections.pop_back();
	return conn;
}

void RedisConnectionPool::Push(RedisConnection* connection)
{
	std::lock_guard<std::mutex> lock(_redisLock);
	if (connection)
	{
		_connections.push_back(connection);
	}
}
