#pragma once

#include "NetAddress.h"
#include "IocpCore.h"
#include "Listener.h"
#include <functional>

enum class ServiceType : uint8
{
	MMOServer,
	DediServer
};

using SessionFactory = std::function<SessionRef(void)>;


class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(NetAddress address, IocpCoreRef core, SessionFactory factory,int32 maxSessionCount = 1)
		:_netAddress(address), _iocpCore(core), _sessionFactory(factory), _maxSessionCount(maxSessionCount)
	{
	}
	virtual ~Service();

	virtual bool Start() abstract;
	bool CanStart() { return _sessionFactory != nullptr; }

	virtual void CloseService();
	void SetSessionFactory(SessionFactory func) { _sessionFactory = func; }

	SessionRef CreateSession();
	void AddSession(SessionRef session);
	void ReleaseSession(SessionRef Session);

	void Broadcast();

	ServiceType GetServiceType() { return _type; }
	IocpCoreRef GetIocpCore() { return _iocpCore; }
	NetAddress GetNetAddress() { return _netAddress; }

	int32 GetCurrentSessionCount() { return sessionCount; }
	int32 GetMaxSessionCount() { return _maxSessionCount; }

protected:
	std::mutex _serviceLock;
	NetAddress _netAddress;
	IocpCoreRef _iocpCore;
	ListenerRef _listener;

	std::set<SessionRef> _sessions;
	int32 sessionCount = 0;
	int32 _maxSessionCount = 0;

	SessionFactory _sessionFactory;
	ServiceType _type;
};

class MMOServerService : public Service
{
public:
	MMOServerService(NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~MMOServerService() {}

	virtual bool Start() override;
	virtual void CloseService() override;
};

class DediService : public Service
{
public:
	DediService(NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~DediService() {}

	virtual bool Start() override;
	virtual void CloseService() override;
};
