#pragma once

#include "../../asio/include/asio.hpp"
#include "lib/rtp/rtp-demuxer.h"
#include "lib/mpeg/mpeg-ps.h"

#ifdef _WIN32
#pragma comment(lib,"librtp")
#pragma comment(lib,"libmpeg")
#endif
using asio::ip::udp;

class rtp_receiver
{
public:
    rtp_receiver(short localport, const char* peerIp, short peerPort);
    ~rtp_receiver();

    void run();

    void on_receive_rtp_from();
    void on_receive_rtcp_from();
    void on_send_rtcp_to();

    void parseRtpData(const uint8_t* data, size_t length);
    void parseRtcpData(const uint8_t* data, size_t length);
    void initRtpContext(int frequency, int payload, const char* encode);
    const char* GetEncoding();
    void inputPsData(const uint8_t* data, size_t length);


    int write_file(const uint8_t* data, size_t length);
private:
    FILE* fp;
    char encoding[64];

    enum { max_length = 2048, ps_max_length = 1024 * 1024 * 1 };
    uint8_t rtp_buffer[2 * 1024];
    uint8_t rtcp_buffer[2 * 1024];

    struct rtp_demuxer_t* demuxer;
    struct ps_demuxer_t* psdemuxer;
    uint8_t* p_psdata_;
    int psdata_length;

    asio::io_service io_service;
    udp::socket rtp_socket_;
    udp::socket rtcp_socket_;
    udp::endpoint sender_endpoint_;
    udp::endpoint to_endpoint_;

};
// static int rtp_onpacket(void* param, const void* packet, int bytes, uint32_t timestamp, int flags);
// static int onpacket(void* param, int stream, int avtype, int flags, int64_t pts, int64_t dts, const void* data, size_t bytes);