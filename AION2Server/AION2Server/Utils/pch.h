#pragma once

#define _HAS_STD_BYTE 0

#include "Types.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"

#include <iostream>
#include <assert.h>

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <windows.h>
#include <vector>
#include <set>
#include <queue>
#include <functional>
#include <memory>

#include "JobQueue.h"
#include "Session.h"

