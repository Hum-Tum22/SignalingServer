#pragma once
#include <string>
#include <memory>
#include <stdint.h>
#include <atomic>
#include <map>
#include <atomic>
#include "media-source.h"
#include "../tools/memPool.h"
#include "../tools/lineBuf.h"
#include "../Plugin/PluginInterface.h"
#include "../tools/unLockQueue.h"

struct vframe_t
{
	const uint8_t* nalu;
	int64_t time;
	long bytes;
	bool idr; // IDR frame
	int frameType;
	int gap;

	vframe_t():nalu(NULL),time(0),bytes(0),idr(false),frameType(0), gap(40)
	{
	}
	bool operator < (const struct vframe_t& v) const
	{
		return time < v.time;
	}
};
typedef enum
{
	GB_CODEC_UNKNOWN = -1,
	GB_CODEC_H264,
	GB_CODEC_H265,
}STREAM_CODEC;
class FrameMemPool : public MemPool, public unLockQueue<vframe_t>
{
	//int Init();
	int Init264();
	int Init265();
	STREAM_CODEC frameType;
	int m_duration;
	int fgap;

	uint8_t *sps;
	uint8_t *pps;
	uint8_t *vps;
	size_t spslen, ppslen, vpslen;
	bool vpsspspps;

	size_t frameIndex;
	size_t lastIdrPos;

	int Init();
public:
	FrameMemPool(size_t length = 1024*1024*5);
	~FrameMemPool();

	int InputFrame(STREAM_CODEC type, uint8_t* data, size_t size, int gap);
	int getReader();
};
using SHANDLE = unsigned long;
class MediaStream
{
public:
	using Ptr = std::shared_ptr<MediaStream>;
	MediaStream(const char* devId, const char* streamId);


	virtual ~MediaStream();

	virtual int getvalue() = 0;
	virtual time_t LastFrameTime() = 0;
	const std::string& getStreamId();
	const std::string& getDeviceId();

	void setStreamHandle(SHANDLE handle);
	const SHANDLE getStreamHandle();

	//void setMediaSource(IMediaSource*s);

	void OnMediaStream(STREAM_CODEC code, uint8_t* data, size_t size, int gap);

	int GetNextFrame(uint32_t readhandle, vframe_t & frame);

	uint32_t createReader();
	void removeReader(uint32_t handle);
	void increasing();
	void reduction();
	int refNum();
protected:
	int     GetReadPos(unsigned int readhandle);
	void    SetReadPos(unsigned int readhandle, unsigned int Pos);
	
private:
	std::string deviceId;
	std::string streamId;
	std::atomic<int> def;
	SHANDLE streamHandle;
	FrameMemPool framePool;

	std::atomic<uint32_t> mHandler;
	std::map<uint32_t, size_t> mReaderMap;
};