#pragma once

#include <iostream>
#include "../device/DeviceManager.h"

class InStreamInfo
{
public:
	unsigned int streamHandle;
	std::string streamId;
	int streamType;				//0:������,1:������,2:����
	int useCount;
	Device devInfo;
};
