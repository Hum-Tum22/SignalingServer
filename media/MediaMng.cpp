#include "MediaMng.h"


MediaMng& MediaMng::GetInstance()
{
	static MediaMng g_MediaMng;
	return g_MediaMng;
}
MediaMng::MediaMng()
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