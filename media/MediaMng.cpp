#include "MediaMng.h"
#include "../deviceMng/JsonDevice.h"
#include "../deviceMng/deviceMng.h"

MediaMng& MediaMng::GetInstance()
{
	static MediaMng *g_MediaMng = new MediaMng();
	return *g_MediaMng;
}
MediaMng::MediaMng():statusRun(false)
{
}
MediaMng::~MediaMng()
{
}

void MediaMng::addStream(MediaStream::Ptr stream)
{
	if (!stream)
		return;
	GMUTEX lock(stmMtx);
	mStreamMap[stream->getStreamId()] = stream;
}
void MediaMng::removeStream(std::string Id)
{
	GMUTEX lock(stmMtx);
	auto it = mStreamMap.find(Id);
	if (it != mStreamMap.end())
	{
		if (it->second.use_count() <= 2)
		{
			mStreamMap.erase(Id);
		}
	}
}
MediaStream::Ptr MediaMng::findStream(std::string Id)
{
	GMUTEX lock(stmMtx);
	auto it = mStreamMap.find(Id);
	if (it != mStreamMap.end())
	{
		return it->second;
	}
	return NULL;
}
void MediaMng::checkStreamStatus()
{
	int count = 0;
	std::map<std::string, MediaStream::Ptr> tmpMap;
	while (statusRun)
	{
		if (count++ > 5)
		{
			{
				GMUTEX lock(stmMtx);
				tmpMap = mStreamMap;
			}
			for (auto& it : tmpMap)
			{
				if (time(0) - it.second->LastFrameTime() >= 10)
				{
					BaseChildDevice* childDev = DeviceMng::Instance().findChildDevice(it.second->getDeviceId());
					if (childDev)
					{
						BaseDevice::Ptr parentDev = childDev->getParentDev();
						if (parentDev && parentDev->devType == BaseDevice::JSON_NVR)
						{
							int err = 0;
							parentDev->Dev_StopPreview(it.second->getStreamHandle(), err);
							if (err == 0)
							{
								MediaMng::GetInstance().removeStream(it.second->getStreamId());
							}
						}
					}
				}
			}
			tmpMap.clear();
		}
	}
}