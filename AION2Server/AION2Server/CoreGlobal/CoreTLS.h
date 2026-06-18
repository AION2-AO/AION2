#pragma once

// 스레드 고유의 ID
extern thread_local uint32 LThreadId;
extern thread_local uint64 LEndTickCount;

extern thread_local class JobQueue* LCurrentJobQueue;
