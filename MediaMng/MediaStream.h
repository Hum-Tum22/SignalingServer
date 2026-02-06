#ifndef MEDIA_STREAM_H_
#define MEDIA_STREAM_H_
#include <iostream>
#include <atomic>
#include <vector>
#include "avFrame.h"

using StreamHandle = unsigned long;
class MediaStream
{
public:
    using Ptr = std::shared_ptr<MediaStream>;
    MediaStream(const char* streamId);
    virtual ~MediaStream();

    virtual int getvalue() = 0;
    virtual time_t LastFrameTime() = 0;
    const std::string& getStreamId();

    // return 0:ok, 1:full, 2:error,3:write fail
    int OnJsonMediaVideoStream(MEDIA_CODEC_TYPE code, uint8_t* data, size_t size, ushort fps, ushort ibp, ushort w, ushort h, uint64_t pts);
    int OnJsonMediaAudioStream(MEDIA_CODEC_TYPE code, uint8_t* data, size_t size, int chl, int sampleRate);

    int GetNextFrame(uint32_t readhandle, av_Frame& frame, ushort& fIndex);
    void freeFrameByIndex(ushort fIndex);

    void increasing();
    void reduction();
    int refNum();
private:
    std::string streamId;
    std::atomic<int> def;
    ushort rIndex, wIndex;
    std::vector<av_Frame> mFrames;

    std::mutex clk;
    std::unordered_map<std::string, void*> publishs;
    std::thread publishThread;
};

#endif