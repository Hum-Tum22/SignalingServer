
#include "send_rtp.h"

#include "lib/mov/mov-reader.h"
#include "lib/mov/mov-format.h"
#include "lib/flv/mpeg4-hevc.h"
#include "lib/flv/mpeg4-avc.h"
#include "lib/flv/mpeg4-aac.h"
#include "lib/flv/webm-vpx.h"
#include "lib/flv/aom-av1.h"
#include "lib/rtp/rtp-payload.h"
#include "lib/rtp/rtp-profile.h"
#include "lib/mpeg/mpeg-ps.h"

#include <iostream>

#ifdef _WIN32
#pragma comment(lib,"librtp")
#pragma comment(lib,"libmpeg")
#pragma comment(lib,"libflv")
#pragma comment(lib,"libmov")
#endif


send_rtp::send_rtp(short localport, const char* peerIp, short peerPort):
	 rtp_socket_(io_service, udp::endpoint(udp::v4(), localport))
	, rtcp_socket_(io_service, udp::endpoint(udp::v4(), localport + 1))
	, rtp_peer_endpoint_(asio::ip::address_v4::from_string(peerIp), peerPort)
	, rtcp_peer_endpoint_(asio::ip::address_v4::from_string(peerIp), peerPort + 1)
{
}
send_rtp::~send_rtp()
{
}
void send_rtp::run()
{
	io_service.run();
}
void send_rtp::send_rtp_to(const uint8_t* data, size_t length)
{
    rtp_socket_.async_send_to(asio::buffer(data, length), rtp_peer_endpoint_,
        [this](const asio::error_code& error, std::size_t bytes_transferred)
        {
            if (!error)
            {

            }
            else
            {
                rtp_socket_.close();
            }
        });
}
void send_rtp::send_rtcp_to(const uint8_t* data, size_t length)
{
    rtcp_socket_.async_send_to(asio::buffer(data, length), rtcp_peer_endpoint_,
        [this](const asio::error_code& error, std::size_t bytes_transferred)
        {
            if (!error)
            {

            }
            else
            {
                rtcp_socket_.close();
            }
        });
}
void send_rtp::on_receive_rtcp_from()
{
    rtcp_socket_.async_receive_from(asio::buffer(rtcp_buffer, max_length), receiver_rtcp_peer_endpoint_,
        [this](std::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                //
                send_rtcp_to(rtcp_buffer, length);
                on_receive_rtcp_from();
            }
            else
            {
                rtp_socket_.close();
            }
        }
    );
}
void send_rtp::parseRtcpData(const uint8_t* data, size_t length)
{
    if (!demuxer || length < 12)
    {
        return;
    }
    //int r = rtp_demuxer_rtcp(demuxer, (void*)data, length);
    /*if (r > 0)
    {
        std::cout << "sn err" << std::endl;
    }*/
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

//#define RTP_VIDEO_WITH_PS

#define RTP_LOST_PERCENT 5

extern "C" const struct mov_buffer_t* mov_file_buffer(void);

static uint8_t s_packet[2 * 1024 * 1024];
static uint8_t s_buffer[4 * 1024 * 1024];
static struct mpeg4_hevc_t s_hevc;
static struct mpeg4_avc_t s_avc;
static struct mpeg4_aac_t s_aac;
static struct webm_vpx_t s_vpx;
static struct aom_av1_t s_av1;

struct mov_rtp_test_t;
struct mov_rtp_test_stream_t
{
    struct mov_rtp_test_t* ctx;

    int av;
    int object;
    int track;
    int psi;
    int64_t dts;

    void* encoder;
};

struct mov_rtp_test_t
{
    struct mov_rtp_test_stream_t a, v;

    struct ps_muxer_t* psenc;
};

static unsigned char packet[8 * 1024 * 1024];

static void* rtp_alloc(void* /*param*/, int bytes)
{
    static uint8_t buffer[2 * 1024 * 1024 + 4] = { 0, 0, 0, 1, };
    assert(bytes <= sizeof(buffer) - 4);
    return buffer + 4;
}

static void rtp_free(void* /*param*/, void* /*packet*/)
{
}

static int rtp_encode_packet(void* param, const void* packet, int bytes, uint32_t timestamp, int /*flags*/)
{
    struct mov_rtp_test_stream_t* ctx = (struct mov_rtp_test_stream_t*)param;

    //int x = rand();
    //if( (x % 100) < RTP_LOST_PERCENT )
    //{
    //    printf("======== discard [%s] timestamp: %u ==============\n", ctx->av ? "V" : "A", (unsigned int)timestamp);
    //    return 0;
    //}

    int r = 0;// rtp_payload_decode_input(ctx->decoder, packet, bytes);
    return r >= 0 ? 0 : r;
}

static int rtp_payload_codec_create(struct mov_rtp_test_stream_t* ctx, int payload, const char* encoding, uint16_t seq, uint32_t ssrc)
{

    struct rtp_payload_t handler2;
    handler2.alloc = rtp_alloc;
    handler2.free = rtp_free;
    handler2.packet = rtp_encode_packet;
    ctx->encoder = rtp_payload_encode_create(payload, encoding, seq, ssrc, &handler2, ctx);

    return 0;
}

inline const char* ftimestamp(uint32_t t, char* buf)
{
    sprintf(buf, "%02u:%02u:%02u.%03u", t / 3600000, (t / 60000) % 60, (t / 1000) % 60, t % 1000);
    return buf;
}

static void onread(void* param, uint32_t track, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags)
{
    static char s_pts[64], s_dts[64];
    static int64_t v_pts, v_dts;
    static int64_t a_pts, a_dts;
    struct mov_rtp_test_t* ctx = (struct mov_rtp_test_t*)param;
    int n = bytes;

    if (ctx->v.track == track)
    {
        if (MOV_OBJECT_H264 == ctx->v.object)
        {
            n = h264_mp4toannexb(&s_avc, buffer, bytes, s_packet, sizeof(s_packet));
        }
        else if (MOV_OBJECT_HEVC == ctx->v.object)
        {
            n = h265_mp4toannexb(&s_hevc, buffer, bytes, s_packet, sizeof(s_packet));
        }
        else if (MOV_OBJECT_AV1 == ctx->v.object)
        {
            n = aom_av1_codec_configuration_record_save(&s_av1, s_packet, sizeof(s_packet));
        }
        else if (MOV_OBJECT_VP8 == ctx->v.object || MOV_OBJECT_VP9 == ctx->v.object)
        {
            // nothing to do
        }
        else
        {
            assert(0);
        }

        printf("[V] pts: %s, dts: %s, diff: %03d/%03d, %d%s\n", ftimestamp(pts, s_pts), ftimestamp(dts, s_dts), (int)(pts - v_pts), (int)(dts - v_dts), (int)n, flags ? " [I]" : "");
        v_pts = pts;
        v_dts = dts;
        if (MOV_OBJECT_H264 == ctx->v.object || MOV_OBJECT_HEVC == ctx->v.object)
        {
            ctx->v.dts = dts;
            ps_muxer_input(ctx->psenc, ctx->v.psi, (MOV_AV_FLAG_KEYFREAME & flags) ? 0x0001 : 0, pts, dts, s_packet, n);
            return;
        }
        assert(0 == rtp_payload_encode_input(ctx->v.encoder, s_packet, n, (unsigned int)dts));
    }
    else if (ctx->a.track == track)
    {
        if (MOV_OBJECT_AAC == ctx->a.object)
        {
            n = mpeg4_aac_adts_save(&s_aac, bytes, s_packet, sizeof(s_packet));
        }
        else if (MOV_OBJECT_OPUS == ctx->a.object)
        {
            assert(0);
        }
        else
        {
            assert(0);
        }

        printf("[A] pts: %s, dts: %s, diff: %03d/%03d, %d\n", ftimestamp(pts, s_pts), ftimestamp(dts, s_dts), (int)(pts - a_pts), (int)(dts - a_dts), (int)n);
        a_pts = pts;
        a_dts = dts;
        assert(0 == rtp_payload_encode_input(ctx->a.encoder, s_packet, n, (unsigned int)dts));
    }
    else
    {
        assert(0);
    }
}

static void mov_video_info(void* param, uint32_t track, uint8_t object, int /*width*/, int /*height*/, const void* extra, size_t bytes)
{
    struct mov_rtp_test_t* ctx = (struct mov_rtp_test_t*)param;
    ctx->v.track = track;
    ctx->v.object = object;
    ctx->v.av = 1;

    if (MOV_OBJECT_H264 == object)
    {
        ctx->v.psi = ps_muxer_add_stream(ctx->psenc, PSI_STREAM_H264, NULL, 0);
        assert(bytes == mpeg4_avc_decoder_configuration_record_load((const uint8_t*)extra, bytes, &s_avc));
    }
    else if (MOV_OBJECT_HEVC == object)
    {
        ctx->v.psi = ps_muxer_add_stream(ctx->psenc, PSI_STREAM_H265, NULL, 0);
        assert(bytes == mpeg4_hevc_decoder_configuration_record_load((const uint8_t*)extra, bytes, &s_hevc));
    }
    else if (MOV_OBJECT_AV1 == object)
    {
        assert(0 == rtp_payload_codec_create(&ctx->v, 96, "AV1", 0, 0));
        assert(bytes == aom_av1_codec_configuration_record_load((const uint8_t*)extra, bytes, &s_av1));
    }
    else if (MOV_OBJECT_VP9 == object)
    {
        assert(0 == rtp_payload_codec_create(&ctx->v, 96, "VP9", 0, 0));
        assert(bytes == webm_vpx_codec_configuration_record_load((const uint8_t*)extra, bytes, &s_vpx));
    }
    else if (MOV_OBJECT_VP8 == object)
    {
        assert(0 == rtp_payload_codec_create(&ctx->v, 96, "VP8", 0, 0));
        assert(bytes == webm_vpx_codec_configuration_record_load((const uint8_t*)extra, bytes, &s_vpx));
    }
    else
    {
        assert(0);
    }
}

static void mov_audio_info(void* param, uint32_t track, uint8_t object, int /*channel_count*/, int /*bit_per_sample*/, int /*sample_rate*/, const void* extra, size_t bytes)
{
    struct mov_rtp_test_t* ctx = (struct mov_rtp_test_t*)param;
    ctx->a.track = track;
    ctx->a.object = object;
    ctx->a.av = 0;

    if (MOV_OBJECT_AAC == object)
    {
        assert(0 == rtp_payload_codec_create(&ctx->a, 97, "MP4A-LATM", 0, 0));
        assert(bytes == mpeg4_aac_audio_specific_config_load((const uint8_t*)extra, bytes, &s_aac));
    }
    else if (MOV_OBJECT_OPUS == object)
    {
        assert(0 == rtp_payload_codec_create(&ctx->a, 97, "OPUS", 0, 0));
    }
    else
    {
        assert(0);
    }
}

static void* ps_alloc(void* /*param*/, size_t bytes)
{
    static char s_buffer[2 * 1024 * 1024];
    assert(bytes <= sizeof(s_buffer));
    return s_buffer;
}

static void ps_free(void* /*param*/, void* /*packet*/)
{
    return;
}

static int ps_write(void* param, int stream, void* packet, size_t bytes)
{
    struct mov_rtp_test_t* ctx = (struct mov_rtp_test_t*)param;
    return rtp_payload_encode_input(ctx->v.encoder, packet, bytes, (unsigned int)ctx->v.dts);
}

static int ps_onpacket(void* ps, int stream, int codecid, int flags, int64_t pts, int64_t dts, const void* data, size_t bytes)
{
    printf("PS Decode [V] pts: %08lu, dts: %08lu, bytes: %u, %s\n", (unsigned long)pts, (unsigned long)dts, (unsigned int)bytes, flags ? " [I]" : "");
    return 0;
}

void mov_rtp_test(const char* mp4)
{
    //struct mov_rtp_test_t ctx;
    //memset(&ctx, 0, sizeof(ctx));
    //ctx.a.ctx = &ctx;
    //ctx.v.ctx = &ctx;

    //struct ps_muxer_func_t handler;
    //handler.alloc = ps_alloc;
    //handler.write = ps_write;
    //handler.free = ps_free;
    //ctx.psenc = ps_muxer_create(&handler, &ctx);
    //assert(0 == rtp_payload_codec_create(&ctx.v, 96, "MP2P", 0, 0));

    //FILE* fp = fopen(mp4, "rb");
    //mov_reader_t* mov = mov_reader_create(mov_file_buffer(), fp);
    //uint64_t duration = mov_reader_getduration(mov);

    //struct mov_reader_trackinfo_t info = { mov_video_info, mov_audio_info };
    //mov_reader_getinfo(mov, &info, &ctx);

    ////srand((int)time(NULL));
    //while (mov_reader_read(mov, s_buffer, sizeof(s_buffer), onread, &ctx) > 0)
    //{
    //}

    //if (ctx.a.encoder)
    //    rtp_payload_encode_destroy(ctx.a.encoder);
    //if (ctx.v.encoder)
    //    rtp_payload_encode_destroy(ctx.v.encoder);

    //ps_muxer_destroy(ctx.psenc);
    //mov_reader_destroy(mov);
}
