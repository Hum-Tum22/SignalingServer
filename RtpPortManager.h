#pragma once


#include <deque>


class RTPPortManager
{
public:
	RTPPortManager(int portRangeMin = 17000, int portRangeMax = 18000);
	unsigned int allocateRTPPort();
	void freeRTPPort(unsigned int port);

private:
	unsigned int mPortRangeMin;
	unsigned int mPortRangeMax;
	std::deque<unsigned int> mRTPPortFreeList;
};
