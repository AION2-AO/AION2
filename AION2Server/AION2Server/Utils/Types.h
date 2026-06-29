#pragma once

#include <mutex>
#include <atomic>
#include <assert.h>

using BYTE = unsigned char;
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

#define OUT
#define ASSERT_CRASH(expr) assert(expr)

#define USING_SHARED_PTR(name)  using name##Ref = std::shared_ptr<class name>;

USING_SHARED_PTR(IocpCore);
USING_SHARED_PTR(IocpObject);
USING_SHARED_PTR(Listener);
USING_SHARED_PTR(Service);
USING_SHARED_PTR(MMOServerService);
USING_SHARED_PTR(DediService);

USING_SHARED_PTR(Session);
USING_SHARED_PTR(PacketSession);
USING_SHARED_PTR(GameSession);
USING_SHARED_PTR(DedicatedSession);
USING_SHARED_PTR(SendBuffer);
USING_SHARED_PTR(Player);

USING_SHARED_PTR(JobQueue);
USING_SHARED_PTR(Job);

USING_SHARED_PTR(Room);
USING_SHARED_PTR(Dungeon);
USING_SHARED_PTR(DungeonWaitingRoom);


const uint64 WORKER_TICK = 64;

#define size16(val)             static_cast<int16>(sizeof(val))
#define size32(val)             static_cast<int32>(sizeof(val))
#define len16(arr)              static_cast<int16>(sizeof(arr)/sizeof(arr[0]))
#define len32(arr)              static_cast<int32>(sizeof(arr)/sizeof(arr[0]))
