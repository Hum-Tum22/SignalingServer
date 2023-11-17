#include "MediaStream.h"
#include <assert.h>


#define H264_NAL(v)	(v & 0x1F)
enum { NAL_IDR = 5, NAL_SEI = 6, NAL_SPS = 7, NAL_PPS = 8 };
#define H265_NAL(v)	 ((v>> 1) & 0x3f)
enum { NAL_IDR_W_RADL = 19, NAL_IDR_N_LP = 20, NAL_VPS = 32, NAL_SPS_5 = 33, NAL_PPS_5 = 34, NAL_SEI_5 = 39 };


static inline const uint8_t* search_start_code(const uint8_t* ptr, const uint8_t* end)
{
	for (const uint8_t* p = ptr; p + 3 < end; p++)
	{
		if (0x00 == p[0] && 0x00 == p[1] && (0x01 == p[2] || (0x00 == p[2] && 0x01 == p[3])))
			return p;
	}
	return end;
}

static inline int h264_nal_type(const unsigned char* ptr)
{
	int i = 2;
	assert(0x00 == ptr[0] && 0x00 == ptr[1]);
	if (0x00 == ptr[2])
		++i;
	assert(0x01 == ptr[i]);
	return H264_NAL(ptr[i + 1]);
}

static inline int h264_nal_new_access(const unsigned char* ptr, const uint8_t* end)
{
	int i = 2;
	if (end - ptr < 4)
		return 1;
	assert(0x00 == ptr[0] && 0x00 == ptr[1]);
	if (0x00 == ptr[2])
		++i;
	assert(0x01 == ptr[i]);
	int nal_unit_type = H264_NAL(ptr[i + 1]);
	if (nal_unit_type < 1 || nal_unit_type > 5)
		return 1;

	if (ptr + i + 2 > end)
		return 1;

	// Live555 H264or5VideoStreamParser::parse
	// The high-order bit of the byte after the "nal_unit_header" tells us whether it's
	// the start of a new 'access unit' (and thus the current NAL unit ends an 'access unit'):
	return (ptr[i + 2] & 0x80) != 0 ? 1 : 0;
}


static inline int h265_nal_type(const unsigned char* ptr)
{
	int i = 2;
	assert(0x00 == ptr[0] && 0x00 == ptr[1]);
	if (0x00 == ptr[2])
		++i;
	assert(0x01 == ptr[i]);
	return H265_NAL(ptr[i + 1]);
}

//int FrameMemPool::Init()
//{
//	//NAL_IDR = 5, NAL_SEI = 6, NAL_SPS = 7, NAL_PPS = 8
//	if (frameType == 264)
//	{
//		Init264();
//	}
//	else if (frameType == 265)
//	{
//		Init265();
//	}
//	return 0;
//}
static FILE* pfp = NULL;
static FILE* sfp = NULL;
int FrameMemPool::Init264()
{
	const uint8_t* end = readPtr + readSize;
	const uint8_t* nalu = search_start_code(readPtr, end);
	const uint8_t* p = nalu;

	while (p < end)
	{
		const unsigned char* pn = search_start_code(p + 4, end);
		size_t bytes = pn - nalu;

		int nal_unit_type = h264_nal_type(p);
		//printf("Init264 ** nalu type:%d, time:%lld\n", nal_unit_type, time(0));
		assert(0 != nal_unit_type);
		if (nal_unit_type <= 5 && h264_nal_new_access(pn, end))
		{
			if (sps && pps) vpsspspps = false; // don't need more sps/pps

			vframe_t frame;
			frame.nalu = nalu;
			frame.bytes = (long)bytes;
			frame.frameType = 264;
			frame.time = 40 * frameIndex++;
			frame.idr = 5 == nal_unit_type; // IDR-frame
			//m_videos.push_back(frame);
			push(frame);
#ifdef _DEBUG
			if (!pfp)
			{
				pfp = fopen("poolH264.h264", "wb+");
			}
			if (pfp)
			{
				fwrite(frame.nalu, frame.bytes, 1, pfp);
			}
#endif
			if (frame.idr)
			{
				//printf("push idr time:%lld\n", time(0));
			}
			
			nalu = pn;
		}
		else if (NAL_SPS == nal_unit_type || NAL_PPS == nal_unit_type)
		{
			//printf("----   sps or pps \n");
			if (vpsspspps)
			{
				size_t n = 0x01 == p[2] ? 3 : 4;
				if (NAL_SPS == nal_unit_type)
				{
					spslen = bytes - n;
					sps = new uint8_t[spslen];
					memcpy(sps, p + n, spslen);
				}
				else if (NAL_PPS == nal_unit_type)
				{
					ppslen = bytes - n;
					pps = new uint8_t[ppslen];
					memcpy(pps, p + n, ppslen);
				}
			}
		}

		p = pn;
	}

	if (wPos - rPos >= readSize)
	{
		rPos += readSize;
	}
	else
	{
		printf("************ init264\n");
	}
	readPtr = NULL;
	readSize = 0;
	m_duration = 40 * frameIndex;
	return 0;
}

