#pragma once
#include <string>
#include <mutex>
#include <map>
#include "MediaStream.h"
class MediaMng
{
public:
	using GMUTEX = std::lock_guard<std::mutex>;
	MediaMng();
	~MediaMng();
	static MediaMng& GetInstance();


	void addStream(MediaStream::Ptr);
	void removeStream(std::string Id);
	MediaStream::Ptr findStream(std::string Id);

	void checkStreamStatus();
	MediaStream::Ptr createLiveStream(std::string deviceId, int streamId);
	MediaStream::Ptr createVodStream(std::string deviceId, time_t start, time_t end);
	bool CloseStreamByStreamId(MediaStream::Ptr& ms);

	std::string getStreamId(const std::string& deviceId, time_t start, time_t end);

private:
	std::mutex stmMtx;
	std::map<std::string, MediaStream::Ptr> mStreamMap;
	bool statusRun;
};