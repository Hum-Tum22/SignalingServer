// main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <signal.h>
#include <systemd/sd-daemon.h>
#include "rutil/Socket.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"

#include "rutil/WinLeakCheck.hxx"
#include "SipServer.h"
#include "http.h"
#include "ws.h"
#include "SipServerConfig.h"
#include "SelfLog.h"
#include "dbManager.h"
#include "JsonDevice.h"
#include "TypeConversion.h"
#include "IDManager.h"

#include <map>
#include <set>

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::REPRO

using namespace sipserver;
using namespace resip;
using namespace std;
void getJsonNvrChannelList(BaseDevice::Ptr dev, std::list<JsonChildDevic> &channelList);


int main(int argc, char** argv)
{
    if(argc >= 2)
    {
        if(std::string(argv[1]) == std::string("-v"))
        {
            printf("1.15.9\n");
            return 0;
        }
    }
    LogConfig LgConfig;
    LgConfig.nFileNum = 5;
    LgConfig.nFileSize = 5 * 1024 * 1024;
    int out = E_LOG_TARGET_BOTH;
    LgConfig.target = E_LOG_TARGET_BOTH;
    LgConfig.logPath = "./logs";
    LgConfig.logName = "app";
    std::map<int, std::string> logMdName;
    logMdName[SIPMSG] = "SIPMSG";
    logMdName[HTTP] = "HTTP";
    logMdName[CTRL] = "CTRL";
    logMdName[SDK] = "SDK";
    logMdName[MEDIA] = "MEDIA";
    logMdName[BLL] = "BLL";
    logMdName[CONFIG] = "CONFIG";
    logMdName[THREAD] = "THREAD";
    logMdName[DB] = "DB";
    
    LgConfig.mLogMod["SIPMSG"] = 5;
    LgConfig.mLogMod["HTTP"] = 5;
    LgConfig.mLogMod["CTRL"] = 5;
    LgConfig.mLogMod["SDK"] = 5;
    LgConfig.mLogMod["MEDIA"] = 5;
    LgConfig.mLogMod["BLL"] = 5;
    LgConfig.mLogMod["CONFIG"] = 5;
    LgConfig.mLogMod["THREAD"] = 5;
    LgConfig.mLogMod["DB"] = 5;
    CLog::Instance().setLogModName(logMdName);
    CLog::Instance().InitLog(&LgConfig);
    // Initialize network
    initNetwork();

#if defined(WIN32) && defined(_DEBUG) && defined(LEAK_CHECK) 
    { FindMemoryLeaks fml;
#endif
    // if(access("/usr/local/etc/cms/ID", F_OK) != 0)
    // {
    //     return 0;
    // }
    // FILE* fp = fopen("/usr/local/etc/cms/ID", "rb");
    // if(fp)
    // {
    //     int id = 0;
    //     fread((void*)&id, 4, 1, fp);
    //     fclose(fp);
    //     LogOut(BLL, L_INFO, "id:%d", id);
    //     if(id != 20260210)
    //     {
    //         return 0;
    //     }
    // }
    // else
    // {
    //     return 0;
    // }
    MyServerConfig& svrCfgi = GetSipServerConfig();
    /*SipServer repro;
    if (!repro.run(argc, argv))
    {
        cerr << "Failed to start repro, exiting..." << endl;
        exit(-1);
    }
    HttpServer httpSv;
    repro.mainLoop();

    repro.shutdown();*/
    resip::Data localGBID = svrCfgi.getConfigData("GBID", "34020000002000000001", true);
    CDevCodeMng::Instance().SetLocalGBID(localGBID.c_str());
    if(!CDbManager::Instance().initDb())
    {
        LogOut(BLL, L_ERROR, "db init failed");
        return -1;
    }
    else
    {
        std::list<std::shared_ptr<JsonNvrDevic>> devList;
        CDbManager::Instance().QueryDeviceInfoList(devList);
        for(auto &it: devList)
        {
            CDevCodeMng::Instance().AddDevCodeID(it->getGBID());
        }
        std::list<JsonChildDevic> chlList;
        CDbManager::Instance().QuerySubDeviceInfoList(chlList);
        for (auto &it: chlList)
        {
            CDevCodeMng::Instance().AddDevCodeID(it.getGBID());
        }

        // for(auto &it: devList)
        // {
        //     std::list<JsonChildDevic> channelList;
        //     getJsonNvrChannelList(it, channelList);
        //     for(auto &item : channelList)
        //     {
        //         JsonChildDevic channelInfo("");
        //         CDbManager::Instance().QuerySubDeviceInfo(it->deviceId, item.getChannel(), channelInfo);
        //         LogOut(BLL, L_INFO, "channel info nvrid:%s,nvrGBID:%s channel:%d, channelId:%s channelGBID:%s",
        //         it->deviceId.c_str(), it->getGBID().c_str(), item.getChannel(), channelInfo.getDeviceId().c_str(), channelInfo.getGBID().c_str());
        //         if(channelInfo.getDeviceId().empty())
        //         {
        //             std::string gbid = CDevCodeMng::Instance().CreateDevCode(IPC_CODE);
        //             int nRet = CDbManager::Instance().AddSubDeviceInfo(it->deviceId, gbid, item.getChannel(), item.getName());
        //             if(nRet != 0)
        //             {
        //                 CDevCodeMng::Instance().DelDevCodeID(gbid, 3);
        //             }
        //         }
        //     }
        // }
    }
    LogOut(BLL, L_ERROR, "start server!");
    SipServer* pSipSvr = GetServer();
    if (pSipSvr)
    {
        LogOut(BLL, L_ERROR, "server starting 1!");
        if (!pSipSvr->run(argc, argv))
        {
            cerr << "Failed to start repro, exiting..." << endl;
            exit(-1);
        }
        LogOut(BLL, L_ERROR, "server starting 2!");
        HttpServer httpSv(pSipSvr->gbHttpPort);
        LogOut(BLL, L_ERROR, "http server starting!");
        int wsPort = svrCfgi.getConfigInt("WS_PORT", 9899);
        WsServer s(wsPort);
        LogOut(BLL, L_ERROR, "ws server starting!");
        // 通知systemd服务已准备好
        sd_notify(0, "READY=1");
        // 如果服务支持reload，还可以发送这个信号
        sd_notify(0, "WATCHDOG=1");
        pSipSvr->mainLoop();

        pSipSvr->shutdown();
    }

#if defined(WIN32) && defined(_DEBUG) && defined(LEAK_CHECK) 
    }
#endif
    LogOut(BLL, L_ERROR, "end!");
    // 服务结束前通知systemd
    sd_notify(0, "STOPPING=1");
    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
