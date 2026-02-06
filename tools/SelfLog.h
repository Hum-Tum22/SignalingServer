#ifndef _LOG_H_
#define _LOG_H_

#ifdef HAS_FORMAT_H
#include <format>
#endif

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <string.h>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>

#if (defined(_WIN32) || defined(_WIN64))
#define FILENAME strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__
#elif (defined(__linux__))
#define FILENAME strrchr((__FILE__), '/') ? strrchr((__FILE__), '/') + 1 : (__FILE__)
#endif

#if (defined(_WIN32) || defined(_WIN64))
#define NONE__          ""
#define RED             ""
#define LIGHT_RED       ""
#define GREEN           ""
#define LIGHT_GREEN     ""
#define YELLOW          ""
#define LIGHT_YELLOW    ""
#define BLUE            ""
#define LIGHT_BLUE      ""
#define GRAY            ""
#define LOG_NEWLINE "\r\n"
#elif (defined(__linux__))
#define NONE__          "\033[0m"
#define RED             "\033[0;32;31m"
#define LIGHT_RED       "\033[1;31m"
#define GREEN           "\033[0;32;32m"
#define LIGHT_GREEN     "\033[1;32m"
#define YELLOW          "\033[0;32;33m"
#define LIGHT_YELLOW    "\033[1;33m"
#define BLUE            "\033[0;32;34m"
#define LIGHT_BLUE      "\033[1;34m"
#define GRAY            "\033[0;32;37m"
#define LOG_NEWLINE		"\n"
#endif

typedef enum
{
    LOGMD, ///< LOGMD 日志模块专用
    MOD1,
    MOD2,
    MOD3,
    MOD4,
    MOD5,
    MOD6,
    MOD7,
    MOD8,
    MOD9,
    MOD10,
    MOD11,
    MOD12,
    MOD13,
    MOD14,
    MOD15,
    MOD16,
    MOD17,
    MOD18,
    MOD19,
    MOD20,
    MOD21,
    MOD22,
    MOD23,
    MOD24,
    MOD25,
    MOD26,
    MOD27,
    MOD28,
    MOD29,
    MOD30,
    MAX_LOG_MODULE,
} LOG_MODULE;

typedef enum
{
    SIPMSG = MOD1,
    HTTP = MOD2,
    CTRL = MOD3,
    SDK = MOD4,
    MEDIA = MOD5,
    BLL = MOD6,
    CONFIG = MOD7,
    THREAD = MOD8,
    CUSTOM_MAX_LOG_MODULE,
} CUSTOM_LOG_MODULE; ///< 该枚举定义可以跟据需要自由修改

typedef enum
{
    L_FATAL = 0,
    L_ERROR = 1,
    L_WARN = 2,
    L_NOTICE = 3,
    L_INFO = 4,
    L_DEBUG = 5,
    L_TRACE = 6,
    L_TRACE1 = 7,
    L_TRACE2 = 8,
    LEVEL_NUM = 9
} LOG_LEVEL;

typedef enum
{
    E_LOG_TARGET_NONE = 0,
    E_LOG_TARGET_STDERR = 1,
    E_LOG_TARGET_FILE = 2,
    E_LOG_TARGET_BOTH = 3,
    E_LOG_TARGET_MAX = 4
} LOG_TARGET;
struct logInfo
{
    std::string log;
    int level;
    logInfo() :level(0) {};
    logInfo(const char* buf, int size, int lv) :log(buf, size), level(lv) {};
};

struct LogKeyVlue
{
    std::string name;
    int value;
    LogKeyVlue() : value(0) {}
};
typedef struct
{
    int nFileNum;
    uint64_t nFileSize;
    LOG_TARGET target;
    std::string logPath;
    std::string logName;
    std::unordered_map<std::string, int> mLogMod;
}LogConfig;

