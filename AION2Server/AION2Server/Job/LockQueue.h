#pragma once

template<typename T>
class LockQueue
{
public:
	void Push(T item)
	{
		lock_guard<std::mutex>lock(_lock);
		_items.push(item);
	}

	T Pop()
	{
		lock_guard<std::mutex>lock(_lock);
		if (_items.empty())
			return T();

		T ret = _items.front();
		_items.pop();
		return ret;
	}
	void PopAll(vector<T>& items)
	{
		lock_guard<std::mutex>lock(_lock);
		while (T item = Pop())
		{
			items.push_back(item);
		}
	}
	void Clear()
	{
		lock_guard<std::mutex>lock(_lock);
		_items = queue<T>();
	}

private:
	std::mutex _lock;
	queue<T>_items;
};

