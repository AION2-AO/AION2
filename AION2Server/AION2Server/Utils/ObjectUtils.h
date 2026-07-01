#pragma once
class ObjectUtils
{
public:
	static PlayerRef CreatePlayer(GameSessionRef session);

private:
	static std::atomic<uint32> _idGenerator;
};

