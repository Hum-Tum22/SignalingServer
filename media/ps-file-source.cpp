#include "ps-file-source.h"
#include "videoNalu.hpp"
//#include "cstringext.h"
#include "SelfLog.h"
#include "rtp/lib/rtp/rtp-profile.h"
#include "rtp/lib/rtp/rtp-payload.h"
#include <assert.h>
#include <chrono>
#include <arpa/inet.h>

//extern "C" uint32_t rtp_ssrc(void);


PSFileSource::PSFileSource(const char* file, uint32_t ssrc)
    :m_reader(file), m_h265Reader(file), m_codecType(CodecType::H264), IsRun(false), playType(1), media(NULL), readhandle(0), m_pspacker(NULL), m_pos(0), m_seq(0), mGap(0), frameNum(0), nSsrc(ssrc), m_ps_video_stream(0), m_ps_video_stream_inited(false)
{
    m_speed = 1.0;
    m_status = 0;
    m_ps_clock = 0;
    m_rtp_clock = 0;
    m_rtcp_clock = 0;

    uint32_t lssrc = ssrc;

    struct ps_muxer_func_t func;
    func.alloc = Alloc;
    func.free = Free;
    func.write = Packet;
    m_ps = ps_muxer_create(&func, this);

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
    LogOut(BLL, L_DEBUG, "--- transport end PSFileSource");
    if (media)
    {
        media->removeReader(readhandle);
        LogOut(BLL, L_INFO, "PSFileSource::~PSFileSource: streamId=%s, use_count=%ld", media->getStreamId().c_str(), media.use_count());
    }
    if (m_rtp)
        rtp_destroy(m_rtp);
    if (m_pspacker)
        rtp_payload_encode_destroy(m_pspacker);
    ps_muxer_destroy(m_ps);
    LogOut(BLL, L_DEBUG, "--- delete PSFileSource");
}
void PSFileSource::run()
{
    IsRun = true;
    loop = std::thread(&PSFileSource::SourceLoop, this);
    LogOut(BLL, L_DEBUG, "--- transport start PSFileSource");

}
void PSFileSource::runPlayback()
{
    IsRun = true;
    loop = std::thread(&PSFileSource::SourceLoopPlayBack, this);
    LogOut(BLL, L_DEBUG, "--- transport start PSFileSource");

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
void PSFileSource::SourceLoopPlayBack()
{
    while (IsRun)
    {
        Playback();
        m_transport->run();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
int PSFileSource::SetTransport(const char * /*track*/, std::shared_ptr<IRTPTransport> transport)
{
    m_transport = transport;
    return 0;
}
void PSFileSource::Input(int datatype, const uint8_t* data, size_t size)
{
    LogOut(BLL, L_DEBUG, "PSFileSource::Input: datatype=%d, size=%zu", datatype, size);
    if (datatype == 0)
    {
        if (!m_ps_video_stream_inited && size > 0)
        {
            int nal_type = -1;
            if (data[0] == 0x00 && data[1] == 0x00)
            {
                nal_type = h264_nal_type(data);
                if (nal_type >= 0 && nal_type <= 31)
                {
                    m_codecType = CodecType::H264;
                    LogOut(BLL, L_INFO, "PSFileSource::Input: detected H264 codec, nal_type=%d", nal_type);
                }
                else
                {
                    nal_type = h265_nal_type(data);
                    m_codecType = CodecType::H265;
                    LogOut(BLL, L_INFO, "PSFileSource::Input: detected H265 codec, nal_type=%d", nal_type);
                }
            }
            
            int ps_stream_type = (m_codecType == CodecType::H265) ? PSI_STREAM_H265 : PSI_STREAM_H264;
            m_ps_video_stream = ps_muxer_add_stream(m_ps, ps_stream_type, NULL, 0);
            m_ps_video_stream_inited = true;
            LogOut(BLL, L_INFO, "PSFileSource::Input: initialized PS stream, codecType=%d, stream_type=%d", (int)m_codecType, ps_stream_type);
        }

        if (m_codecType == CodecType::H265)
        {
            LogOut(BLL, L_DEBUG, "PSFileSource::Input: processing H265 data, size=%zu", size);
            // m_h265Reader.Input(data, size);
            // m_h265Reader.GetParameterSets();
        }
        else
        {
            LogOut(BLL, L_DEBUG, "PSFileSource::Input: processing H264 data, size=%zu", size);
            // m_reader.Input(data, size);
            // m_reader.GetParameterSets();
        }
    }

}
int PSFileSource::PlayEx()
{
    LogOut(BLL, L_DEBUG, "PSFileSource::PlayEx: start");
    m_status = 1;

    //time64_t clock = time64_now();
    int64_t clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (0 == m_rtp_clock || m_rtp_clock + 40 < (clock - m_ps_clock))
    {
        size_t bytes;
        const uint8_t* ptr;
        int ret = -1;
        if (m_codecType == CodecType::H265)
        {
            LogOut(BLL, L_DEBUG, "PSFileSource::PlayEx: getting H265 frame, pos=%zu", m_pos);
            ret = m_h265Reader.GetNextFrameEx(m_pos, ptr, bytes);
        }
        else
        {
            LogOut(BLL, L_DEBUG, "PSFileSource::PlayEx: getting H264 frame, pos=%zu", m_pos);
            ret = m_reader.GetNextFrameEx(m_pos, ptr, bytes);
        }
        
        if (ret == 0)
        {
            LogOut(BLL, L_DEBUG, "PSFileSource::PlayEx: got frame, bytes=%zu, codecType=%d", bytes, (int)m_codecType);
            if (0 == m_ps_clock)
                m_ps_clock = clock;
            LogOut(BLL, L_DEBUG, "PSFileSource::PlayEx: muxing frame, pts=%ju, dts=%ju, bytes=%zu", (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, bytes);
            if(!m_ps_video_stream_inited)
            {
                Input(0, ptr, bytes);
            }
            ps_muxer_input(m_ps, m_ps_video_stream, 0, (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, ptr, bytes);
            m_rtp_clock += 40;
            m_pos += bytes;

            SendRTCP();
            LogOut(BLL, L_DEBUG, "PSFileSource::PlayEx: sent frame, returning 1");
            return 1;
        }
        else
        {
            LogOut(BLL, L_DEBUG, "PSFileSource::PlayEx: no frame available, ret=%d", ret);
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
            // LogOut(BLL, L_DEBUG, "PSFileSource::Play: got frame, frameType=%d, bytes=%zu, idr=%d", frame.frameType, frame.bytes, frame.idr);
            if (!frame.nalu)
            {
                LogOut(BLL, L_WARN, "PSFileSource::Play: frame nalu is NULL");
                return 0;
            }
            if (0 == m_ps_clock)
                m_ps_clock = clock;
            if (frame.nalu && frame.nalu[0] != 0x00)
            {
                LogOut(BLL, L_ERROR, "data err %x,%x,%x,%x,nalu:%d", frame.nalu[0], frame.nalu[1], frame.nalu[2], frame.nalu[3], (frame.nalu[4] & 0x1F));
                return 0;
            }
            if (frame.frameType == GB_CODEC_H264 || frame.frameType == GB_CODEC_H265)
            {
                frameNum++;
                // LogOut(BLL, L_DEBUG, "PSFileSource::Play: frameNum=%ju, codecType=%d", frameNum, frame.frameType);
                if(!m_ps_video_stream_inited)
                {
                    int ps_stream_type = PSI_STREAM_H264;
                    if(frame.frameType == GB_CODEC_H264)
                    {
                        ps_stream_type = PSI_STREAM_H264;
                    }
                    else if(frame.frameType == GB_CODEC_H265)
                    {
                        ps_stream_type = PSI_STREAM_H265;
                    }
                    m_ps_video_stream = ps_muxer_add_stream(m_ps, ps_stream_type, NULL, 0);
                    m_ps_video_stream_inited = true;
                }
                ps_muxer_input(m_ps, m_ps_video_stream, 0, (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, frame.nalu, frame.bytes);
                m_rtp_clock += 40; 
            }
            else
            {
                if(!m_ps_audio_stream_inited)
                {
                    int ps_stream_type = PSI_STREAM_AUDIO_G711A;
                    if(frame.frameType == CODEC_G711A)
                    {
                        ps_stream_type = PSI_STREAM_AUDIO_G711A;
                    }
                    else if(frame.frameType == CODEC_G711U)
                    {
                        ps_stream_type = PSI_STREAM_AUDIO_G711U;
                    }
                    else if(frame.frameType == CODEC_AAC)
                    {
                        ps_stream_type = PSI_STREAM_AAC;
                    }
                    else
                    {
                        return 0;
                    }
                    m_ps_audio_stream = ps_muxer_add_stream(m_ps, ps_stream_type, NULL, 0);
                    m_ps_audio_stream_inited = true;
                }
                if(frame.frameType == CODEC_G711A || frame.frameType == CODEC_AAC || frame.frameType == CODEC_G711U)
                {
                    ps_muxer_input(m_ps, m_ps_audio_stream, 0, 0, 0, frame.nalu, frame.bytes);
                }
            }
            if (frame.idr)
            {
                mGap = frame.gap;
                // LogOut(BLL, L_INFO, "PSFileSource::Play: IDR frame, gap=%d, frameNum=%ju, ssrc=%u", mGap, frameNum, nSsrc);
            }
            // LogOut(BLL, L_DEBUG, "PSFileSource::Play: muxing frame, pts=%ju, dts=%ju, bytes=%zu m_ps:%p, m_ps_video_stream:%d", (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, frame.bytes, m_ps, m_ps_video_stream);
            
            SendRTCP();
            // LogOut(BLL, L_DEBUG, "PSFileSource::Play: sent frame, returning 1");
            return 1;
        }
    }

    return 0;
}
int PSFileSource::Playback()
{
    m_status = 1;

    //time64_t clock = time64_now();
    int64_t clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (0 == m_rtp_clock || m_rtp_clock + mGap < (clock - m_ps_clock))
    {
        av_Frame frame;
        ushort index = 0;
        if(media && 0 == media->GetNextFrameEx(readhandle, frame, index))
        {
            // LogOut(BLL, L_DEBUG, "PSFileSource::Playback: got frame, index=%hu, codecType=%d, dataLen=%d, isKeyFrame=%d", index, frame.GetCodecType(), frame.DataLen(), frame.IsKeyFram());
            if (0 == m_ps_clock)
                m_ps_clock = clock;
            if(frame.IsVideo())
            {
                if(!m_ps_video_stream_inited)
                {
                    int ps_stream_type = PSI_STREAM_H264;
                    if(frame.GetCodecType() == CODEC_H264)
                    {
                        ps_stream_type = PSI_STREAM_H264;
                    }
                    else if(frame.GetCodecType() == CODEC_H265)
                    {
                        ps_stream_type = PSI_STREAM_H265;
                    }
                    m_ps_video_stream = ps_muxer_add_stream(m_ps, ps_stream_type, NULL, 0);
                    m_ps_video_stream_inited = true;
                }
                ps_muxer_input(m_ps, m_ps_video_stream, 0, (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, frame.Data(), frame.DataLen());
                m_rtp_clock += 40;
            }
            else
            {
                if(!m_ps_audio_stream_inited)
                {
                    int ps_stream_type = PSI_STREAM_AUDIO_G711A;
                    if(frame.GetCodecType() == CODEC_G711A)
                    {
                        ps_stream_type = PSI_STREAM_AUDIO_G711A;
                    }
                    else if(frame.GetCodecType() == CODEC_G711U)
                    {
                        ps_stream_type = PSI_STREAM_AUDIO_G711U;
                    }
                    else if(frame.GetCodecType() == CODEC_AAC)
                    {
                        ps_stream_type = PSI_STREAM_AAC;
                    }
                    else
                    {
                        frame.dereference();
                        media->freeFrameByIndex(index);
                        return 0;
                    }
                    m_ps_audio_stream = ps_muxer_add_stream(m_ps, ps_stream_type, NULL, 0);
                    m_ps_audio_stream_inited = true;
                }
                if(frame.GetCodecType() == CODEC_G711A || frame.GetCodecType() == CODEC_AAC || frame.GetCodecType() == CODEC_G711U)
                {
                    ps_muxer_input(m_ps, m_ps_audio_stream, 0, 0, 0, frame.Data(), frame.DataLen());
                }
            }
            if (frame.GetCodecType() == CODEC_H264 || frame.GetCodecType() == CODEC_H265)
            {
                frameNum++;
                // LogOut(BLL, L_DEBUG, "PSFileSource::Playback: frameNum=%ju, codecType=%d", frameNum, frame.GetCodecType());
            }
            if (frame.IsKeyFram())
            {
                if(frame.GetFramRate() > 0)
                {
                    mGap = 1000 / frame.GetFramRate();
                }
                else
                {
                    mGap = 40;
                }
                // LogOut(BLL, L_INFO, "PSFileSource::Playback: key frame, gap=%d, frameNum=%ju, ssrc=%u, frameRate=%d", mGap, frameNum, nSsrc, frame.GetFramRate());
            }
            // LogOut(BLL, L_DEBUG, "PSFileSource::Playback: muxing frame, pts=%ju, dts=%ju, bytes=%d", (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, frame.DataLen());
            frame.dereference();
            media->freeFrameByIndex(index);

            SendRTCP();
            // LogOut(BLL, L_DEBUG, "PSFileSource::Playback: sent frame, returning 1");
            return 1;
        }
    }

    return 0;
}
int PSFileSource::InputH264(const uint8_t *data, size_t bytes)
{
    LogOut(BLL, L_DEBUG, "PSFileSource::InputH264: start, bytes=%zu", bytes);
    m_status = 1;

    //time64_t clock = time64_now();
    int64_t clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (0 == m_rtp_clock || m_rtp_clock + 40 < (clock - m_ps_clock))
    {
        if (0 == m_ps_clock)
            m_ps_clock = clock;
        LogOut(BLL, L_DEBUG, "PSFileSource::InputH264: muxing frame, pts=%ju, dts=%ju, bytes=%zu", (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, bytes);
        ps_muxer_input(m_ps, m_ps_video_stream, 0, (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, data, bytes);
        m_rtp_clock += 40;

        SendRTCP();
        LogOut(BLL, L_DEBUG, "PSFileSource::InputH264: sent frame, returning 1");
        return 1;
    }

    return 0;
}
void PSFileSource::setMediaStream(MediaStream::Ptr p)
{
    media = p;
    readhandle = p->createReader();
    LogOut(BLL, L_INFO, "PSFileSource setMediaStream: streamId=%s, use_count=%ld", p->getStreamId().c_str(), p.use_count());
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
    if (m_codecType == CodecType::H265)
    {
        return m_h265Reader.Seek(pos);
    }
    return m_reader.Seek(pos);
}

int PSFileSource::SetSpeed(double speed)
{
    m_speed = speed;
    return 0;
}

int PSFileSource::GetDuration(int64_t& duration) const
{
    if (m_codecType == CodecType::H265)
    {
        return m_h265Reader.GetDuration(duration);
    }
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
    // LogOut(BLL, L_INFO, "PSFileSource Packet: bytes:%zu", bytes);
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
    // LogOut(BLL, L_INFO, "PSFileSource RTPPacket: bytes:%d", bytes);
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
