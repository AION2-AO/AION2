#include "pch.h"
#include <iostream>
#include "Service.h"
#include "GameSession.h"
#include "DedicatedSession.h"
#include "ThreadManager.h"
#include "DB/DBConnectionPool.h"
#include "Room/Room.h"
#include "PacketHandler.h"

void DoWorkerJob(const ServiceRef& service)
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

	//if (!GNavMeshService.LoadMapData())
	//{
	//	std::cout << "Failed to load NavMesh map data!" << std::endl;
	//	return 0;
	//}

	// DB Connect
	ASSERT_CRASH(GDBConnectionPool->Connect(5, L"Driver={ODBC Driver 17 for SQL Server}; Server=localhost\\SQLEXPRESS03; Database=AION2_DB; Trusted_Connection=yes;"));
	IocpCoreRef iocpCore = std::make_shared<IocpCore>();
	IocpCoreRef dediIocpCore = std::make_shared<IocpCore>();

	MMOServerServiceRef service = make_shared<MMOServerService>(
		NetAddress(L"172.16.30.107", 7777),
		iocpCore,
		[]() { return  std::make_shared<GameSession>(); },
		100
	);

	DediServiceRef dediService = make_shared<DediService>(
		NetAddress(L"172.16.30.107", 9999),
		dediIocpCore,
		[]() { return  std::make_shared<DedicatedSession>(); },
		10
	);

	if (service->Start())
	{
		std::cout << "Server Started on Port 7777" << std::endl;
	}

	if (dediService->Start())
	{
		std::cout << "Dedi Server Started on Port 9999" << std::endl;
	}

	GRoom->DoTimer(180000, &Room::HandleSavePlayerHp);

	int32 maxCore = std::thread::hardware_concurrency();
	for (int32 i = 0; i < maxCore - 5; i++)
	{
		GThreadManager->Launch([&service]()
			{
				DoWorkerJob(service);
			});
	}

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([&dediService]()
			{
				DoWorkerJob(dediService);
			});
	}

	// Main Thread
	DoWorkerJob(service);

	GThreadManager->Join();
}
