#pragma once
#include "../MediaStream.h"
#include "../../deviceMng/JsonDevice.h"
#include <chrono>


class JsonStream : public MediaStream
{
	typedef enum
	{
		_VKDATA_OK,
		_VKDATA_NOTENOUGH,
		_VKDATA_DATAERROR,
		_VKDATA_LOSTDATA,//数据校验失败

	}VKDATDA_ERORCODE;

	typedef enum//编码类型
	{
		PT_MPEG2TS = 1,  // MPEG-2 Transport Stream	
		PT_QCELP,// QCELP audio
		PT_AMR,// AMR audio (narrowband)
		PT_AMR_WB,// AMR audio (wideband)
		PT_MPEGAUDIO,// MPEG-1 or 2 audio
		PT_MP4AUDIO,// MPEG-4 LATM audio
		PT_PCM8,// PCM u-law audio
		PT_AC3,// AC3 audio
		PT_MPEG4V_ES,// MPEG-4 ES
		PT_MPVIDEO,// MPEG-1 or 2 video
		PT_PCMA = 14,		// VSK_G711A
		PT_AAC = 15,		// VSK_AAC
		PT_G711A = 19,
		PT_G711U = 20,
		PT_PCMU = 22,		// VSK_G711U
		PT_AUDIO_INVALID,
		PT_H261,// H.261
		PT_H263,// H.263
		PT_H264 = 26,// H.264 标准视频帧类型
		PT_MJPEG,// motion JPEG
		PT_G726_16,// G.726, 16 kbps
		PT_G726_24,// G.726, 24 kbps
		PT_G726_32,// G.726, 32 kbps
		PT_G726_40,// G.726, 40 kbps
		PT_VSK_H264 = 32,			// 包含arm nvr头的 帧数据  h264, h265 音频
		PT_PSTREAM,// 33;
		PS_TSTREAM,
		PT_H265 = 35,// H.265 标准视频帧类型
		PT_VSK_DATA = 36,			// 包含x86 nvr头的 帧数据

		PT_H264_OR_H265 = 37,		// 解码库使用:  自动判断 PT_H264 或者 PT_H265, 默认为PT_H264
		PT_UNKNOW,

		PT_VSK_POINT_CLOUD_XYZIT = 39,			// 包含x86 nvr头的 点云数据 SPointTypeXYZITag, 默认需要压缩, 压缩后变为PT_VSK_POINT_CLOUD_DRACO
		PT_VSK_POINT_CLOUD_DRACO = 40,			// 包含x86 nvr头的 点云数据, 使用draco算法压缩,不用再次压缩
		PT_VSK_POINT_CLOUD_ANALYZE = 41,		// 包含x86 nvr头的 点云分析帧综合压缩数据, 不用再次压缩.
		PT_VSK_POINT_CLOUD_ANALYZE_TLV = 42,	// 包含x86 nvr头的 点云分析帧综合压缩数据, tlv格式

		PT_VSK_POINT_CLOUD_MAX = 50,	// 为点云数据 预留10个压缩类型

	}PAYLOAD_TYPE_E;

	/*the nalu type of H264E*///H264的帧的类型，i帧，p帧等
	typedef enum
	{
		H264E_NALU_PSLICE = 1, /*PSLICE types*/
		H264E_NALU_ISLICE = 5, /*ISLICE types*/
		H264E_NALU_SEI = 6, /*SEI types*/
		H264E_NALU_SPS = 7, /*SPS types*/
		H264E_NALU_PPS = 8, /*PPS types*/
		H264E_NALU_BUTT = 9,
		H264E_NALU_UNKNOW = 0x00
	} H264E_NALU_TYPE_E;

	/*the nalu type of H265*///H265的帧的类型，i帧，p帧等
	typedef enum
	{
		H265_NALU_PSLICE = 0,       /*PSLICE types*/
		H265_NALU_PSLICE_MAX = 18,  /*PSLICE types*/
		H265_NALU_ISLICE = 19,      /*ISLICE types*/
		H265_NALU_ISLICE_MAX = 20,  /*ISLICE types*/
		H265_NALU_VPS = 32, /*VPS types*/
		H265_NALU_SPS = 33, /*SPS types*/
		H265_NALU_PPS = 34, /*PPS types*/
		H265_NALU_SEI = 39, /*SEI types*/
		H265_NALU_SEI_MAX = 40, /*SEI types*/
		H265_NALU_MAX
	} H265_NALU_TYPE;
	struct audioparam
	{
		int chl; //1 单声道，2双声道，实际写入buf 里面的为char
		unsigned int frate;//采样率
	};
	struct KeyFrameparam
	{
		unsigned int vwidth;//视频宽 
		unsigned int vheight;//视频高
		uint64_t     timestamp;//时间戳 utc 时间，关键帧才有
	};

	//  x86 nvr 帧头格式
	struct DataHeard
	{
		unsigned short codect;//编码类型	PAYLOAD_TYPE_E
		unsigned short framet;//帧类型  h264:H264E_NALU_TYPE_E, h265:H265_NALU_TYPE
		unsigned short gapms;	//帧间距 默认 40
		unsigned short framerate;//平均帧率 默认25
		unsigned int framelen; //帧长度 不包括 格式封装的结束符的长度
		union
		{
			audioparam audio;
			KeyFrameparam KeyFramE;
		};

		void Clear();
		bool IsVideo();
		bool IsPointCloud();	// 是否为点云数据

		bool IsKeyFram();
		bool IsPFram();

		bool IsKeyFramStart();

		unsigned int GetHeaderLen();
		unsigned int GetFrameBufSize();	// header+ body 长度
	};
	int PraseFrameHeard(unsigned char* pbuf, unsigned int buflen, DataHeard* pheard/*out*/, unsigned int* skipsize/*out*/);
	int IskeyFrame(DataHeard& heard);
	unsigned int  basicHeardLen();
	//根据帧类型判断 帧heard 长度,数据buf 里面的长度 不是 sizeof(DataHeard)
	unsigned int FrameHeardLen(unsigned short codectype, unsigned short frametype);
	int MatchDataHeard(unsigned char* pbuf, unsigned int dwSize);
	int ReadBasicHeardFormBuf(unsigned char* pbuf, unsigned int pbufsize, DataHeard* pheard);
	STREAM_CODEC switchFromToGB(int type);
public:
	JsonStream(const char* devId, const char* streamId);
	virtual ~JsonStream();

	int getvalue() { return 0; }
	time_t LastFrameTime();
	static void CALLBACK VskX86NvrRtPreDataCb(unsigned int PlayHandle, uint8_t* pBuffer, unsigned int BufferSize, unsigned int DateType, time_t systime, unsigned int TimeSpace, void *pUser);
	static void CALLBACK DataPlayCallBack(unsigned int PlayHandle, unsigned int DateType, uint8_t* pBuffer, unsigned int BufferSize, void* pUser);
	static void CALLBACK PlayBackEndCb(unsigned int pbhandle, int errorcode, void* puser);
	void OnVskJsonStream(uint8_t* data, size_t size);
private:
	LineBuffer mFrame;
	time_t lastTime, curTime;
	uint64_t nFrameNum;
	uint32_t frameRate;
	std::chrono::time_point<std::chrono::steady_clock> firstTime;
	std::chrono::time_point<std::chrono::steady_clock> latestTime;
};