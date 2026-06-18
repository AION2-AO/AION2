#include "pch.h"
#include <iostream>
#include "Service.h"
#include "GameSession.h"
#include "ThreadManager.h"
#include "DB/DBConnectionPool.h"
#include "PacketHandler.h"

void DoWorkerJob(MMOServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;

		service->GetIocpCore()->Dispatch(10);

		ThreadManager::DistributeReservedJobs();

		ThreadManager::DoGlobalQueueWork();
	}
}

int main()
{
	PacketHandler::Init();

	// DB 연결
	//ASSERT_CRASH(GDBConnectionPool->Connect(10, L"Driver={SQLServer}; Server = (localdb)\\MSSQLLocalDB; Database = AION2_DB; Trusted_Connection=yes;"));
	IocpCoreRef iocpCore = make_shared<IocpCore>();

	MMOServerServiceRef service = make_shared<MMOServerService>(
		NetAddress(L"127.0.0.1", 7777),
		iocpCore,
		[]() { return make_shared<GameSession>(); },
		100
	);

	if (service->Start())
	{
		std::cout << "Server Started on Port 7777" << std::endl;
	}
	else
	{
		std::cout << "Server Start Failed" << std::endl;
		return 0;
	}

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([&service]()
			{
				DoWorkerJob(service);
			});
	}

	// Main Thread
	DoWorkerJob(service);

	GThreadManager->Join();
}
