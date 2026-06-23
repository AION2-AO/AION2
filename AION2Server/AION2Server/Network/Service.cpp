#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "Listener.h"


Service::~Service()
{
}

void Service::CloseService()
{
}

SessionRef Service::CreateSession()
{
	SessionRef session = _sessionFactory();
	session->SetService(shared_from_this());

	if (_iocpCore->Register(session) == false)
		return nullptr;

	return session;
}

void Service::AddSession(SessionRef session)
{
	std::lock_guard<std::mutex> lock(_serviceLock);
	sessionCount++;
	_sessions.insert(session);
}

void Service::ReleaseSession(SessionRef session)
{
	std::lock_guard<std::mutex> lock(_serviceLock);
	if (_sessions.erase(session) > 0)
	{
		sessionCount--;
	}
}

void Service::Broadcast()
{
}

MMOServerService::MMOServerService(NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: Service(address, core, factory, maxSessionCount)
{
}

bool MMOServerService::Start()
{
	if (CanStart() == false)
		return false;

	_listener = std::make_shared<Listener>();
	if (_listener == nullptr)
		return false;

	return _listener->StartAccept(shared_from_this());
}

void MMOServerService::CloseService()
{
	Service::CloseService();
}

DediService::DediService(NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: Service(address, core, factory, maxSessionCount)
{
}

bool DediService::Start()
{
	if (CanStart() == false)
		return false;

	_listener = std::make_shared<Listener>();
	if (_listener == nullptr)
		return false;

	return _listener->StartAccept(shared_from_this());
}

void DediService::CloseService()
{
	Service::CloseService();
}
