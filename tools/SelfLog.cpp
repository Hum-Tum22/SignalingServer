#include "SelfLog.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <string.h>
#include <chrono>

#ifdef _MSC_VER
#include <io.h>
//#define F_OK 0
#endif
#if (defined(_WIN32) || defined(_WIN64))
#define localtime_r(a,b) localtime_s(b,a)
#define SetThreadName(pThName)
#define log_access(file) _access(file, 0)
#else
#include <unistd.h>
#include <sys/prctl.h>
#define SetThreadName(pThName) prctl(PR_SET_NAME, pThName)
#define log_access(file) ::access(file, F_OK)
#endif

struct logColorInfo
{
    const char *name;
    const char *color;
};
static const logColorInfo gColorInfo[LEVEL_NUM] = {
    {"F", LIGHT_RED},
    {"E", RED},
    {"W", YELLOW},
    {"N", BLUE},
    {"I", GREEN},
    {"D", GRAY},
    {"S", LIGHT_YELLOW},
    {"T", LIGHT_BLUE},
    {"P", LIGHT_GREEN} };

static std::map<LOG_MODULE, std::string> gLogMods = {
    { LOGMD, "LOGMD" },
};

CLog &CLog::Instance()
{
    static CLog g_Clog;
    return g_Clog;
}
CLog::CLog()
    :nFileNum(5)
    , nFileSize(1 * 1024 * 1024)
    , nCurFileSize(0)
    , nTarget(E_LOG_TARGET_FILE)
    , mPath("./logs")
    , mName("app")
    , nNewLinelen(strlen(LOG_NEWLINE))
    , mRunStatus(0)
    , mLogNum(0)
{
    //map先完成初始化,后续不进行增删,则可以不用加锁;新增日志模块必须在此完成初始中化;  在此完成初始化,后续调用SetLogLevel就不会发生冲突
    mLogVc.resize(MAX_LOG_MODULE);
    mLogVc[LOGMD].name = "LOGMD";
    mLogVc[LOGMD].value = L_DEBUG;
}
CLog::~CLog()
{
    StopLog();
}

void CLog::OutPut(int mod, int level, const char* fileName, int lineNo, const char* msg, ...)
{
    if(mod >= MAX_LOG_MODULE || mod < 0)
    {
        OutPutCall(LOGMD, L_ERROR, FILENAME, __LINE__, "log name mode not exist :%d", mod);
        return;
    }
    if(mLogVc[mod].value >= level)
    {
        //(2023-03-31 16:39:21,130)[Common-INFO-commonTool.cpp:231]
        //Dev.cpp:1305]

        //(2023-03-31 16:39:21,130)[Common-INFO-

        //struct timeval current;
        //gettimeofday(&current, NULL);
        long long curtime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        char strTime[20] = { 0 };
        time_t cur_t = curtime / 1000;

        struct tm tmtime;
        memset(&tmtime, 0, sizeof(tmtime));
        localtime_r(&cur_t, &tmtime);
        strftime(strTime, sizeof(strTime), "%Y-%m-%d %H:%M:%S", &tmtime);


        char tmp[1024 * 50] = { 0 };
        int size = 0, tmpSize = sizeof(tmp) - 1 - nNewLinelen;

        size += ::snprintf(&tmp[size], tmpSize - size - 1, "%s.%03d[%s-%s-%s:%d]:", strTime, int(curtime % 1000),
                           mLogVc[mod].name.c_str(), levelName(level), fileName, lineNo);
        va_list args;
        va_start(args, msg);
        int needlen = ::vsnprintf(NULL, 0, msg, args);
        va_end(args);
        if(needlen + 1 > tmpSize - size)
        {
            needlen = tmpSize - size;
        }
        va_start(args, msg);
        int formatlen = ::vsnprintf(&tmp[size], needlen + 1, msg, args);
        va_end(args);
        size += needlen;
        if(needlen != formatlen)
        {
            OutPutCall(LOGMD, L_WARN, FILENAME, __LINE__, "formatlen:%d needlen:%d", formatlen, needlen);
        }

        if(E_LOG_TARGET_STDERR == nTarget || E_LOG_TARGET_BOTH == nTarget)
        {
            printf("%s%s%s\n", levelColor(level), tmp, NONE__);
        }
        if(E_LOG_TARGET_FILE == nTarget || E_LOG_TARGET_BOTH == nTarget)
        {
            logInfo log(tmp, size, level);
            bool isEmpty = true;
            {
                std::unique_lock<std::mutex> lock(logMtx);
                mLogs.push_back(std::move(log));
                isEmpty = mLogs.empty();
            }
            mLogNum.fetch_add(1, std::memory_order_relaxed);
            if(mLogNum.load() > 100)
            {
                cv.notify_one();
                OutPutCall(LOGMD, L_WARN, FILENAME, __LINE__, "mLogs not empty: %u:%d file name:%s", mLogNum.load(),
                           isEmpty, mName.c_str());
            }
            cv.notify_one();
        }
    }
}

