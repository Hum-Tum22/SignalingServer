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

private:
	std::mutex stmMtx;
	std::map<std::string, MediaStream::Ptr> mStreamMap;
	bool statusRun;
};