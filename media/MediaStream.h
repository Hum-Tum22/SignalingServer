#pragma once
#include <string>
#include <memory>
#include <stdint.h>
#include <atomic>
#include <map>
#include "media-source.h"
#include "../tools/memPool.h"
#include "../tools/lineBuf.h"

struct vframe_t
{
	const uint8_t* nalu;
	int64_t time;
	long bytes;
	bool idr; // IDR frame
	int frameType;

	vframe_t():nalu(NULL),time(0),bytes(0),idr(false),frameType(0)
	{
	}
	bool operator < (const struct vframe_t& v) const
	{
		return time < v.time;
	}
};

class FrameMemPool : public MemPool, public unLockQueue<vframe_t>
{
	//int Init();
	int Init264();
	int Init265();
	int frameType;
	int m_duration;

	uint8_t *sps;
	uint8_t *pps;
	uint8_t *vps;
	size_t spslen, ppslen, vpslen;
	bool vpsspspps;

	size_t frameIndex;

	LineBuffer mFrame;
	int Init();
public:
	FrameMemPool();
	~FrameMemPool();

	int Input(int type, uint8_t* data, size_t size);
	int InputFrame(uint8_t* data, size_t size);
	int getReader();
};
using SHANDLE = unsigned long;
class MediaStream
{
public:
	using Ptr = std::shared_ptr<MediaStream>;
	MediaStream(const char* Id);


	virtual ~MediaStream();

	virtual int getvalue() = 0;
	const std::string getStreamId();

	void setStreamHandle(SHANDLE handle);
	const SHANDLE getStreamHandle();

	void setMediaSource(IMediaSource*s);

	void OnMediaStream(int type, uint8_t* data, size_t size);

	int GetNextFrame(uint32_t readhandle, vframe_t & frame);

	uint32_t createReader();
	void removeReader(uint32_t handle);
protected:
	int     GetReadPos(unsigned int readhandle);
	void    SetReadPos(unsigned int readhandle, unsigned int Pos);
	
private:
	std::string streamId;
	SHANDLE streamHandle;
	IMediaSource* source;
	FrameMemPool framePool;

	std::atomic<uint32_t> mHandler;
	std::map<uint32_t, size_t> mReaderMap;
};