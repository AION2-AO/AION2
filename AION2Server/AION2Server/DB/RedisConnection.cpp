#include "pch.h"
#include "DB/RedisConnection.h"

bool RedisConnection::ConnectRedis(const std::string& ip, int32 port)
{
	_context = redisConnect(ip.c_str(), port);
	if (_context == nullptr || _context->err)
	{
		if (_context)
		{
			redisFree(_context);
			_context = nullptr;
		}
		return false;
	}

	return true;
}

void RedisConnection::ClearRedis()
{
	if (_context)
	{
		redisFree(_context);
		_context = nullptr;
	}
}


redisReply* RedisConnection::Command(const char* format, ...)
{
	if(_context == nullptr)
	return nullptr;

	va_list ap;
	va_start(ap, format);
	redisReply* reply = (redisReply*)redisvCommand(_context, format, ap);
	va_end(ap);

	return reply;
}
