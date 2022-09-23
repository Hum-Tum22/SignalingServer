#include <rutil/ResipAssert.h>
#include "RtpPortManager.h"

//using namespace recon;
using namespace std;

RTPPortManager::RTPPortManager(int portRangeMin, int portRangeMax)
    : mPortRangeMin(portRangeMin),
    mPortRangeMax(portRangeMax)
{
    mPortRangeMin = portRangeMin;
    mPortRangeMax = portRangeMax;
    mRTPPortFreeList.clear();
    for (unsigned int i = mPortRangeMin; i <= mPortRangeMax;)
    {
        mRTPPortFreeList.push_back(i);
        i = i + 2;  // only add even ports - note we are assuming rtpPortRangeMin is even
    }
}

unsigned int
RTPPortManager::allocateRTPPort()
{
    unsigned int port = 0;
    if (!mRTPPortFreeList.empty())
    {
        port = mRTPPortFreeList.front();
        mRTPPortFreeList.pop_front();
    }
    return port;
}

void
RTPPortManager::freeRTPPort(unsigned int port)
{
    resip_assert(port >= mPortRangeMin && port <= mPortRangeMax);
    mRTPPortFreeList.push_back(port);
}