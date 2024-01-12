#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <queue>

//#include "unLockQueue.h"
//#include "lineBuf.h"


//template <typename T>
class avMemPool
{
public:
	avMemPool(size_t size = 0) : buf(NULL), bm_ptr(NULL), curBuf(NULL)
		,bufSize(size), readPtr(NULL), readSize(0)
	{
		//6M Kbps  =>11M 缓存15s
		if (bufSize == 0)
		{
			bufSize = 1024 * 1024 * 1;
		}
		buf = new uint8_t[bufSize];
		bm_ptr = new uint8_t[bufSize];
		curBuf = buf;
		rPos = wPos = frPos = fwPos = 0;
		printf("new avMemPool buf:%p, bm_ptr:%p, curBuf:%p\n", buf, bm_ptr, curBuf);
	};
	~avMemPool()
	{
		if (buf)
		{
			delete buf; buf = NULL;
		}
		if (bm_ptr)
		{
			delete bm_ptr; bm_ptr = NULL;
		}
		printf("delete avMemPool\n");
	}
	int wirteData(void* data, size_t size)
	{
		if (bufSize - wPos >= size)
		{
			readPtr = curBuf + wPos;
			readSize = size;
			memcpy(curBuf + wPos, data, size);
			wPos += size;
			return Init();
		}
		else
		{
			if (wPos != rPos)
			{
				frPos = rPos;
				fwPos = wPos;
				printf("------- wPos:%lu rPos:%lu error\n", wPos, rPos);
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
	
protected:
	uint8_t* buf, * bm_ptr, *curBuf;
	size_t bufSize;
	size_t rPos, wPos, frPos, fwPos;

	//可读数据开始指针、有效数据长度
	const uint8_t* readPtr;
	size_t readSize;

	virtual int Init() = 0;
};