int FrameMemPool::Init265()
{
	size_t count = 0;
	bool vpsspspps = true;

	const uint8_t* end = readPtr + readSize;
	const uint8_t* nalu = search_start_code(readPtr, end);
	const uint8_t* p = nalu;

	while (p < end)
	{
		const unsigned char* pn = search_start_code(p + 4, end);
		size_t bytes = pn - nalu;

		int nal_unit_type = h265_nal_type(p);
		assert(0 <= nal_unit_type);

		if (NAL_VPS == nal_unit_type || NAL_SPS_5 == nal_unit_type || NAL_PPS_5 == nal_unit_type)
		{
			if (vpsspspps)
			{
				size_t n = 0x01 == p[2] ? 3 : 4;

				if (NAL_SPS_5 == nal_unit_type)
				{
					spslen = bytes - n;
					sps = new uint8_t[spslen];
					memcpy(sps, p + n, spslen);
				}
				else if (NAL_PPS_5 == nal_unit_type)
				{
					ppslen = bytes - n;
					pps = new uint8_t[ppslen];
					memcpy(pps, p + n, ppslen);
				}
				else if (NAL_VPS == nal_unit_type)
				{
					vpslen = bytes - n;
					vps = new uint8_t[vpslen];
					memcpy(vps, p + n, vpslen);
				}
			}
		}

		{
			if (sps && pps && vps) vpsspspps = false; // don't need more vps/sps/pps

			vframe_t frame;
			frame.nalu = nalu;
			frame.bytes = bytes;
			frame.frameType = 265;
			frame.time = 40 * count++;
			frame.idr = (NAL_IDR_N_LP == nal_unit_type || NAL_IDR_W_RADL == nal_unit_type); // IDR-frame
			push(frame);
			nalu = pn;
		}

		p = pn;
	}

	m_duration = 40 * count;
	return 0;
}
FrameMemPool::FrameMemPool() :unLockQueue(), MemPool(1024*1024*5), frameType(-1), m_duration(0), sps(NULL), pps(NULL), vps(NULL), spslen(0), ppslen(0), vpslen(0), vpsspspps(true)
	, frameIndex(0), mFrame(512 * 1024)
{
}
FrameMemPool::~FrameMemPool()
{
#ifdef _DEBUG
	if (pfp)
	{
		fclose(pfp); pfp = NULL;
	}
#endif
	//sps(NULL), pps(NULL), vps(NULL)
	if (sps)
	{
		delete sps; sps = NULL;
	}
	if (pps)
	{
		delete pps; pps = NULL;
	}
	if (vps)
	{
		delete vps; vps = NULL;
	}
}
int FrameMemPool::Init()
{
	int ret = 0;
	if (frameType == 264)
	{
		ret = Init264();
	}
	else
	{
		ret = Init265();
	}
	return ret;
}
int FrameMemPool::Input(int type, uint8_t *data, size_t size)
{
	if (frameType < 0 && type == 0)
	{
		int nalutype = h264_nal_type(data);
		//NAL_IDR = 5, NAL_SEI = 6, NAL_SPS = 7, NAL_PPS = 8
		if (nalutype == NAL_IDR || nalutype == NAL_SEI || nalutype == NAL_SPS || nalutype == NAL_PPS)
		{
			frameType = 264;
		}
		else
		{
			nalutype = h265_nal_type(data);
			//NAL_IDR_W_RADL = 19, NAL_IDR_N_LP = 20, NAL_VPS = 32, NAL_SPS_5 = 33, NAL_PPS_5 = 34, NAL_SEI_5 = 39
			if (nalutype == NAL_IDR_W_RADL || nalutype == NAL_IDR_N_LP || nalutype == NAL_VPS || nalutype == NAL_SPS_5
				|| nalutype == NAL_PPS_5 || nalutype == NAL_SEI_5)
			{
				frameType = 265;
			}
			else
			{
				return 0;
			}
		}
	}
	else
	{
	}
	if (frameType == 264)
	{
		int nalutype = h264_nal_type(data);
		if (nalutype == NAL_SPS || nalutype == NAL_PPS)
		{
			const uint8_t* end = data + size;
			const uint8_t* nalu = search_start_code(data + 3, end);
			if (nalu == end)
			{
				if (mFrame.tailSize() > (int)size)
				{
					mFrame.writeBuf((char*)data, size);
				}
				else if(mFrame.freeSize() > (int)size)
				{
					mFrame.DataForward();
					mFrame.writeBuf((char*)data, size);
				}
				else
				{
					int datalen = mFrame.dataSize();
					mFrame.resetBuf(datalen + size);
					if (mFrame.tailSize() > (int)size)
					{
						mFrame.writeBuf((char*)data, size);
					}
					else
					{
						Input(0, data, size);
					}
				}
			}
		}
		else
		{
			if (mFrame.dataSize() > 0)
			{
				if (mFrame.tailSize() > (int)size)
				{
					mFrame.writeBuf((char*)data, size);
				}
				else if (mFrame.freeSize() > (int)size)
				{
					mFrame.DataForward();
					mFrame.writeBuf((char*)data, size);
				}
				else
				{
					int datalen = mFrame.dataSize();
					mFrame.resetBuf(datalen + size);
					if (mFrame.tailSize() > (int)size)
					{
						mFrame.writeBuf((char*)data, size);
					}
					else
					{
						Input(0, data, size);
					}
				}
				int nal_unit_type = h264_nal_type(data);
				if (nal_unit_type != 5)
				{
					printf("\n");
				}
				//printf("--- input idr time:%lld \n", time(0));
				int ret = InputFrame((uint8_t*)mFrame.Buf(), mFrame.dataSize());
				mFrame.clear();
				return ret;
			}
			return InputFrame(data, size);
		}
	}
	else if (frameType == 265)
	{
		int nalutype = h265_nal_type(data);
		//NAL_VPS = 32, NAL_SPS_5 = 33, NAL_PPS_5
		if (nalutype == NAL_SPS_5 || nalutype == NAL_PPS_5 || nalutype == NAL_VPS)
		{
			const uint8_t* end = data + size;
			const uint8_t* nalu = search_start_code(data + 3, end);
			if (nalu == end)
			{
				if (mFrame.tailSize() > (int)size)
				{
					mFrame.writeBuf((char*)data, size);
				}
				else if (mFrame.freeSize() > (int)size)
				{
					mFrame.DataForward();
					mFrame.writeBuf((char*)data, size);
				}
				else
				{
					int datalen = mFrame.dataSize();
					mFrame.resetBuf(datalen + size);
					mFrame.writeBuf((char*)data, size);
				}
			}
		}
		else
		{
			if (mFrame.dataSize() > 0)
			{
				if (mFrame.tailSize() > (int)size)
				{
					mFrame.writeBuf((char*)data, size);
				}
				else if (mFrame.freeSize() > (int)size)
				{
					mFrame.DataForward();
					mFrame.writeBuf((char*)data, size);
				}
				else
				{
					int datalen = mFrame.dataSize();
					mFrame.resetBuf(datalen + size);
					mFrame.writeBuf((char*)data, size);
				}
				int ret = InputFrame((uint8_t*)mFrame.Buf(), mFrame.dataSize());
				printf("--- input idr time:%lld\n", time(0));
				mFrame.clear();
				return ret;
			}
			return InputFrame(data, size);;
		}
	}
	return 0;
}
int FrameMemPool::InputFrame(uint8_t* frame, size_t size)
{
	if (frameType == 264)
	{
		return wirteData(frame, size);
		//return Init264();
	}
	else if (frameType == 265)
	{
		return wirteData(frame, size);
		//return Init265();
	}
	return 0;
}
int FrameMemPool::getReader()
{
	int size = getMaxSize();
	vframe_t frame;
	frame.idr = false;
	while (size-- > 0)
	{
		int ret = GetNext(frame);
		if (ret == 0 && frame.idr)
		{
			return getReadPos() - 1;
		}
	}
	return -1;
}

