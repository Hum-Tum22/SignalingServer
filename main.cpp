// main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <signal.h>
#include "rutil/Socket.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"

#include "rutil/WinLeakCheck.hxx"
#include "SipServer.h"
#include "http.h"
#include "ws.h"
#include "SipServerConfig.h"
#include "SelfLog.h"

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::REPRO

using namespace sipserver;
using namespace resip;
using namespace std;

int main(int argc, char** argv)
{
    CLog::Instance().InitLog();
    // Initialize network
    initNetwork();

#if defined(WIN32) && defined(_DEBUG) && defined(LEAK_CHECK) 
    { FindMemoryLeaks fml;
#endif
    GetSipServerConfig();
    /*SipServer repro;
    if (!repro.run(argc, argv))
    {
        cerr << "Failed to start repro, exiting..." << endl;
        exit(-1);
    }
    HttpServer httpSv;
    repro.mainLoop();

    repro.shutdown();*/

    SipServer* pSipSvr = GetServer();
    if (pSipSvr)
    {
        if (!pSipSvr->run(argc, argv))
        {
            cerr << "Failed to start repro, exiting..." << endl;
            exit(-1);
        }
        //HttpServer httpSv(pSipSvr->gbHttpPort);
        WsServer s(9002);
        pSipSvr->mainLoop();

        pSipSvr->shutdown();
    }

#if defined(WIN32) && defined(_DEBUG) && defined(LEAK_CHECK) 
    }
#endif
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
