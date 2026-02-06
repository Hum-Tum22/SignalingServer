#pragma once
#include <stdint.h>
#include <assert.h>

#define H264_NAL(v)	(v & 0x1F)
#define H265_NAL(v)	 ((v>> 1) & 0x3f)
enum { NAL_IDR = 5, NAL_SEI = 6, NAL_SPS = 7, NAL_PPS = 8 };
enum { NAL_IDR_W_RADL = 19, NAL_IDR_N_LP = 20, NAL_VPS = 32, NAL_SPS_5 = 33, NAL_PPS_5 = 34, NAL_SEI_5 = 39 };


inline const uint8_t* search_start_code(const uint8_t* ptr, const uint8_t* end)
{
	for (const uint8_t* p = ptr; p + 3 < end; p++)
	{
		if (0x00 == p[0] && 0x00 == p[1] && (0x01 == p[2] || (0x00 == p[2] && 0x01 == p[3])))
			return p;
	}
	return end;
}

inline int h264_nal_type(const unsigned char* ptr)
{
	int i = 2;
	assert(0x00 == ptr[0] && 0x00 == ptr[1]);
	if (0x00 == ptr[2])
		++i;
	assert(0x01 == ptr[i]);
	return H264_NAL(ptr[i + 1]);
}

inline int h264_nal_new_access(const unsigned char* ptr, const uint8_t* end)
{
	int i = 2;
	if (end - ptr < 4)
		return 1;
	assert(0x00 == ptr[0] && 0x00 == ptr[1]);
	if (0x00 == ptr[2])
		++i;
	assert(0x01 == ptr[i]);
	int nal_unit_type = H264_NAL(ptr[i + 1]);
	if (nal_unit_type < 1 || nal_unit_type > 5)
		return 1;

	if (ptr + i + 2 > end)
		return 1;

	// Live555 H264or5VideoStreamParser::parse
	// The high-order bit of the byte after the "nal_unit_header" tells us whether it's
	// the start of a new 'access unit' (and thus the current NAL unit ends an 'access unit'):
	return (ptr[i + 2] & 0x80) != 0 ? 1 : 0;
}


inline int h265_nal_type(const unsigned char* ptr)
{
	int i = 2;
	assert(0x00 == ptr[0] && 0x00 == ptr[1]);
	if (0x00 == ptr[2])
		++i;
	assert(0x01 == ptr[i]);
	return H265_NAL(ptr[i + 1]);
}