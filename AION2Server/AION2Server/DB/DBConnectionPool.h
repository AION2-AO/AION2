#pragma once
#include "DBConnection.h"
#include <mutex>
#include <vector>

/*-------------------
	DBConnectionPool
--------------------*/

class DBConnectionPool
{
public:
	DBConnectionPool();
	~DBConnectionPool();

	bool					Connect(int32 connectionCount, const WCHAR* connectionString);
	void					Clear();

	DBConnection*			Pop();
	void					Push(DBConnection* connection);

private:
	std::mutex				_lock;
	SQLHENV					_environment = SQL_NULL_HANDLE;
	std::vector<DBConnection*>	_connections;
};