void CLog::StopLog()
{
    if(mRunStatus == 1)
    {
        while(1)
        {
            {
                if(mLogNum.load() == 0)
                {
                    break;
                }
                cv.notify_all();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        mRunStatus = 2;
        if(mLogThread.joinable())
        {
            mLogThread.join();
        }
    }
    mRunStatus = 0;
}

void CLog::InitLog(LogConfig* config)
{
    LOGOUT((*this), LOGMD, L_FATAL, "config %p", config);
    if(config)
    {
        nFileNum = config->nFileNum > 0 ? config->nFileNum : 5;
        nFileSize = config->nFileSize > 0 ? config->nFileSize : 1 * 1024 * 1024;
        nTarget = config->target;
        if(!config->logPath.empty())
            mPath = config->logPath;
        if(!config->logName.empty())
            mName = config->logName;
        LOGOUT((*this), LOGMD, L_FATAL, "fileNum = %d", nFileNum);
        LOGOUT((*this), LOGMD, L_FATAL, "fileSize = %ld", nFileSize);
        LOGOUT((*this), LOGMD, L_FATAL, "target = %d", nTarget);
        LOGOUT((*this), LOGMD, L_FATAL, "log_path = %s", mPath.c_str());
        LOGOUT((*this), LOGMD, L_FATAL, "log_name = %s", mName.c_str());


        for(auto& it : config->mLogMod)
        {
            for(auto& mod : gLogMods)
            {
                if(mod.second == it.first)
                {
                    LOG_MODULE logMd = (LOG_MODULE)mod.first;
                    if (logMd > LOGMD && logMd < MAX_LOG_MODULE)
                    {
                        mLogVc[logMd].name = it.first;
                        mLogVc[logMd].value = it.second;
                        LOGOUT((*this), LOGMD, L_FATAL, "log mod %s level %d", it.first.c_str(), it.second);
                    }
                    break;
                }
            }
        }
    }
    LOGOUT((*this), LOGMD, L_FATAL, "log thread status:%d %u file name:%s", mRunStatus, mLogNum.load(), mName.c_str());
    if(mRunStatus == 0)
    {
        mRunStatus = 1;
        mLogThread = std::thread(&CLog::writeLogThread, this);
    }
}

void CLog::setLogModName(std::map<int, std::string> logMdName)
{
    for(auto& it : logMdName)
    {
        if ((LOG_MODULE)it.first > LOGMD)
        {
            gLogMods[(LOG_MODULE)it.first] = it.second;
        }
    }
    for (int i = 0; i < MAX_LOG_MODULE; i++)
    {
        if (i > LOGMD && !gLogMods[(LOG_MODULE)i].empty())
        {
            mLogVc[i].name = gLogMods[(LOG_MODULE)i];
        }
    }
}

void CLog::reNameFile()
{
    if((int)mArrLogFileName.size() == nFileNum)
    {
        for(int i = nFileNum - 1; i > 0; i--)
        {
            if(nFileNum - 1 == i && 0 == log_access(mArrLogFileName.at(i).c_str()))
            {
                if(0 != ::remove(mArrLogFileName.at(i).c_str()))
                {
                    threadOut(LOGMD, L_INFO, FILENAME, __LINE__, "ERROR: Failed to remove app log file: %s!",
                              mArrLogFileName.at(i).c_str());
                    return;
                }
            }

            if(0 == log_access(mArrLogFileName.at(i - 1).c_str()))
            {
                if(0 != ::rename(mArrLogFileName.at(i - 1).c_str(), mArrLogFileName.at(i).c_str()))
                {
                    threadOut(LOGMD, L_INFO, FILENAME, __LINE__, "ERROR: Failed to rename app log file: %s!",
                              mArrLogFileName.at(i - 1).c_str());
                    return;
                }
            }
        }
    }
}

void CLog::writeLogThread()
{
    SetThreadName("Log");
    auto timeout = std::chrono::milliseconds(200);
    bool isEmpty = true;
    logInfo log;
    LOGOUT((*this), LOGMD, L_INFO, "log thread status:%d, %u, %d file name:%s", mRunStatus, mLogNum.load(), mLogs.empty(), mName.c_str());
    while(mRunStatus == 1)
    {
        {
            std::unique_lock<std::mutex> lock(logMtx);
            cv.wait_for(lock, timeout, [&] { return mLogNum.load() > 0; });
            isEmpty = mLogs.empty();
            if(isEmpty)
                continue;
            log = std::move(mLogs.front()); // 取出队首元素，返回队首元素值，并进行右值引用
            mLogs.pop_front(); // 弹出入队的第一个元素
        }
        if (mLogNum.load() > 0)
        {
            mLogNum.fetch_sub(1, std::memory_order_relaxed);
        }
        else
        {
            std::unique_lock<std::mutex> lock(logMtx);
            mLogNum.store(mLogs.size());
        }
        // threadOut(LOGMD, L_INFO, FILENAME, __LINE__, "log num:%u, %d file name:%s", mLogNum.load(), isEmpty,
        // mName.c_str());

        if(!log.log.empty())
        {
            log.log.append(LOG_NEWLINE);
            bool bRet = LogWrite(log.log.c_str(), log.log.size(), log.level);
            if(!bRet)
            {
                closeFile();
                // std::unique_lock<std::mutex> lock(vctMtx);
                threadOut(LOGMD, L_INFO, FILENAME, __LINE__, "Write app log file failed:%s",
                          mArrLogFileName.at(0).c_str());
            }
        }
    }
    threadOut(LOGMD, L_INFO, FILENAME, __LINE__, "log thread exit!");
    closeFile();
}

void CLog::SetLogLevel(int mod, int level)
{
    if(level < L_FATAL || level > LEVEL_NUM || mod < 0 || mod >= MAX_LOG_MODULE)
    {
        LOGOUT((*this), LOGMD, L_FATAL, "CLog set level %s,%d", mLogVc[mod].name.c_str(), level);
        return;
    }
    mLogVc[mod].value = level;
    LOGOUT((*this), LOGMD, L_FATAL, "set log %s = %d", mLogVc[mod].name.c_str(), level);
}
void CLog::SetLogTarget(int eTarget)
{
    if(eTarget >= E_LOG_TARGET_NONE && eTarget < E_LOG_TARGET_MAX)
    {
        nTarget = (LOG_TARGET)eTarget;
        LOGOUT((*this), LOGMD, L_FATAL, "CLog set target:%d", eTarget);
    }
}

void CLog::SetLogFileSize(int size)
{
    if(size > 0)
    {
        nFileSize = size * 1024 * 1024;
        LOGOUT((*this), LOGMD, L_FATAL, "CLog set file size:%dM", size);
    }
}
void CLog::SetLogFileNum(int num)
{
    if(nFileNum != num)
    {
        nFileNum = num;
    }
    LOGOUT((*this), LOGMD, L_FATAL, "CLog set file num:%d", num);
}

bool CLog::LogOpen()
{
    LogInit();

    if(nFileNum < 1 || pLogFile)
    {
        return false;
    }
    if(openFile(mArrLogFileName.at(0).c_str()) == 0)
    {
        return true;
    }
    return false;
}

bool CLog::LogWrite(const char* szLog, size_t nSize, int Level)
{
    if(!pLogFile || nCurFileSize + nSize > nFileSize)
    {
        if(!SwitchFile())
        {
            threadOut(LOGMD, L_INFO, FILENAME, __LINE__, "SwitchFile failed");
            return false;
        }
    }
    size_t wlen = writeFile(szLog, nSize);
    if (Level <= L_ERROR)
    {
        flush();
    }
    return nSize == wlen ? true : false;
}
bool CLog::SwitchFile()
{
    closeFile();
    reNameFile();
    return LogOpen();
}
bool CLog::LogInit()
{
    if(nFileNum != (int)mArrLogFileName.size())
    {
        createDir(mPath.c_str());
        char szPath[256] = { 0 };
        mArrLogFileName.resize(nFileNum);
        for(int i = 0; i < nFileNum; i++)
        {
            if(0 == i)
            {
                ::snprintf(szPath, 255, "%s/%s.log", mPath.c_str(), mName.c_str());
            }
            else
            {
                ::snprintf(szPath, 255, "%s/%s_%d.log", mPath.c_str(), mName.c_str(), i);
            }
            mArrLogFileName.at(i) = szPath;
            threadOut(LOGMD, L_INFO, FILENAME, __LINE__, "push file:%s", szPath);
        }
        threadOut(LOGMD, L_INFO, FILENAME, __LINE__, "mArrLogFileName size:%lu", mArrLogFileName.size());
    }
    return true;
}

int CLog::openFile(const char* fileName)
{
    if(pLogFile == nullptr)
    {
        pLogFile = fopen(fileName, "a+");
        if(pLogFile != nullptr)
        {
            fseek(pLogFile, 0, SEEK_END);
            nCurFileSize = ftell(pLogFile);
            return 0;
        }
        int errnum = errno;
        threadOut(LOGMD, L_INFO, FILENAME, __LINE__, " fopen failed err:%d %s %s", errnum, strerror(errnum), fileName);
        return -1;
    }
    return 0;
}
size_t CLog::writeFile(const char buffer[], size_t size)
{
    if(pLogFile != nullptr)
    {
        size_t wSize = fwrite(buffer, 1, size, pLogFile);
        if(wSize > 0)
        {
            nCurFileSize += wSize;
        }
        if(wSize < size)
        {
            int errnum = errno;
            threadOut(LOGMD, L_INFO, FILENAME, __LINE__, " fwrite failed err:%d %s", errnum, strerror(errnum));
        }
        return wSize;
    }
    return 0;
}
int CLog::closeFile()
{
    if(pLogFile != nullptr)
    {
        fflush(pLogFile);
        threadOut(LOGMD, L_INFO, FILENAME, __LINE__, " close handle.fileFp:%p", pLogFile);
        fclose(pLogFile);
        pLogFile = nullptr;
        return 0;
    }
    return -1;
}
void CLog::flush()
{
    if(pLogFile != nullptr)
    {
        fflush(pLogFile);
    }
}

void CLog::threadOut(int mod, int level, const char* fileName, int lineNo, const char* msg, ...)
{
    if(mod >= MAX_LOG_MODULE || mod < 0)
    {
        printf("log name mode not exist :%d\n", mod);
        return;
    }
    if(mLogVc[mod].value >= level)
    {
        //(2023-03-31 16:39:21,130)[Common-INFO-commonTool.cpp:231]
        //Dev.cpp:1305]

        //(2023-03-31 16:39:21,130)[Common-INFO-

        //struct timeval current;
        //gettimeofday(&current, NULL);
        long long curtime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        char strTime[20] = { 0 };
        time_t cur_t = curtime / 1000;

        struct tm tmtime;
        memset(&tmtime, 0, sizeof(tmtime));
        localtime_r(&cur_t, &tmtime);
        strftime(strTime, sizeof(strTime), "%Y-%m-%d %H:%M:%S", &tmtime);


        char tmp[1024 * 50] = { 0 };
        int size = 0, tmpSize = sizeof(tmp) - 1 - nNewLinelen;

        size += ::snprintf(&tmp[size], tmpSize - size - 1, "%s.%03d[%s-%s-%s:%d]:", strTime, int(curtime % 1000),
                           mLogVc[mod].name.c_str(), levelName(level), fileName, lineNo);
        va_list args;
        va_start(args, msg);
        int needlen = ::vsnprintf(NULL, 0, msg, args);
        va_end(args);
        if(needlen + 1 > tmpSize - size)
        {
            needlen = tmpSize - size;
        }
        va_start(args, msg);
        int formatlen = ::vsnprintf(&tmp[size], needlen + 1, msg, args);
        va_end(args);
        size += needlen;
        if(needlen != formatlen)
        {
            printf("formatlen:%d needlen:%d\n", formatlen, needlen);
        }

        if(E_LOG_TARGET_STDERR == nTarget || E_LOG_TARGET_BOTH == nTarget)
        {
            printf("%s%s%s\n", levelColor(level), tmp, NONE__);
        }
        if(E_LOG_TARGET_FILE == nTarget || E_LOG_TARGET_BOTH == nTarget)
        {
            writeFile(tmp, size);
            writeFile(LOG_NEWLINE, nNewLinelen);
            if (level <= L_ERROR)
            {
                flush();
            }
        }
    }
}
void CLog::OutPutCall(int mod, int level, const char* fileName, int lineNo, const char* msg, ...)
{
    //(2023-03-31 16:39:21,130)[Common-INFO-commonTool.cpp:231]
    //Dev.cpp:1305]

    //(2023-03-31 16:39:21,130)[Common-INFO-

    //struct timeval current;
    //gettimeofday(&current, NULL);
    long long curtime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    char strTime[20] = { 0 };
    time_t cur_t = curtime / 1000;

    struct tm tmtime;
    memset(&tmtime, 0, sizeof(tmtime));
    localtime_r(&cur_t, &tmtime);
    strftime(strTime, sizeof(strTime), "%Y-%m-%d %H:%M:%S", &tmtime);


    char tmp[1024 * 50] = { 0 };
    int size = 0, tmpSize = sizeof(tmp) - 1 - nNewLinelen;

    size += ::snprintf(&tmp[size], tmpSize - size - 1, "%s.%03d[%s-%s-%s:%d]:", strTime, int(curtime % 1000),
                       mLogVc[mod].name.c_str(), levelName(level), fileName, lineNo);
    va_list args;
    va_start(args, msg);
    int needlen = ::vsnprintf(NULL, 0, msg, args);
    va_end(args);
    if(needlen + 1 > tmpSize - size)
    {
        needlen = tmpSize - size;
    }
    va_start(args, msg);
    int formatlen = ::vsnprintf(&tmp[size], needlen + 1, msg, args);
    va_end(args);
    size += needlen;
    if(needlen != formatlen)
    {
        printf("formatlen:%d needlen:%d\n", formatlen, needlen);
    }

    if(E_LOG_TARGET_STDERR == nTarget || E_LOG_TARGET_BOTH == nTarget)
    {
        printf("%s%s%s\n", levelColor(level), tmp, NONE__);
    }
    if(E_LOG_TARGET_FILE == nTarget || E_LOG_TARGET_BOTH == nTarget)
    {
        logInfo log(tmp, size, level);
        {
            std::unique_lock<std::mutex> lock(logMtx);
            mLogs.push_back(std::move(log));
        }
        mLogNum.fetch_add(1, std::memory_order_relaxed);
        cv.notify_one();
    }
}

void CLog::createDir(const char* muldir)
{
    // printf("mkdir -p %s\n", muldir);
#ifndef _WIN32
    size_t i, len = strlen(muldir);
    char str[1024] = { 0 };
    snprintf(str, sizeof(str), "%s", muldir);
    // strncpy(str, muldir, len < 1024 ? len : 1024 - 1);
    len = strlen(str);
    for(i = 0; i < len; i++)
    {
        if(str[i] == '/')
        {
            str[i] = '\0';
            if(access(str, F_OK) != 0)
            {
                mkdir(str, 0775);
            }
            str[i] = '/';
        }
    }
    if(len > 0 && access(str, F_OK) != 0)
    {
        mkdir(str, 0775);
    }
    return;
#endif
}

const char* CLog::levelName(int level)
{
    if (level >= 0 && level < LEVEL_NUM)
    {
        return gColorInfo[level].name;
    }
    return gColorInfo[1].name;
}

const char* CLog::levelColor(int level)
{
    if (level >= 0 && level < LEVEL_NUM)
    {
        return gColorInfo[level].color;
    }
    return gColorInfo[1].color;
}