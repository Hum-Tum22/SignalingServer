#pragma once
#include <string>
#include <mutex>
#include <map>
#include <list>
#include "MediaStream.h"
#include "DeviceInfo.h"
class MediaMng
{
public:
	using GMUTEX = std::lock_guard<std::mutex>;
	MediaMng();
	~MediaMng();
	static MediaMng& GetInstance();


	void addStream(MediaStream::Ptr);
	void removeStream(std::string streamId);
	MediaStream::Ptr findStream(std::string streamId);

	void checkStreamStatus();
	MediaStream::Ptr createLiveStream(std::string deviceId, int streamId);
	MediaStream::Ptr createVodStream(std::string deviceId, time_t start, time_t end);
	bool CloseStreamByStreamId(MediaStream::Ptr &ms);

	bool GB28181QueryRecordInfo(RecordInfoQueryMsg recordQuery, std::list<RecordInfoResponseItem> &records);

	std::string CreateStreamId(const std::string& deviceId, time_t start, time_t end);

private:
	std::mutex stmMtx;
	std::map<std::string, MediaStream::Ptr> mStreamMap;
	bool statusRun;
};