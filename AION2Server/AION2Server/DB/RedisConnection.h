#pragma once
#include <string>

class RedisConnection
{
public:
	RedisConnection();
	~RedisConnection(){}

	bool ConnectRedis(const std::string& ip, int32 port);
	void Clear();
};