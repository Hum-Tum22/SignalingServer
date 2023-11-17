#pragma once
#include "h264-file-reader.h"
#include "media-source.h"
#include "rtp/lib/mpeg/mpeg-ps.h"
//#include "sys/process.h"
//#include "time64.h"
#include "rtp/lib/rtp/rtp.h"
#include <string>
#include <thread>

//#include "mediaIn/JsonStream.h"
#include "MediaStream.h"

class PSFileSource : public IMediaSource
{
public:
	PSFileSource(const char* file, uint32_t ssrc);
	virtual ~PSFileSource();

public:
	void run();
	void SourceLoop();
	virtual int PlayEx();
	virtual int Play();
	virtual int Pause();
	virtual int Seek(int64_t pos);
	virtual int SetSpeed(double speed);
	virtual int GetDuration(int64_t& duration) const;
	virtual int GetSDPMedia(std::string& sdp) const;
	virtual int GetRTPInfo(const char* uri, char* rtpinfo, size_t bytes) const;
	virtual int SetTransport(const char* track, std::shared_ptr<IRTPTransport> transport);

	void Input(int datatype, const uint8_t* data, size_t size);
	int InputH264(const uint8_t* data, size_t bytes);

	void setMediaStream(MediaStream::Ptr);

private:
	static void* Alloc(void* param, size_t bytes);
	static void Free(void* param, void* packet);
	static int Packet(void* param, int avtype, void* packet, size_t bytes);
	static void* RTPAlloc(void* param, int bytes);
	static void RTPFree(void* param, void* packet);
	static int RTPPacket(void* param, const void* packet, int bytes, uint32_t timestamp, int flags);

	static void OnRTCPEvent(void* param, const struct rtcp_msg_t* msg);
	void OnRTCPEvent(const struct rtcp_msg_t* msg);
	int SendRTCP();

private:
	ps_muxer_t* m_ps;
	int m_ps_stream;
	void* m_rtp;
	int m_status;
	int64_t m_pos;
	double m_speed;
	unsigned short m_seq;

	uint64_t m_ps_clock;
	uint64_t m_rtp_clock;
	uint64_t m_rtcp_clock;
	H264FileReader m_reader;
	MediaStream::Ptr media;
	std::shared_ptr<IRTPTransport> m_transport;
	uint32_t readhandle;

	std::thread loop;
	bool IsRun;
	int playType;//1:‘§¿¿,2:ªÿ∑≈,3:œ¬‘ÿ

	void* m_pspacker;
	unsigned char m_packet[MAX_UDP_PACKET + 14];
};