#ifndef _LOG_H_
#define _LOG_H_


#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <thread>
#include <tuple>
#include "InitFile.h"
#include "SafeQueue.h"

#define L_FATAL         0
#define L_ERROR         1
#define L_WARN          2
#define L_NOTICE        3
#define L_INFO          4
#define L_DEBUG         5
#define L_TRACE         6
#define L_TRACE1        7
#define L_TRACE2        8
#define LEVEL_NUM     	9

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
#define LOG_NEWLINE		"\n"
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
    E_LOG_LV_FATAL      = 0,
    E_LOG_LV_ERROR      = 1,
    E_LOG_LV_WARN      	= 2,
    E_LOG_LV_NOTICE		= 3,
    E_LOG_LV_INFO       = 4,
    E_LOG_LV_DEBUG      = 5,
} APP_LOG_LEVEL;

typedef enum
{
    E_LOG_TARGET_NONE   = 0,
    E_LOG_TARGET_STDERR = 1,
    E_LOG_TARGET_FILE   = 2,
    E_LOG_TARGET_BOTH   = 3,
    E_LOG_TARGET_MAX    = 4
} APP_LOG_TARGET;

class CLog
{
	int nFileNum;
	long nFileSize;
	APP_LOG_TARGET nTarget;
	FILE* mLogFilePtr;
	std::string mPath;
    std::string mName;
	std::map<std::string,int> mLogMap;
	std::vector<std::string> mArrLogFileName;
	int nNoneColorlen;
	int nNewLinelen;
	long mCurFileSize;
	IniFile mLogConfig;
    std::thread mLogThread;
    bool mRunStatus;
    SafeQueue<std::tuple<std::string, int>> mLogs;
    bool changeFile;
public:
	CLog();
	~CLog();
    static CLog& Instance();
	void Output(const std::string name, int level, std::string &padmsg, const char* msg, ...);
	void StopLog();
	void InitLog();
public:
	void SetLogLevel(std::string name, int level);
    void SetLogTarget(int eTarget);
	void SetLoggerPath(const char* path, const char* name);
	void SetLogFileSize(int size);//单位M
	void SetLogFileNum(int num);

	int GetLogLevel(const std::string name);
	int GetLogTarget();
	int GetLogFileSize();
	int GetLogFileNum();
	void GetLogPathName(std::string &path, std::string &name);

private:
	bool LogOpen();
	void LogClose();
    bool LogWrite(const char* szLog, int nSize, int nLevel);
	bool SwitchFile();
	bool LogInit();
    void writeLogThread();
};


#define _S_LINE(x) #x
#define __S_LINE(x) _S_LINE(x)
#define __S_LINE__ __S_LINE(__LINE__)
#if (defined(_WIN32) || defined(_WIN64))
#define FILENAME(x) strrchr(x,'\\')?strrchr(x,'\\')+1:x
#elif (defined(__linux__))
#define FILENAME(x) strrchr(x,'/')?strrchr(x,'/')+1:x
#endif
#define catMsg(msg) std::string().append(FILENAME(__FILE__)).append(":").append(__S_LINE__).append("]:").append(msg)
#define padMsg() std::string().append(FILENAME(__FILE__)).append(":").append(__S_LINE__).append("]:")

//参数1： 模块名称：Common、ProxyCli、ProxySvr、Ws、Msg
//参数2： 日志等级
//参数3： 日志格式化字符串
//输出参数
#define LogOut(name, level, msg, ...)  CLog::Instance().Output(name,level,padMsg(),msg,##__VA_ARGS__);

#define LogLevelSet(name, level)  CLog::Instance().SetLogLevel(name,level);
#define LogTargetSet(target)  CLog::Instance().SetLogTarget(target);
#define LogPathNameSet(path, file)  CLog::Instance().SetLoggerPath(path, file);
#define LogFileSizeSet(size)  CLog::Instance().SetLogFileSize(size);//单位M
#define LogFileNumSet(num)  CLog::Instance().SetLogFileNum(num);

#define LogLevelGet(name)  CLog::Instance().GetLogLevel(name);
#define LogTargetGet()  CLog::Instance().GetLogTarget();
#define LogPathNameGet(path, file)  CLog::Instance().GetLogPathName(path, file);
#define LogFileSizeGet()  CLog::Instance().GetLogFileSize();
#define LogFileNumGet()  CLog::Instance().GetLogFileNum();


#define LOG_W(fmt, ...)     CLog::Instance().Output("SDK",L_FATAL,padMsg(),fmt,##__VA_ARGS__);
#define LOG_E(fmt, ...)     CLog::Instance().Output("SDK",L_ERROR,padMsg(),fmt,##__VA_ARGS__);
#define LOG_I(fmt, ...)     CLog::Instance().Output("SDK",L_INFO,padMsg(),fmt,##__VA_ARGS__);
#define LOG_N(fmt, ...)     CLog::Instance().Output("SDK",L_NOTICE,padMsg(),fmt,##__VA_ARGS__);
#define LOG_D(fmt, ...)     CLog::Instance().Output("SDK",L_DEBUG,padMsg(),fmt,##__VA_ARGS__);
#define LOGGER(l, fmt, ...)   CLog::Instance().Output("SDK",l,padMsg(),fmt,##__VA_ARGS__);


#endif /* _APP_LOG_71158D90_7332_48F2_9200_2239312AA03D_H_ */

