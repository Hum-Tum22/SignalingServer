#include "avFrame.h"
#include "SelfLog.h"
#include <memory>

av_Frame::av_Frame(int size) :mData(NULL), mDataSize(0), mBufSize(size)
{
    if(size > 0)
    {
        mData = new uint8_t[size];
    }
    memset(&mFrame, 0, sizeof(mFrame));
}
av_Frame::~av_Frame()
{
    if(mData)
    {
        mDataSize = 0;
        mBufSize = 0;
        LogOut(BLL, L_ERROR, "av_Frame delete :%p", mData);
        delete[] mData;
        mData = NULL;
    }
}
int av_Frame::writeData(uint8_t* data, int size)
{
    if(mData && mBufSize >= size)
    {
        if(memcpy(mData, data, size))
        {
            mDataSize = size;
            return 0;
        }
        return 2;
    }
    return 1;
}
int av_Frame::copyFrom(av_Frame& f)
{
    if(mBufSize < f.DataLen())
    {
        reSizeBuf(f.DataLen());
    }
    if(mData && mBufSize >= f.DataLen())
    {
        memcpy(mData, f.Data(), f.DataLen());
        mDataSize = f.DataLen();
        mFrame = f.frameHead();
        return 0;
    }
    return -1;
}
const unsigned char* av_Frame::Data()
{
    if(mData)
    {
        return mData;
    }
    return NULL;
}
unsigned int av_Frame::BufLen()
{
    return mBufSize;
}
unsigned int av_Frame::DataLen()
{
    return mDataSize;
}
void av_Frame::SetCodecType(MEDIA_CODEC_TYPE t)
{
    mFrame.codect = t;
}
MEDIA_CODEC_TYPE av_Frame::GetCodecType()
{
    return (MEDIA_CODEC_TYPE)mFrame.codect;
}
bool av_Frame::IsIde()
{
    if(mDataSize > 0)
    {
        return false;
    }
    return true;
}
void av_Frame::ClearData()
{
    mDataSize = 0;
}
MediaAvFrame& av_Frame::frameHead()
{
    return mFrame;
}
int av_Frame::GetChls()
{
    if(mFrame.codect == CODEC_AAC)
    {
        return mFrame.audio.chl;
    }
    return 0;
}
void av_Frame::SetChls(int chl)
{
    if(mFrame.codect == CODEC_AAC)
    {
        mFrame.audio.chl = chl;
    }
}
int av_Frame::GetSampleRate()
{
    if(mFrame.codect == CODEC_AAC)
    {
        return mFrame.audio.sampleRate;
    }
    return 0;
}
void av_Frame::SetSampleRate(int sampleRate)
{
    if(mFrame.codect == CODEC_AAC)
    {
        mFrame.audio.sampleRate = sampleRate;
    }
}
unsigned int av_Frame::GetFramRate()
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        return mFrame.video.frameRate;
    }
    return 0;
}
void av_Frame::SetFrameRate(unsigned int fr)
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        mFrame.video.frameRate = fr;
    }
}
int av_Frame::GetIBPType()
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        return mFrame.video.frameType;
    }
    return -1;
}
void av_Frame::SetIBPType(int ibp)
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        mFrame.video.frameType = ibp;
    }
}
void av_Frame::SetWidHeight(unsigned int width, unsigned int height)
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        mFrame.video.vheight = height;
        mFrame.video.vwidth = width;
    }
}
bool av_Frame::GetWidHeight(unsigned int& width, unsigned int& height)
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        if(mFrame.video.frameType == 0)
        {
            width = mFrame.video.vwidth;
            height = mFrame.video.vheight;
            return true;
        }
    }
    return false;
}
uint64_t av_Frame::PTS()
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        return mFrame.video.timestamp;
    }
    return 0;
}
void av_Frame::SetPTS(uint64_t pts)
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        mFrame.video.timestamp = pts;
    }
}
bool av_Frame::IsVideo()
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        return true;
    }
    return false;
}
bool av_Frame::IsKeyFram()
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        if(mFrame.video.frameType == 0 || mFrame.video.frameType == 1)
        {
            return true;
        }
    }
    return false;
}
bool av_Frame::IsIDRFram()
{
    if(mFrame.codect == CODEC_H264 || mFrame.codect == CODEC_H265)
    {
        if(mFrame.video.frameType == 0)
        {
            return true;
        }
    }
    return false;
}
bool av_Frame::reSizeBuf(int size)
{
    if(IsIde())
    {
        if(mData)
        {
            LogOut(BLL, L_WARN, "av_Frame resize buf :%p, old mBufSize:%d new mBufSize:%d", mData, mBufSize, size);
            delete[] mData;
            mData = NULL;
        }
        mData = new uint8_t[size];
        if(mData)
        {
            mBufSize = size;
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}
void av_Frame::dereference()
{
    if(mData)
    {
        // LogOut(BLL, L_DEBUG, "av_Frame dereference buf :%p", mData);
        mData = NULL;
        mDataSize = mBufSize = 0;
    }
}