class CLog
{
    int nFileNum;
    size_t nFileSize;
    size_t nCurFileSize;
    LOG_TARGET nTarget;
    FILE* pLogFile;
    std::string mPath;
    std::string mName;
    std::vector<LogKeyVlue> mLogVc;
    std::mutex vctMtx;
    std::vector<std::string> mArrLogFileName;
    int nNewLinelen;
    std::thread mLogThread;
    int mRunStatus;
    std::mutex logMtx;
    std::condition_variable cv;
    std::list<logInfo> mLogs;
    std::atomic<uint32_t> mLogNum;
public:
    CLog();
    ~CLog();
    static CLog& Instance();
#ifdef _WIN32
    void OutPut(int mod, int level, const char* fileName, int lineNo, const char* msg, ...);
#else
    void OutPut(int mod, int level, const char* fileName, int lineNo, const char* msg, ...)
        __attribute__((format(printf, 6, 7)));
#endif
    template <typename... Args>
    void CPPOutPut(int mod, int level, const char* fileName, int lineNo, const char* fmt, Args&&... args)
    {
        if (mod >= MAX_LOG_MODULE || mod < 0)
        {
            OutPutCall(LOGMD, L_ERROR, FILENAME, __LINE__, "log name mode not exist :%d", mod);
            return;
        }
        if (mLogVc[mod].value >= level)
        {
            //(2023-03-31 16:39:21,130)[Common-INFO-commonTool.cpp:231]

            // struct timeval current;
            // gettimeofday(&current, NULL);
            long long curtime = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count();
            time_t cur_t = curtime / 1000;

            struct tm tmtime;
            memset(&tmtime, 0, sizeof(tmtime));
            localtime_r(&cur_t, &tmtime);
            char time_str[64] = { 0 };
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tmtime);

            std::string strLogLine;
#if defined(HAS_FORMAT_H)
            strLogLine = std::format("{}.{:03d}[{}-{}-{}:{}]:", time_str, int(curtime % 1000), mLogVc[mod].name.c_str(),
                                     levelName(level), fileName, lineNo);
            strLogLine.append(std::vformat(fmt, std::make_format_args(args...)));
#else
            return;
#endif

            if (E_LOG_TARGET_STDERR == nTarget || E_LOG_TARGET_BOTH == nTarget)
            {
                std::cerr << levelColor(level) << strLogLine << NONE__ << LOG_NEWLINE;
            }
            if (E_LOG_TARGET_FILE == nTarget || E_LOG_TARGET_BOTH == nTarget)
            {
                logInfo log;
                log.log = std::move(strLogLine);
                log.level = level;
                bool isEmpty = true;
                {
                    std::unique_lock<std::mutex> lock(logMtx);
                    mLogs.push_back(std::move(log));
                    isEmpty = mLogs.empty();
                }
                mLogNum.fetch_add(1, std::memory_order_relaxed);
                if (mLogNum.load() > 100)
                {
                    OutPutCall(LOGMD, L_WARN, FILENAME, __LINE__, "mLogs not empty: %u:%d file name:%s", mLogNum.load(),
                               isEmpty, mName.c_str());
                }
                cv.notify_one();
            }
        }
    }

    void StopLog();
    // void InitLog(tools::IniFile* config = NULL);
    void InitLog(LogConfig* config = NULL);
    // 设置自定义日志模块名称;应在CLog创建后第一时间调用; logMdName初始化为{{CUSTOM_LOG_MODULE, ""}, ...}
    void setLogModName(std::map<int, std::string> logMdName);

public:
    void SetLogLevel(int mod, int level);
    void SetLogTarget(int eTarget);
    void SetLogFileSize(int size);//单位M
    void SetLogFileNum(int num);

private:
    bool LogOpen();
    bool LogWrite(const char *szLog, size_t nSize, int nLevel);
    bool SwitchFile();
    bool LogInit();
    void reNameFile();
    void writeLogThread();

    int openFile(const char* fileName);
    size_t writeFile(const char buffer[], size_t size);
    int closeFile();
    void flush();
#ifdef _WIN32
    // writeLogThread 线程中专用
    void threadOut(int mod, int level, const char* fileName, int lineNo, const char* msg, ...);
    // OutPut 函数内调用
    void OutPutCall(int mod, int level, const char* fileName, int lineNo, const char* msg, ...);
#else
    // writeLogThread 线程中专用
    void threadOut(int mod, int level, const char* fileName, int lineNo, const char* msg, ...)
        __attribute__((format(printf, 6, 7)));
    // OutPut 函数内调用
    void OutPutCall(int mod, int level, const char* fileName, int lineNo, const char* msg, ...)
        __attribute__((format(printf, 6, 7)));
#endif
    void createDir(const char* muldir);

    const char* levelName(int);
    const char* levelColor(int);
};

/**
 *@brief 日志输出宏
 * @param[in]          mod                                  日志模块
 * @param[in]          lv                                   日志等级
 * @param[in]          fmt                                  格式化字符串
 * @param[in]          ...                                  格式化参数
 */
#define LogOut(mod, lv, fmt, ...) CLog::Instance().OutPut(mod, lv, FILENAME, __LINE__, fmt, ##__VA_ARGS__);
/**
 *@brief 日志输出宏
 * @param[in]          clog                                 CLog类对象
 * @param[in]          mod                                  日志模块
 * @param[in]          lv                                   日志等级
 * @param[in]          fmt                                  格式化字符串
 * @param[in]          ...                                  格式化参数
 */
#define LOGOUT(clog, mod, lv, fmt, ...) clog.OutPut(mod, lv, FILENAME, __LINE__, fmt, ##__VA_ARGS__);

/**
 *@brief 支持c++20或fmt格式化的日志输出宏
 * @param[in]          mod                                  日志模块
 * @param[in]          lv                                   日志等级
 * @param[in]          fmt                                  格式化字符串
 * @param[in]          ...                                  格式化参数
 */
#define LOG(mod, level, fmt, ...) CLog::Instance().CPPOutPut(mod, level, FILENAME, __LINE__, fmt, ##__VA_ARGS__);
#define Log(clog, mod, level, fmt, ...) clog.CPPOutPut(mod, level, FILENAME, __LINE__, fmt, ##__VA_ARGS__);

#define LogLevelSet(mod, level)  CLog::Instance().SetLogLevel(mod,level);
#define LogTargetSet(target)  CLog::Instance().SetLogTarget(target);
#define LogFileSizeSet(size)  CLog::Instance().SetLogFileSize(size);//单位M
#define LogFileNumSet(num)  CLog::Instance().SetLogFileNum(num);

#define LogLevelGet(name)  CLog::Instance().GetLogLevel(name);
#define LogTargetGet()  CLog::Instance().GetLogTarget();
#define LogPathNameGet(path, file)  CLog::Instance().GetLogPathName(path, file);
#define LogFileSizeGet()  CLog::Instance().GetLogFileSize();
#define LogFileNumGet()  CLog::Instance().GetLogFileNum();


#endif

