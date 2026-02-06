#include "MediaStream.h"
#include "SelfLog.h"
#include <assert.h>
#include "videoNalu.hpp"
#include <thread>
#include <chrono>


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
            frame.frameType = GB_CODEC_H264;
            frame.time = fgap * frameIndex++;
            frame.idr = 5 == nal_unit_type; // IDR-frame
            frame.gap = fgap;
            if (frame.idr)
            {
                lastIdrPos = getWritePos();
            }
            push(frame);


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
        LogOut(BLL, L_DEBUG, "************ init264");
    }
    readPtr = NULL;
    readSize = 0;
    m_duration = fgap * frameIndex;
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
            frame.frameType = GB_CODEC_H265;
            frame.time = fgap * count++;
            frame.gap = fgap;
            frame.idr = (NAL_IDR_N_LP == nal_unit_type || NAL_IDR_W_RADL == nal_unit_type); // IDR-frame
            push(frame);
            nalu = pn;
        }

        p = pn;
    }

    m_duration = fgap * count;
    return 0;
}
FrameMemPool::FrameMemPool(size_t length) : avMemPool(length), unLockQueue(), frameType(GB_CODEC_UNKNOWN), m_duration(0), fgap(40), sps(NULL), pps(NULL), vps(NULL), spslen(0), ppslen(0), vpslen(0), vpsspspps(true)
, frameIndex(0), lastIdrPos(0)
{
}
FrameMemPool::~FrameMemPool()
{
    //sps(NULL), pps(NULL), vps(NULL)
    if (sps)
    {
        delete[] sps; sps = NULL;
    }
    if (pps)
    {
        delete[] pps; pps = NULL;
    }
    if (vps)
    {
        delete[] vps; vps = NULL;
    }
}
int FrameMemPool::Init()
{
    int ret = 0;
    if (frameType == GB_CODEC_H264)
    {
        ret = Init264();
    }
    else if (frameType == GB_CODEC_H265)
    {
        ret = Init265();
    }
    return ret;
}
int FrameMemPool::InputFrame(STREAM_CODEC type, uint8_t* data, size_t size, int gap)
{
    if (frameType < 0 || type != frameType)
    {
        frameType = type;
    }
    if (fgap == 0)
        fgap = gap;
    return wirteData(data, size);
}
int FrameMemPool::getReader()
{
    return lastIdrPos;
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
void FrameMemPool::setGap(int gap)
{
    fgap = gap;
}
const int FrameMemPool::getGap()
{
    return fgap;
}

MediaStream::MediaStream(const char* devId, const char* streamId) :deviceId(devId), streamId(streamId), def(0), streamHandle(0), streamType(0), framePool(1024 * 1024 * 10)//, m_duration(40)
{
    rIndex = wIndex = 0;
    mFrames.resize(2);
}
MediaStream::~MediaStream()
{
    mFrames.clear();
}
const std::string& MediaStream::getStreamId()
{
    return streamId;
}
const std::string& MediaStream::getDeviceId()
{
    return deviceId;
}
void MediaStream::setStreamHandle(SHANDLE handle)
{
    streamHandle = handle;
}
const SHANDLE MediaStream::getStreamHandle()
{
    return streamHandle;
}
void MediaStream::setFrameRate(int fps)
{
    if (fps > 0)
    {
        framePool.setGap(1 / fps);
    }
}
const int MediaStream::getFrameRate()
{
    return framePool.getGap();
}
void MediaStream::setStreamType(int lv)
{
    streamType = lv;
}
const int MediaStream::getStreamType()
{
    return streamType;
}
//void MediaStream::setMediaSource(IMediaSource* s)
//{
//	source = s;
//}
int MediaStream::OnMediaStream(STREAM_CODEC code, uint8_t* data, size_t size, int gap)
{
    if (streamType == 0)
    {
        return framePool.InputFrame(code, data, size, gap);
    }
    else if (streamType == 1)
    {
        return framePool.InputFrame(code, data, size, gap);
        //if()
        //std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    else
    {
        return framePool.InputFrame(code, data, size, gap);
    }
}
int MediaStream::OnJsonMediaVideoStream(MEDIA_CODEC_TYPE code, uint8_t* data, size_t size, ushort fps, ushort ibp, ushort w, ushort h, uint64_t pts)
{
    int vSize = mFrames.size();
    if (vSize > 0)
    {
        if (wIndex < vSize)
        {
            if (mFrames[wIndex].IsIde())
            {
                //写入
                int ret = mFrames[wIndex].writeData(data, size);
                if (ret == 1)
                {
                    mFrames[wIndex].reSizeBuf(size);
                    ret = mFrames[wIndex].writeData(data, size);
                }
                if (ret == 0)
                {
                    mFrames[wIndex].SetIBPType(ibp);
                    mFrames[wIndex].SetCodecType(code);
                    if (ibp == 0)
                    {
                        mFrames[wIndex].SetWidHeight(w, h);
                    }
                    else
                    {
                        mFrames[wIndex].SetWidHeight(0, 0);
                    }
                    mFrames[wIndex].SetFrameRate(fps);
                    mFrames[wIndex].SetPTS(pts);
                    wIndex = (wIndex + 1) % vSize;
                    // LogOut(BLL, L_ERROR, "write frame type ibp:%d", ibp);
                    return 0;
                }
                else
                {
                    LogOut(BLL, L_ERROR, "write frame type ibp:%d error:%d", ibp, ret);
                    return 3;
                }
            }
            return 1;
        }
    }
    return 2;
}
int MediaStream::OnJsonMediaAudioStream(MEDIA_CODEC_TYPE code, uint8_t* data, size_t size, int chl, int sampleRate)
{
    int vSize = mFrames.size();
    if (vSize > 0)
    {
        if (wIndex < vSize)
        {
            if (mFrames[wIndex].IsIde())
            {
                //写入
                int ret = mFrames[wIndex].writeData(data, size);
                if (ret == 1)
                {
                    mFrames[wIndex].reSizeBuf(size);
                    ret = mFrames[wIndex].writeData(data, size);
                }
                if (ret == 0)
                {
                    mFrames[wIndex].SetCodecType(code);
                    mFrames[wIndex].SetChls(chl);
                    mFrames[wIndex].SetSampleRate(sampleRate);
                    wIndex = (wIndex + 1) % vSize;
                    return 0;
                }
                else
                {
                    return 3;
                }
            }
            return 1;
        }
    }
    return 2;
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
int MediaStream::GetNextFrameEx(uint32_t readhandle, av_Frame& frame, ushort& fIndex)
{
    int size = mFrames.size();
    if (size > 0)
    {
        if (rIndex < size)
        {
            if (!mFrames[rIndex].IsIde())
            {
                // 读取
                frame = mFrames[rIndex];
                fIndex = rIndex;
                return 0;
            }
            return 1;
        }
    }
    return 2;
}
void MediaStream::freeFrameByIndex(ushort fIndex)
{
    int size = mFrames.size();
    if (fIndex < size)
    {
        mFrames[rIndex].ClearData();
        rIndex = (rIndex + 1) % size;
    }
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
void MediaStream::increasing()
{
    def++;
}
void MediaStream::reduction()
{
    def--;
}
int MediaStream::refNum()
{
    return def.load();
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