#pragma once
#include <string>
#include <hiredis/hiredis.h>

class RedisConnection
{
public:
	RedisConnection() = default;
	~RedisConnection() { ClearRedis(); }

	bool ConnectRedis(const std::string& ip, int32 port);
	void ClearRedis();

	redisReply* Command(const char* format, ...);

private:
	redisContext* _context = nullptr;
};