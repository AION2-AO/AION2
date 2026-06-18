#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}


bool IocpCore::Register(std::shared_ptr<IocpObject> iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 0, 0);
}

bool IocpCore::Dispatch(uint32_t timeoutMs)
{
	DWORD numBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	if (::GetQueuedCompletionStatus(_iocpHandle, &numBytes, &key, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		IocpObjectRef iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, numBytes);
	}
	else
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode == WAIT_TIMEOUT)
		{
			return false;
		}

		if (iocpEvent != nullptr)
		{
			IocpObjectRef iocpObj = iocpEvent->owner;
			iocpObj->Dispatch(iocpEvent, numBytes);
		}
	}
	return true;
}
