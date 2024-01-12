#pragma once

#include <vector>
#include <queue>
#include <stdio.h>

struct audioparam
{
	int chl; //1 单声道，2双声道，实际写入buf 里面的为char
	unsigned int frate;//采样率
};
struct KeyFrameparam
{
	unsigned int vwidth;//视频宽 
	unsigned int vheight;//视频高
};
struct frame_t
{
	const uint8_t* frame;
	int bufSize;
	int frameLen;
	int64_t time;
	bool idr; // IDR frame
	int frameType;

	frame_t(int size = 1024) :frame(NULL), bufSize(size), frameLen(0), time(0), idr(false), frameType(0)
	{
		frame = new uint8_t[bufSize];
	}
	~frame_t()
	{
		if (frame)
		{
			delete frame; frame = NULL;
			frameLen = 0;
		}
	}
	bool operator < (const struct frame_t& f) const
	{
		return time < f.time;
	}
};
class AVCache
{
	std::vector<frame_t*> mVector;

	std::list<frame_t*> mFreeMaxFrame;
	std::list<frame_t*> mFreeCenFrame;
	std::list<frame_t*> mFreeMinFrame;
	time_t earliest, oldestl;
	size_t write, read;
public:
	AVCache() : earliest(0), oldestl(0), write(0), read(0)
	{
		//mVector.resize(MAX_SIZE);
	};
	~AVCache()
	{
		mVector.clear();
		printf("*** unLockQueue delete\n");
	}
	int GetNext(frame_t*& tt)
	{
		frame_t* frame = front();
		if (frame)
		{
			tt = frame;
			pop();
			return 0;
		}
		return 1;
	}

	frame_t* front()
	{
		if ((write - read + MAX_SIZE) % MAX_SIZE >= 1)
			return &mVector[read];
		return NULL;
	}
	frame_t* front(size_t readIndex)
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

	bool push(frame_t* frame)
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