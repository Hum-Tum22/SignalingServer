#pragma once

#include <time.h>
#include <iostream>

//date 2022-08-24 16:33:56
std::string time2str(time_t now);


class CDateTime
{
public:
	CDateTime();
	CDateTime(time_t now);
	CDateTime(std::string str, std::string format = "%04d-%02d-%02d %02d:%02d:%02d.%3d");
	time_t GetUTCTime();
	std::string tmFormat(std::string format = "%Y-%m-%d %H:%M:%S");
	bool setDatetime(time_t datetime);

private:
	int mYear;
	int mMonth;
    int mDay;
    int mHour;
    int mMin;
    int mSec;
    int mMillisec;
};