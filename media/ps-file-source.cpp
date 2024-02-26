#include "ps-file-source.h"
//#include "cstringext.h"
#include "rtp/lib/rtp/rtp-profile.h"
#include "rtp/lib/rtp/rtp-payload.h"
#include <assert.h>
#include <chrono>
#include <arpa/inet.h>

//extern "C" uint32_t rtp_ssrc(void);


PSFileSource::PSFileSource(const char* file, uint32_t ssrc)
	:m_reader(file), IsRun(false), playType(1), media(NULL), readhandle(0), m_pspacker(NULL),  m_pos(0), m_seq(0), mGap(0), frameNum(0), nSsrc(ssrc)
{
	m_speed = 1.0;
	m_status = 0;
	m_ps_clock = 0;
	m_rtp_clock = 0;
	m_rtcp_clock = 0;

	//uint32_t ssrc = rtp_ssrc();
	uint32_t lssrc = ssrc;
	//if (BYTE_ORDER == LITTLE_ENDIAN)
	{
		//lssrc = htonl(ssrc);
	}

	struct ps_muxer_func_t func;
	func.alloc = Alloc;
	func.free = Free;
	func.write = Packet;
	m_ps = ps_muxer_create(&func, this);
	m_ps_stream = ps_muxer_add_stream(m_ps, PSI_STREAM_H264, NULL, 0);

	static struct rtp_payload_t s_psfunc = {
		PSFileSource::RTPAlloc,
		PSFileSource::RTPFree,
		PSFileSource::RTPPacket,
	};
	m_pspacker = rtp_payload_encode_create(RTP_PAYLOAD_MP2P, "MP2P", (uint16_t)lssrc, lssrc, &s_psfunc, this);

	struct rtp_event_t event;
	event.on_rtcp = OnRTCPEvent;
	m_rtp = rtp_create(&event, this, lssrc, lssrc, 90000, 4 * 1024, 1);
	rtp_set_info(m_rtp, "RTSPServer", "szj.h264");
}

