#pragma once


#include "../../asio/include/asio.hpp"


using asio::ip::udp;

class send_rtp
{
public:
	send_rtp(short port, const char* toIp, short to_port);
	~send_rtp();

	void run();

	void send_rtp_to(const uint8_t* data, size_t length);
	void send_rtcp_to(const uint8_t* data, size_t length);
	void on_receive_rtcp_from();

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

	struct rtp_muxer_t* demuxer;
	struct ps_demuxer_t* psdemuxer;

	struct ps_muxer_t* psenc;
	uint8_t* p_psdata_;
	int psdata_length;

	asio::io_service io_service;
	udp::socket rtp_socket_;
	udp::socket rtcp_socket_;
	udp::endpoint rtp_peer_endpoint_;
	udp::endpoint rtcp_peer_endpoint_;
	udp::endpoint receiver_rtcp_peer_endpoint_;

};