#pragma once

/*----------------
	IocpObject
-----------------*/

class IocpObject : public  std::enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32_t numOfBytes = 0) abstract;
};

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return _iocpHandle; }

	bool Register(std::shared_ptr<IocpObject> iocpObject);
	bool Dispatch(uint32_t timeoutMs = INFINITE);

private: 
	HANDLE _iocpHandle;
};

