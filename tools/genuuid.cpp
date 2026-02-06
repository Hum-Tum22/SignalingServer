#include "genuuid.h"
#define _CRT_RAND_S 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <sys/types.h>
#include <time.h>
//#include <sys/sysinfo.h>


#define unsigned64_t unsigned long long

#define I64(C) C##LL

typedef unsigned long   unsigned32;
typedef unsigned short  unsigned16;
typedef unsigned char   unsigned8;
typedef unsigned char   byte;
typedef unsigned long long uuid_time_t;


#define UUIDS_PER_TICK 1024


#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
namespace imuuid
{
#ifdef _WIN32
int gettimeofday(struct timeval* tp, void* tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = (long)clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return (0);
}
unsigned int random()
{
	errno_t err;
	unsigned int number;
	err = rand_s(&number);
	if (err != 0)
	{
	}
	return number;
}
#endif

typedef struct
{
	unsigned short  time_low1;
	unsigned short  time_low2;
	unsigned short  time_mid;
	unsigned short  time_hi_and_version;
	unsigned short  clock;
	unsigned short  nodefirst;
	unsigned short  nodelast1;
	unsigned short  nodelast2;
} uuid_t_t;

void get_system_time(uuid_time_t* uuid_time)
{
	struct timeval tp;
	gettimeofday(&tp, (struct timezone*)0);

	/* Offset between UUID formatted times and Unix formatted times.
	UUID UTC base time is October 15, 1582.
	Unix base time is January 1, 1970.
	*/

	*uuid_time = ((unsigned64_t)tp.tv_sec * 10000000)
		+ ((unsigned64_t)tp.tv_usec * 10)
		+ I64(0x01B21DD213814000);
}

static unsigned short true_random(void)
{
#ifndef _WIN32
	static int inited = 0;
	uuid_time_t time_now = 0;

	if (!inited)
	{
		get_system_time(&time_now);
		time_now = time_now / UUIDS_PER_TICK;
		srand((unsigned int)
			(((time_now >> 32) ^ time_now) & 0xffffffff));
		inited = 1;
	}

	srand((unsigned int)random());
#endif
	return random();
}

char* uuidgen(void)
{
	uuid_t_t uuid;

	static char src[64];

	unsigned short time_low1 = 0;
	unsigned short time_low2 = 0;
	unsigned short time_mid = 0;
	unsigned short time_hi_and_version = 0;
	unsigned short clock = 0;
	unsigned short nodefirst = 0;
	unsigned short nodelast1 = 0;
	unsigned short nodelast2 = 0;

	time_low1 = true_random();
	time_low2 = true_random();
	time_mid = true_random();
	time_hi_and_version = true_random();
	clock = true_random();
	nodefirst = true_random();
	nodelast1 = true_random();
	nodelast2 = true_random();

	uuid.time_low1 = time_low1;
	uuid.time_low2 = time_low2;
	uuid.time_mid = time_mid;
	uuid.time_hi_and_version = (time_hi_and_version | 0x000F) ^ 0x000B;
	uuid.clock = (clock | 0x0300) ^ 0x0200;
	uuid.nodefirst = nodefirst;
	uuid.nodelast1 = nodelast1;
	uuid.nodelast2 = nodelast2;

	sprintf(src, "%4.4x%4.4x-%4.4x-%4.4x-%4.4x-%4.4x%4.4x%4.4x",
		uuid.time_low1, uuid.time_low2, uuid.time_mid,
		uuid.time_hi_and_version, uuid.clock, uuid.nodefirst,
		uuid.nodelast1, uuid.nodelast2);

	return src;
}
}
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
std::string generateUUID()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 15);
	int64_t curTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	const char* uuid_chars = "0123456789abcdef";
	std::stringstream ss;
	for (int i = 0; i < 32; ++i) {
		int nibble = dis(gen);
		char c = uuid_chars[nibble];
		if (i == 8 || i == 12 || i == 16 || i == 20) {
			ss << "-";
		}
		ss << c;
	}
	return ss.str();
}