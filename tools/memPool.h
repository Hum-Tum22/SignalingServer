#pragma once

#include <stdint.h>
#include <vector>
#include <queue>

#include "unLockQueue.h"
//#include "lineBuf.h"


//template <typename T>
class MemPool
{
public:
	MemPool(/*size_t count = 128,*/ size_t size = 0) :/*mQueue(count),*/ buf(NULL), bm_ptr(NULL), curBuf(NULL)
		,bufSize(size), readPtr(NULL),readSize(0)
	{
		//6M Kbps  =>11M 缓存15s
		if (bufSize == 0)
		{
			//bufsize = 11796480;
			bufSize = 1024 * 1024 * 1;
		}
		buf = new uint8_t[bufSize];
		bm_ptr = new uint8_t[bufSize];
		curBuf = buf;
		rPos = wPos = frPos = fwPos = 0;
	};
	~MemPool()
	{
		if (buf)
		{
			delete buf; buf = NULL;
		}
		if (bm_ptr)
		{
			delete bm_ptr; bm_ptr = NULL;
		}
		printf("delete MemPool\n");
	}
	int wirteData(void* data, size_t size)
	{
		if (bufSize - wPos >= size)
		{
			readPtr = curBuf + wPos;
			readSize = size;
			memcpy(curBuf + wPos, data, size);
			wPos += size;
			/*if (fwPos)
			{
				readPtr = bm_ptr + frPos;
				readSize = fwPos - frPos;
			}
			else
			{
				readPtr = buf + rPos;
				readSize = wPos - rPos;
			}*/
			return Init();
		}
		else
		{
			if (wPos != rPos)
			{
				frPos = rPos;
				fwPos = wPos;
				printf("------- wPos:%u rPos:%u error\n", wPos, rPos);
			}
			wPos = rPos = 0;
			if (curBuf == buf)
			{
				curBuf = bm_ptr;
			}
			else if (curBuf == bm_ptr)
			{
				curBuf = buf;
			}
			else
			{
				printf("------- curBuf error\n");
			}
			wirteData(data, size);
		}
		return 0;
	};
	/*int GetNextFrame(T& tt)
	{
		T* frame = mQueue.front();
		if (frame)
		{
			tt = *frame;
			mQueue.pop();
			return 0;
		}
		return -1;
	}

	T* front()
	{
		return mQueue.front();
	}

	void pop()
	{
		mQueue.pop();
	}

	bool push(T frame)
	{
		return mQueue.push(frame);
	}*/
//private:
protected:
	//unLockQueue<T> mQueue;
	/*std::vector<T> mVector;
	std::queue<T> cacheQueue;
	size_t MAX_SIZE, write, read;*/

	uint8_t* buf, * bm_ptr, *curBuf;
	size_t bufSize;
	size_t rPos, wPos, frPos, fwPos;

	//可读数据开始指针、有效数据长度
	const uint8_t* readPtr;
	size_t readSize;

	virtual int Init() = 0;
};