MediaStream::MediaStream(const char* Id) :streamId(Id), streamHandle(0), source(NULL) //, m_duration(40)
{
}
MediaStream::~MediaStream()
{
#ifdef _DEBUG
	if (sfp)
	{
		fclose(sfp); sfp = NULL;
	}
#endif
}
const std::string MediaStream::getStreamId()
{
	return streamId;
}
void MediaStream::setStreamHandle(SHANDLE handle)
{
	streamHandle = handle;
}
const SHANDLE MediaStream::getStreamHandle()
{
	return streamHandle;
}
void MediaStream::setMediaSource(IMediaSource* s)
{
	source = s;
}
void MediaStream::OnMediaStream(int type, uint8_t* data, size_t size)
{
#ifdef _DEBUG
	if (!sfp)
	{
		sfp = fopen("stream.h264", "wb+");
	}
	if (sfp)
	{
		fwrite(data, size, 1, sfp);
	}
#endif
	framePool.Input(type, data, size);
}
int MediaStream::GetNextFrame(uint32_t handle, vframe_t& frame)
{
	int readpos = GetReadPos(handle);
	if (readpos == -1)
	{
		return -1; // readhandle 不存在, 并且没有I帧
	}
	if (readpos == framePool.getWritePos()) //
	{
		return -2; //  没有收到新的数据
	}
	int ret = framePool.GetNextByHanlde(readpos, frame);
	if (ret == 0)
	{
		if (!frame.nalu)
			return -3;
		readpos = (readpos + 1) % framePool.getMaxSize();
		SetReadPos(handle, readpos);
		return 0;
	}
	return -4;
}

uint32_t MediaStream::createReader()
{
	uint32_t handle = ++mHandler;
	mReaderMap[handle] = framePool.getReader();
	return handle;
}
void MediaStream::removeReader(uint32_t handle)
{
	mReaderMap.erase(handle);
}
int MediaStream::GetReadPos(unsigned int readhandle)
{
	int readPos = -1;
	auto it = mReaderMap.find(readhandle);
	if (it != mReaderMap.end())
	{
		readPos = it->second;
	}
	if (readPos < 0)
	{
		readPos = framePool.getReader();
	}
	return readPos;
}
void MediaStream::SetReadPos(unsigned int readhandle, unsigned int Pos)
{
	auto it = mReaderMap.find(readhandle);
	if (it != mReaderMap.end())
	{
		it->second = Pos;
	}
}