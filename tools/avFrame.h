#ifndef AV_FRAME_H_
#define AV_FRAME_H_
#include <stdint.h>

typedef enum//编码类型
{
    CODEC_UNKNOW,
    CODEC_H264,
    CODEC_H265,
    CODEC_G711A,
    CODEC_G711U,
    CODEC_PCMA,
    CODEC_PCMU,
    CODEC_AAC
}MEDIA_CODEC_TYPE;
struct _videoInfo
{
    unsigned short frameRate;
    unsigned short frameType;//帧类型 IDR、I、B、P:0、1、2、3 
    unsigned short vwidth;//视频宽
    unsigned short vheight;//视频高
    unsigned long long     timestamp;//时间戳 utc 时间，关键帧才有
};
struct _audioInfo
{
    int chl; //1 单声道，2双声道，实际写入buf 里面的为char
    unsigned int sampleRate;//采样率
};
typedef struct _MediaFrame
{
    // unsigned char  HEADCODE[4];//0x00 0x86 0x83 0x75
    unsigned short codect; //编码类型MEDIA_CODEC_TYPE :264|265|aac|g711|pcm
    unsigned int   framelen;
    union
    {
        _videoInfo video;
        _audioInfo audio;
    };
}MediaAvFrame;
#define FRAME_HEARD_LEN sizeof(MediaAvFrame)
class av_Frame
{
    uint8_t* mData;
    int mDataSize;
    int mBufSize;
    MediaAvFrame mFrame;
public:
    av_Frame(int size = 0);
    ~av_Frame();
    // return 0:ok, 1:need buf, 2:error
    int writeData(uint8_t* data, int size);
    int copyFrom(av_Frame& f);

    void ReSetData();
    const unsigned char* GetDataEsBuf(unsigned int& len);
    bool CPBuf(const unsigned char* buf, unsigned int l);

    const unsigned char* Data();
    unsigned int BufLen();
    unsigned int DataLen();

    void  SetCodecType(MEDIA_CODEC_TYPE t);
    MEDIA_CODEC_TYPE GetCodecType();
    bool IsIde();
    void ClearData();
    MediaAvFrame& frameHead();

    // audio
    int GetChls();
    void SetChls(int chl);
    int GetSampleRate();
    void SetSampleRate(int sampleRate);
    // video
    unsigned int GetFramRate();
    void SetFrameRate(unsigned int fr);
    int GetIBPType();
    void SetIBPType(int ibp);
    void SetWidHeight(unsigned int width, unsigned int height);
    bool GetWidHeight(unsigned int& width, unsigned int& height);
    uint64_t PTS();
    void SetPTS(uint64_t pts);
    bool IsVideo();
    bool IsKeyFram();
    bool IsIDRFram();
    // bool IsSPSFram();
    // bool IsPPSFram();
    // bool IsVPSFram();
    bool reSizeBuf(int size);
    //解引用
    void dereference();
};
#endif