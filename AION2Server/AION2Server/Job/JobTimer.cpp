#include "pch.h"
#include "JobTimer.h"
#include "JobQueue.h"

/*--------------
	JobTimer
---------------*/

void JobTimer::Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job)
{
	const uint64 executeTick = ::GetTickCount64() + tickAfter;
	JobData* jobData = new JobData(owner, job);

	lock_guard<std::mutex> lock(_lock);
	_items.push(TimerItem{ executeTick, jobData });
}

void JobTimer::Distribute(uint64 now)
{
	if (_distributing.exchange(true))
		return;

	vector<TimerItem> expired;
	{
		lock_guard<std::mutex> lock(_lock);
		while (_items.empty() == false)
		{
			const TimerItem& item = _items.top();
			if (now < item.executeTick)
				break;

			expired.push_back(item);
			_items.pop();
		}
	}

	for (TimerItem& item : expired)
	{
		JobData* jobData = item.jobData;
		// 일감 주인이 살아있는지 확인
		if (JobQueueRef owner = jobData->owner.lock())
			owner->Push(jobData->job);

		delete jobData;
	}

	_distributing.store(false);
}

void JobTimer::Clear()
{
	lock_guard<std::mutex> lock(_lock);
	while (_items.empty() == false)
	{
		const TimerItem& item = _items.top();
		delete item.jobData;
		_items.pop();
	}
}