PSFileSource::~PSFileSource()
{

	IsRun = false;
	if (loop.joinable())
	{
		loop.join();
	}
	printf("--- transport end PSFileSource\n");
	if (media)
	{
		media->removeReader(readhandle);
	}
	if (m_rtp)
		rtp_destroy(m_rtp);
	if (m_pspacker)
		rtp_payload_encode_destroy(m_pspacker);
	ps_muxer_destroy(m_ps);
	printf("--- delete PSFileSource\n");
}
void PSFileSource::run()
{
	IsRun = true;
	loop = std::thread(&PSFileSource::SourceLoop, this);
	printf("--- transport start PSFileSource\n");
	
}
void PSFileSource::SourceLoop()
{
	while (IsRun)
	{
		Play();
		m_transport->run();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}
int PSFileSource::SetTransport(const char* /*track*/, std::shared_ptr<IRTPTransport> transport)
{
	m_transport = transport;
	return 0;
}
void PSFileSource::Input(int datatype, const uint8_t* data, size_t size)
{
	if (datatype == 0)
	{
		//InputH264(data, size);
		m_reader.Input(data, size);
		m_reader.GetParameterSets();
	}

}
int PSFileSource::PlayEx()
{
	m_status = 1;

	//time64_t clock = time64_now();
	int64_t clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if (0 == m_rtp_clock || m_rtp_clock + 40 < (clock - m_ps_clock))
	{
		size_t bytes;
		const uint8_t* ptr;
		if (0 == m_reader.GetNextFrameEx(m_pos, ptr, bytes))
		{
			if (0 == m_ps_clock)
				m_ps_clock = clock;
			ps_muxer_input(m_ps, m_ps_stream, 0, (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, ptr, bytes);
			m_rtp_clock += 40;

			SendRTCP();
			return 1;
		}
	}

	return 0;
}
int PSFileSource::Play()
{
	m_status = 1;

	//time64_t clock = time64_now();
	int64_t clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if (0 == m_rtp_clock || m_rtp_clock + mGap < (clock - m_ps_clock))
	{
		vframe_t frame;
		if (media && 0 == media->GetNextFrame(readhandle, frame))
		{
			//printf("--------------- get frame time:%lld\n", time(0));
			if (!frame.nalu)
				return 0;
			if (0 == m_ps_clock)
				m_ps_clock = clock;
			if (frame.nalu && frame.nalu[0] != 0x00)
			{
				printf("data err %x,%x,%x,%x,nalu:%d\n", frame.nalu[0], frame.nalu[1], frame.nalu[2], frame.nalu[3], (frame.nalu[4] & 0x1F));
				return 0;
			}
			if (frame.frameType == GB_CODEC_H264 || frame.frameType == GB_CODEC_H265)
			{
				frameNum++;
			}
			if (frame.idr)
			{
				mGap = frame.gap;
				//printf("--------------- idr frame time:%ld, gap:%d, frameNum:%ju, ssrc:%u\n", time(0), mGap, frameNum, nSsrc);
			}
			ps_muxer_input(m_ps, m_ps_stream, 0, (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, frame.nalu, frame.bytes);
			m_rtp_clock += 40;

			SendRTCP();
			return 1;
		}
	}

	return 0;
}
int PSFileSource::InputH264(const uint8_t* data, size_t bytes)
{
	m_status = 1;

	//time64_t clock = time64_now();
	int64_t clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if (0 == m_rtp_clock || m_rtp_clock + 40 < (clock - m_ps_clock))
	{
		if (0 == m_ps_clock)
			m_ps_clock = clock;
		ps_muxer_input(m_ps, m_ps_stream, 0, (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, data, bytes);
		m_rtp_clock += 40;

		SendRTCP();
		return 1;
	}

	return 0;
}
void PSFileSource::setMediaStream(MediaStream::Ptr p)
{
	media = p;
	readhandle = p->createReader();
}
int PSFileSource::Pause()
{
	m_status = 2;
	m_rtp_clock = 0;
	return 0;
}

int PSFileSource::Seek(int64_t pos)
{
	m_rtp_clock = 0;
	return m_reader.Seek(pos);
}

int PSFileSource::SetSpeed(double speed)
{
	m_speed = speed;
	return 0;
}

int PSFileSource::GetDuration(int64_t& duration) const
{
	return m_reader.GetDuration(duration);
}

int PSFileSource::GetSDPMedia(std::string& sdp) const
{
	static const char* pattern =
		"m=video 0 RTP/AVP %d\n"
		"a=rtpmap:%d MP2P/90000\n";

	char media[64];
	snprintf(media, sizeof(media), pattern, RTP_PAYLOAD_MP2P, RTP_PAYLOAD_MP2P);
	sdp = media;
	return 0;
}

int PSFileSource::GetRTPInfo(const char* uri, char* rtpinfo, size_t bytes) const
{
	uint16_t seq;
	uint32_t timestamp;
	rtp_payload_encode_getinfo(m_pspacker, &seq, &timestamp);

	// url=rtsp://video.example.com/twister/video;seq=12312232;rtptime=78712811
	snprintf(rtpinfo, bytes, "url=%s;seq=%hu;rtptime=%u", uri, seq, timestamp);
	return 0;
}

void PSFileSource::OnRTCPEvent(const struct rtcp_msg_t* msg)
{
	msg;
}

void PSFileSource::OnRTCPEvent(void* param, const struct rtcp_msg_t* msg)
{
	PSFileSource* self = (PSFileSource*)param;
	self->OnRTCPEvent(msg);
}

int PSFileSource::SendRTCP()
{
	// make sure have sent RTP packet

	//uint64_t clock = time64_now();
	int64_t clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	int interval = rtp_rtcp_interval(m_rtp);
	if (0 == m_rtcp_clock || m_rtcp_clock + interval < clock)
	{
		char rtcp[1024] = { 0 };
		size_t n = rtp_rtcp_report(m_rtp, rtcp, sizeof(rtcp));

		// send RTCP packet
		m_transport->Send(true, rtcp, n);

		m_rtcp_clock = clock;
	}

	return 0;
}

void* PSFileSource::Alloc(void* /*param*/, size_t bytes)
{
	//	PSFileSource* self = (PSFileSource*)param;
	return malloc(bytes);
}

void PSFileSource::Free(void* /*param*/, void* packet)
{
	//	PSFileSource* self = (PSFileSource*)param;
	return free(packet);
}
int PSFileSource::Packet(void* param, int /*avtype*/, void* pes, size_t bytes)
{
	PSFileSource* self = (PSFileSource*)param;
	int64_t clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return rtp_payload_encode_input(self->m_pspacker, pes, (int)bytes, (uint32_t)(clock * 90 /*kHz*/));
}

void* PSFileSource::RTPAlloc(void* param, int bytes)
{
	PSFileSource* self = (PSFileSource*)param;
	assert(bytes <= sizeof(self->m_packet));
	return self->m_packet;
}

void PSFileSource::RTPFree(void* param, void* packet)
{
	PSFileSource* self = (PSFileSource*)param;
	assert(self->m_packet == packet);
}

int PSFileSource::RTPPacket(void* param, const void* packet, int bytes, uint32_t /*timestamp*/, int /*flags*/)
{
	PSFileSource* self = (PSFileSource*)param;
	assert(self->m_packet == packet);

	/*const char* rtpPacket = (const char*)packet;
	printf("rtp packet:");
	for (int i = 0; i < 12; i++)
	{
		printf("%02x ", rtpPacket[i]);
	}
	printf("\n");*/
	//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	int r = self->m_transport->Send(false, packet, bytes);
	if (r != bytes)
	{
		return -1;
	}

	return rtp_onsend(self->m_rtp, packet, bytes/*, time*/);
}
