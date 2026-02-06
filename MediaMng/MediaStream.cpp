#include "MediaStream.h"
#include "SelfLog.h"
#include <assert.h>
#include "videoNalu.hpp"
#include <thread>
#include <chrono>

MediaStream::MediaStream(const char* streamId) :streamId(streamId), def(0), rIndex(0), wIndex(0)
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
int MediaStream::OnJsonMediaVideoStream(MEDIA_CODEC_TYPE code, uint8_t* data, size_t size, ushort fps, ushort ibp, ushort w, ushort h, uint64_t pts)
{
    int vSize = mFrames.size();
    if(vSize > 0)
    {
        if(wIndex < vSize)
        {
            if(mFrames[wIndex].IsIde())
            {
                //写入
                int ret = mFrames[wIndex].writeData(data, size);
                if(ret == 1)
                {
                    if(mFrames[wIndex].reSizeBuf(size))
                    {
                        ret = mFrames[wIndex].writeData(data, size);
                    }
                    else
                    {
                        LogOut(BLL, L_ERROR, "new buffer error");
                        return 4;
                    }
                }
                if(ret == 0)
                {
                    mFrames[wIndex].SetIBPType(ibp);
                    mFrames[wIndex].SetCodecType(code);
                    if(ibp == 0)
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
    if(vSize > 0)
    {
        if(wIndex < vSize)
        {
            if(mFrames[wIndex].IsIde())
            {
                //写入
                int ret = mFrames[wIndex].writeData(data, size);
                if(ret == 1)
                {
                    if(mFrames[wIndex].reSizeBuf(size))
                    {
                        ret = mFrames[wIndex].writeData(data, size);
                    }
                    else
                    {
                        LogOut(BLL, L_ERROR, "new buffer error");
                        return 4;
                    }
                }
                if(ret == 0)
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

int MediaStream::GetNextFrame(uint32_t readhandle, av_Frame& frame, ushort& fIndex)
{
    int size = mFrames.size();
    if(size > 0)
    {
        if(rIndex < size)
        {
            if(!mFrames[rIndex].IsIde())
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
    if(fIndex < size)
    {
        mFrames[rIndex].ClearData();
        rIndex = (rIndex + 1) % size;
    }
}
void MediaStream::inc()
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