#pragma once

#include <iostream>
#include "../device/DeviceManager.h"

class InStreamInfo
{
public:
	unsigned int streamHandle;
	std::string streamId;
	int streamType;				//0:主码流,1:子码流,2:其它
	int useCount;
	Device devInfo;
};
