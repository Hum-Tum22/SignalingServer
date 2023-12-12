#ifndef XML_MSG_ANALYSIS_H_
#define XML_MSG_ANALYSIS_H_
#include <iostream>
#include <vector>
#include "tinyxml2.h"
#include "device/DeviceInfo.h"


using namespace tinyxml2;
using namespace std;

bool AnalyzeReceivedSipMsg(const char* MsgStr, GB28181XmlMsg& XmlMsg);


#endif