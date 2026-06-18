#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "JobQueue.h"
#include "JobTimer.h"
#include "SocketUtils.h"
#include "GlobalQueue.h"
#include "../DB/DBConnectionPool.h"

ThreadManager* GThreadManager = nullptr;
GlobalQueue* GGlobalQueue = nullptr;
JobTimer* GJobTimer = nullptr;
DBConnectionPool* GDBConnectionPool = nullptr;

class CoreGlobal
{
public:
        CoreGlobal()
        {
                GThreadManager = new ThreadManager();
                GGlobalQueue = new GlobalQueue();
                GJobTimer = new JobTimer();
                GDBConnectionPool = new DBConnectionPool();
                SocketUtils::Init();
        }

        ~CoreGlobal()
        {
                delete GThreadManager;
                delete GGlobalQueue;
                delete GJobTimer;
                delete GDBConnectionPool;
                SocketUtils::Clear();
        }
} GCoreGlobal;
