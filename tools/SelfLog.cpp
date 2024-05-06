#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <string.h>
#include <chrono>


#ifdef _MSC_VER
#include <io.h>
//#define F_OK 0
#endif
#if (defined(_WIN32) || defined(_WIN64))
#define localtime_r(a,b) localtime_s(b,a)
#else
#include <unistd.h>
#endif
#include "SelfLog.h"

static const char *strLevel[LEVEL_NUM] = {"FATAL", "ERROR", "WARN", "NOTICE", "INFO", "DEBUG"};
static const char *strColor[LEVEL_NUM] = {RED, LIGHT_RED, LIGHT_YELLOW, LIGHT_BLUE, LIGHT_GREEN, GRAY};
static const size_t gColor[LEVEL_NUM] = {strlen(RED), strlen(LIGHT_RED), strlen(LIGHT_YELLOW), strlen(LIGHT_BLUE), strlen(LIGHT_GREEN), strlen(GRAY)};

	

CLog& CLog::Instance()
{
	static CLog g_Clog;
    return g_Clog;
}
CLog::CLog() :nFileNum(5), nFileSize(1 * 1024 * 1024), nTarget(E_LOG_TARGET_BOTH), mLogFilePtr(NULL)
, mPath("./"), mName("file"), nNoneColorlen((int)strlen(NONE__)), nNewLinelen(strlen(LOG_NEWLINE)), mCurFileSize(0)
, mLogConfig("./param/appconfig.ini"), mRunStatus(false), changeFile(false)
{
}
CLog::~CLog()
{
    StopLog();
}
void CLog::Output(const std::string name, int level, std::string &padmsg, const char* msg, ...)
{
    //std::string strName(name);
	if (mLogMap.find(name) == mLogMap.end())
        return;
	if(level < L_FATAL || level >= LEVEL_NUM)
	{
		return;
	}
	if(mLogMap[name] >= level)
	{
		//(2023-03-31 16:39:21,130)[Common-INFO-commonTool.cpp:231]
		//Dev.cpp:1305]

		//(2023-03-31 16:39:21,130)[Common-INFO-

		//struct timeval current;
        //gettimeofday(&current, NULL);
		long long curtime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		char strTime[20] = { 0 };
		time_t cur_t = curtime/1000;

		struct tm tmtime;
		memset(&tmtime, 0, sizeof(tmtime));
		localtime_r(&cur_t, &tmtime);
		strftime(strTime, sizeof(strTime), "%Y-%m-%d %H:%M:%S", &tmtime);
		

		char tmp[1024*50] = {0};
		int size = 0, tmpSize = sizeof(tmp) - 1 - nNewLinelen;

        size += ::snprintf(&tmp[size], tmpSize - size - 1, "(%s.%03d)[%s-%s-%s", strTime, int(curtime % 1000), name.c_str(), strLevel[level], padmsg.c_str());
        va_list args;
        va_start(args, msg);
        int needlen = ::vsnprintf(NULL, 0, msg, args);
        va_end(args);
        if (needlen + 1 > tmpSize - size)
        {
            needlen = tmpSize - size;
        }
        va_start(args, msg);
        int formatlen = ::vsnprintf(&tmp[size], needlen + 1, msg, args);
        va_end(args);
        size += needlen;

        if (E_LOG_TARGET_STDERR == nTarget || E_LOG_TARGET_BOTH == nTarget)
        {
            printf("%s%s%s\n", strColor[level], tmp, NONE__);
        }
        if (nullptr != mLogFilePtr && (E_LOG_TARGET_FILE == nTarget || E_LOG_TARGET_BOTH == nTarget))
        {
            tmp[size++] = 0x0A;
            std::string line(tmp, size);
            std::tuple<std::string, int> linetu(line, level);
            mLogs.enqueue(linetu);
        }
    }
	return;
}
void CLog::StopLog()
{
    mRunStatus = false;
    if (mLogThread.joinable())
    {
        mLogThread.join();
    }
	LogClose();
}
void CLog::InitLog()
{
	mLogMap["LOGGER"] = 1;
    // mLogConfig.reload();
	std::string fileNum;
    mLogConfig.readIniFileKeyValue("LOG", "fileNum", fileNum);
    if (!fileNum.empty())
    {
        nFileNum = std::atoi(fileNum.c_str());
    }
	std::string fileSzie;
    mLogConfig.readIniFileKeyValue("LOG", "fileSize", fileSzie);
    if (!fileSzie.empty())
    {
        nFileSize = (1 * 1024 * 1024) * std::atoi(fileSzie.c_str());
    }
    std::string outPut;
    mLogConfig.readIniFileKeyValue("LOG", "target", outPut);
    if (!outPut.empty())
    {
        nTarget = (APP_LOG_TARGET)std::atoi(outPut.c_str());
    }

	mLogConfig.readIniFileKeyValue("LOG", "log_path", mPath);
	mLogConfig.readIniFileKeyValue("LOG", "log_name", mName);
	if (E_LOG_TARGET_FILE == nTarget || E_LOG_TARGET_BOTH == nTarget)
    {
        LogInit();
        LogOpen();
    }
    LogOut("LOGGER", 1, "fileNum = %d", nFileNum);
    LogOut("LOGGER", 1, "fileSize = %ld", nFileSize);
    LogOut("LOGGER", 1, "target = %d", nTarget);
	LogOut("LOGGER", 1, "log_path = %s", mPath.c_str());
	LogOut("LOGGER", 1, "log_name = %s", mName.c_str());
	std::map<std::string, std::string> logLevel;
	mLogConfig.readIniFileSection("LOG_LEVEL", logLevel);
	for(auto &it : logLevel)
	{
		int level = std::atoi(it.second.c_str());
		if(level < L_FATAL || level > L_DEBUG || it.first.empty())
		{
			continue;
		}
		mLogMap[it.first] = level;
		LogOut("LOGGER", 1, "log %s = level:%d", it.first.c_str(), level);
    }

    mRunStatus = true;
    mLogThread = std::thread(&CLog::writeLogThread, this);
}
void CLog::writeLogThread()
{
    prctl(PR_SET_NAME, "Log");
    while (mRunStatus)
    {
        std::tuple<std::string, int> linetu;
        if (mLogs.dequeue(linetu))
        {
            std::string& strLine = std::get<0>(linetu);
            bool bRet = LogWrite(strLine.c_str(), strLine.size(), std::get<1>(linetu));
            if (true != bRet)
            {
                printf("ERROR: Write app log file failed: %s\n", mArrLogFileName.at(0).c_str());
            }
        }
        else
        {
            // printf("mLogs size: %d\n", mLogs.size());
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (changeFile)
        {
            if (E_LOG_TARGET_FILE == nTarget || E_LOG_TARGET_BOTH == nTarget)
            {
                LogClose();
                LogInit();
                LogOpen();
            }
            changeFile = false;
        }
    }
}
void CLog::SetLogLevel(std::string name, int level)
{
    if(level < L_FATAL || level > LEVEL_NUM || name.empty())
    {
        printf("CLog set level %s,%d\n", name.c_str(), level);
        return;
	}
	mLogMap[name] = level;
	LogOut(name, 0, "set log %s = %d", name.c_str(), level);
	return;
}
void CLog::SetLogTarget(int eTarget)
{
	if (eTarget >= E_LOG_TARGET_NONE && eTarget < E_LOG_TARGET_MAX) {
        nTarget = (APP_LOG_TARGET)eTarget;
        printf("CLog set target:%d\n", eTarget);
    }
}

void CLog::SetLoggerPath(const char* path, const char* name)
{
    if (mPath != std::string(path))
    {
        mPath = path;
        changeFile = true;
    }
    if (mName != std::string(name))
    {
        mName = name;
        changeFile = true;
    }
    printf("CLog set file path name:%s,%s\n", path, name);
}
void CLog::SetLogFileSize(int size) 
{
	if(size > 0)
	{
        nFileSize = size * 1024 * 1024;
        printf("CLog set file size:%dM\n", size);
    }
}
void CLog::SetLogFileNum(int num)
{
    nFileNum = num;
    printf("CLog set file num:%d\n", num);
}
int CLog::GetLogLevel(const std::string name)
{
	auto it = mLogMap.find(name);
	if(it != mLogMap.end())
		return mLogMap[name];
	return 0;
}
int CLog::GetLogTarget()
{
	return nTarget;
}
int CLog::GetLogFileSize()
{
	return nFileSize;
}
int CLog::GetLogFileNum()
{
	return nFileNum;
}
void CLog::GetLogPathName(std::string &path, std::string &name)
{
	path = mPath;
	name = mName;
}

bool CLog::LogOpen()
{
    if (E_LOG_TARGET_FILE == nTarget || E_LOG_TARGET_BOTH == nTarget)
	{
        if (nFileNum < 1)
        {
            return false;
        }
#if !(defined(_WIN32) || defined(_WIN64))
        //O_APPEND O_TRUNC
        int LogFd = ::open(mArrLogFileName.at(0).c_str(), O_RDWR | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
        if (LogFd >= 0)
        {
            if ((mLogFilePtr = ::fdopen(LogFd, "a+")) == NULL)
            {
                printf("ERROR: Failed to open app log file: %s!\n", mArrLogFileName.at(0).c_str());
                ::close(LogFd);
                return false;
            }
        }
#else
        if ((mLogFilePtr = ::fopen(mArrLogFileName.at(0).c_str(), "a+")) == NULL)
        {
            printf("ERROR: Failed to open app log file: %s!\n", mArrLogFileName.at(0).c_str());
            return false;
        }	
#endif	
		fseek(mLogFilePtr, 0, SEEK_END);
		mCurFileSize = ftell(mLogFilePtr);
    }
    return true;
}
void CLog::LogClose()
{
	if (mLogFilePtr)
	{
        ::fflush(mLogFilePtr);
        ::fclose(mLogFilePtr);
        mLogFilePtr = nullptr;
    }
}
bool CLog::LogWrite(const char* szLog, int nSize, int Level)
{
    if (nullptr == mLogFilePtr)
    {
        printf("mLogFilePtr NULL\n");
        return false;
    }

    if (mCurFileSize + nSize > nFileSize)
    {
        if (!SwitchFile())
        {
            printf("SwitchFile failed\n");
            return false;
        }
    }
	int wtSize = 0;

    int wlen = ::fwrite(szLog, 1, nSize, mLogFilePtr);
    if (wlen > 0)
    {
        wtSize += wlen;
    }
    else
    {
        printf("log fwrite failed:%d\n", errno);
    }
	if (Level <= E_LOG_LV_ERROR)
	{
        ::fflush(mLogFilePtr);
    }
	mCurFileSize += wtSize;
    return nSize == wtSize ? true : false;
}
bool CLog::SwitchFile()
{
    LogClose();

    mCurFileSize = 0;
    for (int i = nFileNum - 1; i > 0; i--)
    {
        if (nFileNum - 1 == i && 0 == ::access(mArrLogFileName.at(i).c_str(), F_OK))
        {
            if (0 != ::remove(mArrLogFileName.at(i).c_str()))
            {
                printf("ERROR: Failed to remove app log file: %s!\n", mArrLogFileName.at(i).c_str());
                return false;
            }
        }

        if (0 == ::access(mArrLogFileName.at(i - 1).c_str(), F_OK))
        {
            if (0 != ::rename(mArrLogFileName.at(i - 1).c_str(), mArrLogFileName.at(i).c_str()))
            {
                printf("ERROR: Failed to rename app log file: %s!\n", mArrLogFileName.at(i - 1).c_str());
                return false;
            }
        }
    }

    LogOpen();
    return true;
}
bool CLog::LogInit()
{
	mArrLogFileName.clear();
	char szPath[256] = {0};
    for (int i = 0; i < nFileNum; i++)
    {
        if (0 == i)
        {
            ::snprintf(szPath, 255, "%s/%s.log", mPath.c_str(), mName.c_str());
        }
        else
        {
            ::snprintf(szPath, 255, "%s/%s_%d.log", mPath.c_str(), mName.c_str(), i);
        }
        mArrLogFileName.push_back(szPath);
    }
    return true; 
}
