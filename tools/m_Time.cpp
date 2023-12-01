
#include "m_Time.h"

std::string time2str(time_t now)
{
	struct tm* info = localtime(&now);
	char datatime[24] = { 0 };
	strftime(datatime, 24, "%Y-%m-%d %H:%M:%S", info);
	return std::string(datatime);
}

CDateTime::CDateTime():mYear(0)
	, mMonth(0)
	, mDay(0)
	, mHour(0)
	, mMin(0)
	, mSec(0)
	, mMillisec(0)
{
	time_t now;
	time(&now);
	if (now == ((time_t)-1))
	{
		return;
	}

	setDatetime(now);
}
CDateTime::CDateTime(time_t now) :mYear(0)
	, mMonth(0)
	, mDay(0)
	, mHour(0)
	, mMin(0)
	, mSec(0)
	, mMillisec(0)
{
	setDatetime(now);
}
CDateTime::CDateTime(std::string str, std::string format) :mYear(0)
	, mMonth(0)
	, mDay(0)
	, mHour(0)
	, mMin(0)
	, mSec(0)
	, mMillisec(0)
{
	if (str.size() > 0)
	{
		sscanf(str.c_str(), format.c_str(), &mYear, &mMonth, &mDay, &mHour, &mMin, &mSec, &mMillisec);
		//strptime()
	}
}
std::string CDateTime::tmFormat(std::string format)
{
	time_t t_ = GetUTCTime();
	struct tm* info = localtime(&t_);
	char datatime[24] = { 0 };
	strftime(datatime, 24, format.c_str(), info);
	return std::string(datatime);
}
time_t CDateTime::GetUTCTime()
{
	tm tm_;
	tm_.tm_year = mYear - 1900;
	tm_.tm_mon = mMonth - 1;
	tm_.tm_mday = mDay;
	tm_.tm_hour = mHour;
	tm_.tm_min = mMin;
	tm_.tm_sec = mSec;
	return mktime(&tm_);
}
bool CDateTime::setDatetime(time_t datetime)
{
	struct tm gmt;
	struct tm* gmtp = NULL;
#if defined(WIN32) || defined(__sun)
	gmtp = localtime(&datetime);
	if (gmtp == NULL)
	{
		return false;
	}
	memcpy(&gmt, gmtp, sizeof(gmt));
#else
	gmtp = localtime_r(&datetime, &gmt);
	if (gmtp == NULL)
	{
		return false;
	}
#endif
	mYear = gmt.tm_year + 1900;
	mMonth = gmt.tm_mon + 1;
	mDay = gmt.tm_mday;
	mHour = gmt.tm_hour;
	mMin = gmt.tm_min;
	mSec = gmt.tm_sec;
	return true;
}