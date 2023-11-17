#pragma once
#include <string>
#include <memory>

#ifndef MAX_UDP_PACKET
#define MAX_UDP_PACKET (1450-16)
#endif

struct IRTPTransport
{
	virtual int Send(bool rtcp, const void* data, size_t bytes) = 0;
	virtual void run() = 0;
};

struct IMediaSource
{
	virtual ~IMediaSource() {}

	virtual int Play() = 0;
	virtual int Pause() = 0;
	virtual int Seek(int64_t pos) = 0;
	virtual int SetSpeed(double speed) = 0;
	virtual int GetDuration(int64_t& duration) const = 0;
	virtual int GetSDPMedia(std::string& sdp) const = 0;
	virtual int GetRTPInfo(const char* uri, char* rtpinfo, size_t bytes) const = 0;
	virtual int SetTransport(const char* track, std::shared_ptr<IRTPTransport> transport) = 0;
	virtual void Input(int datatype, const uint8_t* data, size_t size) = 0;
};