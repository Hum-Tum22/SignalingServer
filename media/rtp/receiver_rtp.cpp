

#include "receiver_rtp.h"

#include "lib/mpeg/mpeg-types.h"    
#include "SelfLog.h"
#include <iostream>
#include <map>
#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp  strnicmp 
#endif

rtp_receiver::rtp_receiver(short localport, const char* peerIp, short peerPort) :rtp_socket_(io_service, udp::endpoint(udp::v4(), localport))
, rtcp_socket_(io_service, udp::endpoint(udp::v4(), localport + 1))
, to_endpoint_(asio::ip::address_v4::from_string(peerIp), peerPort + 1), demuxer(NULL), psdemuxer(NULL), p_psdata_(NULL)
, psdata_length(0)
{
    memset(rtp_buffer, 0, max_length);
    memset(rtcp_buffer, 0, max_length);
    //asio::async_read_until(s, messageBuffer, delim,
    //    [this](std::error_code ec, std::size_t length)
    //    {
    //        if (!ec)
    //        {
    //            size_t len1 = messageBuffer.size();
    //            ParseRTSPDescribeResponseHeader();
    //            size_t len = messageBuffer.size();
    //            size_t maxlen = messageBuffer.max_size();
    //            //SendRTSPSetupMessage();
    //            //messageBuffer.consume(length);
    //            std::size_t bufferLength = asio::buffer_size(messageBuffer.data());
    //            RecvRTSPSdpResponse((int)(sdpLen - bufferLength));
    //        }
    //        else
    //        {
    //            s.close();
    //        }
    //    });
}
rtp_receiver::~rtp_receiver()
{
}
void rtp_receiver::run()
{
    io_service.run();
}
void rtp_receiver::on_receive_rtp_from()
{
    on_send_rtcp_to();
    rtp_socket_.async_receive_from(asio::buffer(rtp_buffer, max_length), sender_endpoint_,
        [this](std::error_code ec, std::size_t length)
        {
            if(!ec)
            {
                //
                parseRtpData(rtp_buffer, length);
                on_receive_rtp_from();
            }
            else
            {
                rtp_socket_.close();
            }
        }
    );

    /*asio::async_read(s, asio::buffer(ctx->rtp_buffer, rtpPacketLen),
        [this](std::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                if (length < rtpPacketLen)
                {
                    std::cout << "data error" << std::endl;
                }
                else
                {
                    const unsigned char* buffer = (const unsigned char*)ctx->rtp_buffer;
                    clientRTP.ProcessRTPPacket(ctx, buffer, length);
                    RecvTCPInterleavedFrame();
                }
            }
        }
    );*/
}
void rtp_receiver::on_receive_rtcp_from()
{
    rtcp_socket_.async_receive_from(asio::buffer(rtcp_buffer, max_length), sender_endpoint_,
        [this](std::error_code ec, std::size_t length)
        {
            if(!ec)
            {
                //
                on_send_rtcp_to();
                on_receive_rtcp_from();
            }
            else
            {
                rtp_socket_.close();
            }
        }
    );
}
void rtp_receiver::on_send_rtcp_to()
{
    size_t r = rtp_demuxer_rtcp(demuxer, &rtcp_buffer[0], sizeof(rtcp_buffer));
    if(r > 0)
    {
        rtcp_socket_.async_send_to(asio::buffer(rtcp_buffer, r), to_endpoint_,
            [this](const asio::error_code& error, std::size_t bytes_transferred)
            {
                if(!error)
                {

                }
                else
                {
                    rtcp_socket_.close();
                }
            });
    }
}
void rtp_receiver::parseRtpData(const uint8_t* data, size_t length)
{
    if(!demuxer || length < 12)
    {
        return;
    }
    int r = rtp_demuxer_input(demuxer, data, length);
    if(r < 0)
    {
        std::cout << "sn err" << std::endl;
    }
}
void rtp_receiver::parseRtcpData(const uint8_t* data, size_t length)
{
    if(!demuxer || length < 12)
    {
        return;
    }
    int r = rtp_demuxer_rtcp(demuxer, (void*)data, length);
    if(r > 0)
    {
        std::cout << "sn err" << std::endl;
    }
}
static int rtp_onpacket(void* param, const void* packet, int bytes, uint32_t timestamp, int flags)
{
    const uint8_t start_code[] = { 0, 0, 0, 1 };
    rtp_receiver* ctx = (rtp_receiver*)param;
    if(0 == strcmp("H264", ctx->GetEncoding()) || 0 == strcmp("H265", ctx->GetEncoding()))
    {
        ctx->write_file(start_code, 4);
    }
    else if(0 == strcasecmp("mpeg4-generic", ctx->GetEncoding()))
    {
        uint8_t adts[7];
        int len = bytes + 7;
        uint8_t profile = 2;
        uint8_t sampling_frequency_index = 4;
        uint8_t channel_configuration = 2;
        adts[0] = 0xFF; /* 12-syncword */
        adts[1] = 0xF0 /* 12-syncword */ | (0 << 3)/*1-ID*/ | (0x00 << 2) /*2-layer*/ | 0x01 /*1-protection_absent*/;
        adts[2] = ((profile - 1) << 6) | ((sampling_frequency_index & 0x0F) << 2) | ((channel_configuration >> 2) & 0x01);
        adts[3] = ((channel_configuration & 0x03) << 6) | ((len >> 11) & 0x03); /*0-original_copy*/ /*0-home*/ /*0-copyright_identification_bit*/ /*0-copyright_identification_start*/
        adts[4] = (uint8_t)(len >> 3);
        adts[5] = ((len & 0x07) << 5) | 0x1F;
        adts[6] = 0xFC | ((len / 1024) & 0x03);
        //fwrite(adts, 1, sizeof(adts), ctx->fp);
        ctx->write_file(adts, sizeof(adts));
    }
    else if(0 == strcmp("MP4A-LATM", ctx->GetEncoding()))
    {
        // add ADTS header
    }
    size_t n = ctx->write_file((const uint8_t*)packet, bytes);
    if(n > 0)
    {
        std::cout << "data len:" << bytes + 4 << " time:" << timestamp << std::endl;
    }
    (void)timestamp;
    (void)flags;


    if(0 == strcmp("H264", ctx->GetEncoding()))
    {
        uint8_t type = *(uint8_t*)packet & 0x1f;
        if(0 < type && type <= 5)
        {
            // VCL frame
        }
    }
    else if(0 == strcmp("H265", ctx->GetEncoding()))
    {
        uint8_t type = (*(uint8_t*)packet >> 1) & 0x3f;
        if(type <= 32)
        {
            // VCL frame
        }
    }

    return 0;
}
void rtp_receiver::initRtpContext(int frequency, int payload, const char* encode)
{
    snprintf(encoding, sizeof(encoding), "%s", encode);
    snprintf((char*)&rtp_buffer[0], sizeof(rtp_buffer), "udp.%d.%s", payload, encoding);
    fp = fopen((const char*)&rtp_buffer[0], "wb");


    demuxer = rtp_demuxer_create(200, frequency ? frequency : 90000, payload, encoding, rtp_onpacket, this);
    if(NULL == demuxer)
        return; // ignore
    on_receive_rtcp_from();
    on_receive_rtp_from();
}
const char* rtp_receiver::GetEncoding()
{
    return &encoding[0];
}
inline const char* ftimestamp(int64_t t, char* buf)
{
    if(PTS_NO_VALUE == t)
    {
        sprintf(buf, "(null)");
    }
    else
    {
        t /= 90;
        sprintf(buf, "%d:%02d:%02d.%03d", (int)(t / 3600000), (int)((t / 60000) % 60), (int)((t / 1000) % 60), (int)(t % 1000));
    }
    return buf;
}
static int onpacket(void* param, int stream, int avtype, int flags, int64_t pts, int64_t dts, const void* data, size_t bytes)
{
    static std::map<int, std::pair<int64_t, int64_t>> s_streams;
    static char s_pts[64], s_dts[64];

    auto it = s_streams.find(stream);
    if(it == s_streams.end())
        it = s_streams.insert(std::make_pair(stream, std::pair<int64_t, int64_t>(pts, dts))).first;

    if(PTS_NO_VALUE == dts)
        dts = pts;

    if(PSI_STREAM_AAC == avtype || PSI_STREAM_AUDIO_G711A == avtype || PSI_STREAM_AUDIO_G711U == avtype)
    {
        //assert(0 == a_dts || dts >= a_dts);
        LogOut("BLL", L_DEBUG, "[A] pts: %s(%" PRId64 "), dts: %s(%" PRId64 "), diff: %03d/%03d, size: %u", ftimestamp(pts, s_pts), pts, ftimestamp(dts, s_dts), dts, (int)(pts - it->second.first) / 90, (int)(dts - it->second.second) / 90, (unsigned int)bytes);
        //fwrite(data, 1, bytes, afp);
    }
    else if(PSI_STREAM_H264 == avtype || PSI_STREAM_H265 == avtype || PSI_STREAM_VIDEO_SVAC == avtype)
    {
        //assert(0 == v_dts || dts >= v_dts);
        LogOut("BLL", L_DEBUG, "[V] pts: %s(%" PRId64 "), dts: %s(%" PRId64 "), diff: %03d/%03d, size: %u%s", ftimestamp(pts, s_pts), pts, ftimestamp(dts, s_dts), dts, (int)(pts - it->second.first) / 90, (int)(dts - it->second.second) / 90, (unsigned int)bytes, (flags & MPEG_FLAG_IDR_FRAME) ? " [I]" : "");
        //fwrite(data, 1, bytes, vfp);
    }
    else
    {
        //assert(0);
        //assert(0 == x_dts || dts >= x_dts);
        LogOut("BLL", L_DEBUG, "[X] pts: %s(%" PRId64 "), dts: %s(%" PRId64 "), diff: %03d/%03d", ftimestamp(pts, s_pts), pts, ftimestamp(dts, s_dts), dts, (int)(pts - it->second.first), (int)(dts - it->second.second));
    }

    it->second = std::make_pair(pts, dts);
    return 0;
}
void rtp_receiver::inputPsData(const uint8_t* data, size_t length)
{
    if(psdemuxer)
    {
        if(psdata_length > 0)
        {
            int r = ps_demuxer_input(psdemuxer, p_psdata_, psdata_length);
            if(r != psdata_length)
            {
                //inputPsData(p_psdata_+r, )
            }
        }
        else
        {
            int r = ps_demuxer_input(psdemuxer, data, length);
            if(r != length)
            {
                inputPsData(data + r, length - r);
            }
        }
    }
    else
    {
        psdemuxer = ps_demuxer_create(onpacket, this);
        p_psdata_ = new uint8_t[ps_max_length];
        int r = ps_demuxer_input(psdemuxer, data, length);
        if(r > 0 && r != length)
        {
            inputPsData(data + r, length - r);
        }
    }
}
int rtp_receiver::write_file(const uint8_t* data, size_t length)
{
    static size_t frameCount = 0;
    size_t n = fwrite(data, 1, length, fp);
    if(n > 0)
    {
        if(length != 4)
            inputPsData(data, length);
        if(++frameCount % 1000 == 0)
            fflush(fp);
        return n;
    }
    return 0;
}