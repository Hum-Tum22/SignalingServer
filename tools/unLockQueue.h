#pragma once
#include <vector>
#include <queue>
#include <stdio.h>
#include "SelfLog.h"

template <typename T>
class unLockQueue
{
	std::vector<T> mVector;
	std::queue<T> cacheQueue;
	size_t MAX_SIZE, write, read;
	bool dump()
	{
		//缓冲中还有数据
		while (!cacheQueue.empty())
		{
			if ((write + 1) % MAX_SIZE == read)
				return true;
			// 优先将缓冲中的数据写入到队列
			mVector[write] = cacheQueue.front();
			write = ++write % MAX_SIZE;
			cacheQueue.pop();
		}
		return false;
	}
public:
	unLockQueue(size_t count = 128) :MAX_SIZE(count), write(0), read(0) {
		mVector.resize(MAX_SIZE);
	};
	~unLockQueue()
	{
		mVector.clear();
		LogOut("BLL", L_DEBUG, "*** unLockQueue delete");
	}
	int GetNext(T& tt)
	{
		T* frame = front();
		if (frame)
		{
			tt = *frame;
			pop();
			return 0;
		}
		return 1;
	}
	int GetNextByHanlde(uint32_t handle, T& tt)
	{
		T* frame = front(handle);
		if (frame)
		{
			tt = *frame;
			//pop();
			return 0;
		}
		return 1;
	}

	T* front()
	{
		if ((write - read + MAX_SIZE) % MAX_SIZE >= 1)
			return &mVector[read];
		return NULL;
	}
	T* front(size_t readIndex)
	{
		if (readIndex >= MAX_SIZE)
		{
			return NULL;
		}
		if ((write - readIndex + MAX_SIZE) % MAX_SIZE >= 1)
			return &mVector[readIndex];
		return NULL;
	}

	void pop()
	{
		if ((write - read + MAX_SIZE) % MAX_SIZE >= 1)
			read = (++read) % MAX_SIZE;
		else
			return;
	}

	bool push(T frame)
	{
		if (!dump() && !((write + 1) % MAX_SIZE == read))
		{
			//printf("H264FileReader::push write:%d\n", write);
			mVector[write] = frame;
			write = (++write) % MAX_SIZE;
			return true;
		}
		else
		{
			read = (read + 1) % MAX_SIZE;
			//如果队列满了，则写入缓冲
			cacheQueue.push(frame);
		}
		return true;
	}
	//MAX_SIZE, write, read;
	const size_t& getWritePos() { return write; };
	const size_t& getReadPos() { return read; };
	const size_t& getMaxSize() { return MAX_SIZE; };

};