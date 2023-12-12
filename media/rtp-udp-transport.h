#ifndef _rtp_udp_transport_h_
#define _rtp_udp_transport_h_

#include <thread>
//#include "sys/sock.h"
#include "media-source.h"
#include "../tools/SafeQueue.h"
#include "lineBuf.h"

#include "../asio/include/asio.hpp"
using asio::ip::udp;

class RTPUdpTransport : public IRTPTransport
{
public:
	RTPUdpTransport();
	virtual ~RTPUdpTransport();

public:
	virtual int Send(bool rtcp, const void* data, size_t bytes);
	void run();

public:
	int Init(unsigned short localport[2], const char* ip, unsigned short port[2]);
	int Init(int socket[2], const char* peer, unsigned short port[2]);
protected:
	void doRtpSend();

private:
	struct rtpPacket
	{
		uint8_t* buffer;
		int dataSize;
		rtpPacket():buffer(0), dataSize(0)
		{
			buffer = new uint8_t[1500];
		}
		~rtpPacket()
		{
			if (buffer)
			{
				delete[] buffer;
				buffer = NULL;
			}
		}
	};
	asio::io_service io_service;
	std::shared_ptr<udp::socket> rtp_socket_;
	std::shared_ptr<udp::socket> rtcp_socket_;
	udp::endpoint rtp_peer_endpoint_;
	udp::endpoint rtcp_peer_endpoint_;
	udp::endpoint receiver_rtcp_peer_endpoint_;

	SafeQueue<rtpPacket*> mSqueue;
	SafeQueue<rtpPacket*> mFreeQueue;

	uint32_t packetNum;
	/*SOCKET m_socket[2];
	int m_addrlen[2];
	struct sockaddr_storage m_addr[2];*/
};

#endif /* !_rtp_udp_transport_h_ */
