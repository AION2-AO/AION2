#pragma once

template<typename T>
class LockQueue
{
public:
	void Push(T item)
	{
		std::lock_guard<std::mutex>lock(_lock);
		_items.push(item);
	}

	T Pop()
	{
		std::lock_guard<std::mutex>lock(_popLock);
		if (_items.empty())
			return T();

		T ret = _items.front();
		_items.pop();
		return ret;
	}
	void PopAll(std::vector<T>& items)
	{
		std::lock_guard<std::mutex>lock(_lock);
		while (T item = Pop())
		{
			items.push_back(item);
		}
	}
	void Clear()
	{
		std::lock_guard<std::mutex>lock(_lock);
		_items = std::queue<T>();
	}

private:
	std::mutex _lock;
	std::mutex _popLock;
	std::queue<T>_items;
};

