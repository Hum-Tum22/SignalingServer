#include "rtp-udp-transport.h"
#include "SelfLog.h"
//#include "sockpair.h"
//#include "ctypedef.h"
//#include "port/ip-route.h"

RTPUdpTransport::RTPUdpTransport() :packetNum(0)
{
}

RTPUdpTransport::~RTPUdpTransport()
{
    rtp_socket_->close();
    rtcp_socket_->close();
    int squeSize = mSqueue.size();
    int freeQueSize = mFreeQueue.size();
    LogOut("BLL", L_DEBUG, "free mSqueue size:%d mFreeQueue size:%d", squeSize, freeQueSize);
    while (!mSqueue.empty())
    {
        rtpPacket* packet = NULL;
        mSqueue.dequeue(packet);
        delete packet;
    }
    while (!mFreeQueue.empty())
    {
        rtpPacket* packet = NULL;
        mFreeQueue.dequeue(packet);
        delete packet;
    }
}

int RTPUdpTransport::Init(unsigned short localport[2], const char* ip, unsigned short port[2])
{
    rtp_peer_endpoint_.address(asio::ip::address_v4::from_string(ip));
    rtp_peer_endpoint_.port(port[0]);
    rtcp_peer_endpoint_.address(asio::ip::address_v4::from_string(ip));
    rtcp_peer_endpoint_.port(port[1]);
    rtp_socket_.reset(new udp::socket(io_service, udp::endpoint(udp::v4(), localport[0])));
    rtcp_socket_.reset(new udp::socket(io_service, udp::endpoint(udp::v4(), localport[1])));

    //loop = std::thread(&RTPUdpTransport::run, this);
    return 1;
}

int RTPUdpTransport::Init(int socket[2], const char* peer, unsigned short port[2])
{
    /*int r1 = socket_addr_from(&m_addr[0], &m_addrlen[0], peer, port[0]);
    int r2 = socket_addr_from(&m_addr[1], &m_addrlen[1], peer, port[1]);
    if (0 != r1 || 0 != r2)
        return 0 != r1 ? r1 : r2;

    m_socket[0] = socket[0];
    m_socket[1] = socket[1];*/
    return 0;
}

int RTPUdpTransport::Send(bool rtcp, const void* data, size_t bytes)
{
    if (!rtcp)
    {
        /*if (packetNum != 0)
        {
            rtpPacket* packet = NULL;
            if (mFreeQueue.empty())
            {
                packet = new rtpPacket();
            }
            else
            {
                mFreeQueue.dequeue(packet);
            }
            if (packet && packet->buffer && bytes <= 1500)
            {
                memcpy((void*)packet->buffer, data, bytes);
                packet->dataSize = bytes;
                mSqueue.enqueue(packet);
            }
            else
            {
                printf("packet:%p, bytes:%Zu\n", packet, bytes);
            }
            return bytes;
        }
        packetNum++;*/
        rtp_socket_->send_to(asio::buffer(data, bytes), rtp_peer_endpoint_);
        /*rtp_socket_->async_send_to(asio::buffer(data, bytes), rtp_peer_endpoint_,
            [this](const asio::error_code& error, std::size_t bytes_transferred)
            {
                if (!error)
                {
                    packetNum--;
                    doRtpSend();
                }
                else
                {
                    rtp_socket_->close();
                }
            });*/
    }
    else
    {
        rtcp_socket_->send_to(asio::buffer(data, bytes), rtcp_peer_endpoint_);
        /*rtcp_socket_->async_send_to(asio::buffer(data, bytes), rtcp_peer_endpoint_,
            [this](const asio::error_code& error, std::size_t bytes_transferred)
            {
                if (!error)
                {

                }
                else
                {
                    rtcp_socket_->close();
                }
            });*/
    }
    return bytes;
    //return socket_sendto(m_socket[i], data, bytes, 0, (sockaddr*)&m_addr[i], m_addrlen[i]);
}
void RTPUdpTransport::run()
{
    //io_service.run();
    io_service.run_one();
}
void RTPUdpTransport::doRtpSend()
{
    if (mSqueue.empty())
    {
        return;
    }
    else
    {
        rtpPacket* packet = NULL;
        mSqueue.dequeue(packet);
        if (!packet)
            return;
        rtp_socket_->async_send_to(asio::buffer(packet->buffer, packet->dataSize), rtp_peer_endpoint_,
            [&, this](const asio::error_code& error, std::size_t bytes_transferred)
            {
                if (!error)
                {
                    packetNum--;
                    doRtpSend();
                }
                else
                {
                    rtp_socket_->close();
                }
                mFreeQueue.enqueue(packet);
            });
    }
}