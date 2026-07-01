#include "pch.h"
#include "RedisManager.h"
#include "RedisConnection.h"
#include <hiredis/hiredis.h>

RedisManager GRedisManager;

bool RedisManager::Init(int32 connectionCount, const std::string& ip, int32 port)
{
	return _pool->Connect(connectionCount, ip, port);
}

void RedisManager::UpdatePlayerHp(const std::string& playerId, int32 hp)
{
	RedisConnection* conn = _pool->Pop();
	if (conn == nullptr)return;

	redisReply* reply = conn->Command("HSET player:state:hp %s %d", playerId.c_str(), hp);
	if (reply) freeReplyObject(reply);

	_pool->Push(conn);
}

std::unordered_map<std::string, int32> RedisManager::GetPendingHpUpdate()
{
	std::unordered_map<std::string, int32> result;
	RedisConnection* conn = _pool->Pop();
	if (conn == nullptr) return result;

	redisReply* reply = conn->Command("HGETALL player:state:hp");
	if (reply && reply->type == REDIS_REPLY_ARRAY)
	{
		for (size_t i = 0; i < reply->elements; i += 2)
		{
			std::string id = reply->element[i]->str;
			int32 hp = std::stoi(reply->element[i + 1]->str);
			result[id] = hp;
		}
	}
	if (reply) freeReplyObject(reply);
	_pool->Push(conn);

	return result;
}

void RedisManager::ClearPendingHpUpdate(const std::vector<std::string>& ids)
{
	if (ids.empty()) return;

	RedisConnection* conn = _pool->Pop();
	if (conn == nullptr) return;

	std::string cmd = "HDEL player:state:hp";
	for (const auto& id : ids)
	{
		cmd += " " + id;
	}

	redisReply* reply = conn->Command(cmd.c_str());
	if (reply) freeReplyObject(reply);

	_pool->Push(conn);
}
