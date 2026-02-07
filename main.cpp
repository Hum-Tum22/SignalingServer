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

#include <map>
#include <set>

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::REPRO

using namespace sipserver;
using namespace resip;
using namespace std;
void getJsonNvrChannelList(BaseDevice::Ptr dev, std::list<JsonChildDevic> &channelList);
template <class T>
class CHnaldeMrg
{
public:
	CHnaldeMrg(T ui_start,T uimax):m_uistart(ui_start),m_uiMax(uimax),CurIncrHandle(ui_start){};
	virtual ~CHnaldeMrg(){};
	T GetHandle(){
		CurIncrHandle++;
		if(CurIncrHandle>=m_uiMax)
		{
			CurIncrHandle = m_uistart;
		}
		return CurIncrHandle;
	};
	T GetHandle_R(){
		while(m_handlemap.count(CurIncrHandle) != 0)
		{
				if(CurIncrHandle>=m_uiMax)
				{
					CurIncrHandle = m_uistart;
				}
				CurIncrHandle++;
		}
		m_handlemap.insert(CurIncrHandle);
		std::cout << "GetHandle_R " << " CurIncrHandle:" << CurIncrHandle << std::endl;
		return CurIncrHandle;
	}
	T GetGBCodeSeq_R(T h){
		T nistat = h;
		while(m_handlemap.count(nistat) != 0)
		{
				if(nistat > (0x98967F + h))		//0x98967F == 9999999
				{
					return 0;
				}
				nistat++;
		}
		m_handlemap.insert(nistat);
		return nistat;
	}
	void AddHandle_R(T h)
	{
		// std::cout << "AddHandle_R h:" << h << " CurIncrHandle:" << CurIncrHandle << std::endl;
		m_handlemap.insert(h);
	}
	bool ReleasHandle(T h){
		m_handlemap.erase(h);
		if(CurIncrHandle > h)
		{
			CurIncrHandle = h;
		}
		std::cout << "ReleasHandle h:" << h << " CurIncrHandle:" << CurIncrHandle << std::endl;
		return true;
	}
	int Size()
	{
		return m_handlemap.size();
	}
protected:
	T m_uistart;
	T m_uiMax;
	T CurIncrHandle;
	std::set<T> m_handlemap;
};
#define DEVCODE_DOMAIN_LEN 10
enum EM_Node_Type
{
	em_node_unknown = 0,//未知类型, 暂不使用
    DVR_CODE = 111,//DVR
	NVR_CODE = 118,//NVR	
	IPC_CODE = 132,//IPC
	CAMERA_CODE = 131,//摄像机		没有使用到
    CMS_CODE = 200,
	HARDWARE_MEDIASERVER = 214,//硬媒体
	UNIT_CODE = 216,//组织机构	
    MEDIADECODESVR_CODE = 114,//解码器
	MEDIAENCODESVR_CODE = 113,//编码器
	GROUP_CODE = 215,//分组
	ALARMCONTROL_CODE = 117,//报警主机
    EXT_CODE = 217,//扩展的平台设备类型 cms中未知设备类型 
	em_node_type_max,	// 节点类型数量
};
class CDevCodeMng
{
	using DLLINT = unsigned long long;
	std::mutex m_DevCodeMutex;
	std::set<std::string> m_GBIDList;
	std::string strLocalGBID;
	std::mutex m_CodeSeqMutex;
	std::map<int, CHnaldeMrg<DLLINT>*> mDevCodeSeq;
public:
	CDevCodeMng(std::string localGbId):strLocalGBID(localGbId)
    {
        // DLLINT startCode = 0x46555F00, endCode = 0x46EDF57F;
        /// 118 网络视频录像机(NVR)编码
        mDevCodeSeq.emplace(118, new CHnaldeMrg<DLLINT>(0x46555F00 + 1, 0x46EDF57F));		//1180000000~1189999999
        // 132 网络摄像机(IPC)编码
        mDevCodeSeq.emplace(132, new CHnaldeMrg<DLLINT>(0x4EAD9A00 + 1, 0x4F46307F));		//1320000000~1329999999
        // 214 扩展的平台设备类型 cms中为硬件流媒体 
        mDevCodeSeq.emplace(214, new CHnaldeMrg<DLLINT>(0x7F8DCF00 + 1, 0x8026657F));		//2140000000~2149999999
        // 200 中心信令控制服务器编码 
        mDevCodeSeq.emplace(200, new CHnaldeMrg<DLLINT>(0x77359400 + 1, 0x77CE2A7F));		//2000000000~2009999999
        // 216 虚拟组织编码 
        mDevCodeSeq.emplace(216, new CHnaldeMrg<DLLINT>(0x80BEFC00 + 1, 0x8157927F));		//2160000000~2169999999
        // 111 DVR编码 
        mDevCodeSeq.emplace(111, new CHnaldeMrg<DLLINT>(0x42294180 + 1, 0x42C1D7FF));		//1110000000~1119999999
        // 114 解码器编码 
        mDevCodeSeq.emplace(114, new CHnaldeMrg<DLLINT>(0x43F30500 + 1, 0x448B9B7F));		//1140000000~1149999999
        // 113 编码器编码 
        mDevCodeSeq.emplace(113, new CHnaldeMrg<DLLINT>(0x435A6E80 + 1, 0x43F304FF));		//1130000000~1139999999
        // 131 摄像机编码 
        mDevCodeSeq.emplace(131, new CHnaldeMrg<DLLINT>(0x4E150380 + 1, 0x4EAD99FF));		//1310000000~1319999999
        // 215 业务分组编码 
        mDevCodeSeq.emplace(215, new CHnaldeMrg<DLLINT>(0x80266580 + 1, 0x80BEFBFF));		//2150000000~2159999999
        // 117 报警控制器编码 
        mDevCodeSeq.emplace(117, new CHnaldeMrg<DLLINT>(0x45BCC880 + 1, 0x46555EFF));		//1170000000~1179999999
        // 217 扩展的平台设备类型 cms中未知设备类型 
        mDevCodeSeq.emplace(217, new CHnaldeMrg<DLLINT>(0x81579280 + 1, 0x81F028FF));		//2170000000~2179999999
    };
	~CDevCodeMng()
    {
        for(auto &it : mDevCodeSeq)
        {
            delete it.second;
        }
    };
	void AddDevCodeID(const std::string &gbid)
    {
        if(gbid.size() != 20)
		return;
        {
            std::unique_lock<std::mutex> lck(m_DevCodeMutex);
            auto iter = m_GBIDList.find(gbid);
            if(iter == m_GBIDList.end())
            {
                m_GBIDList.emplace(gbid);
                LogOut(BLL, L_INFO, "AddDevCodeID add gbid :%s", gbid.c_str());
            }
        }
        if(strLocalGBID.size() == 20 && gbid.size() == 20 && strLocalGBID.substr(0,DEVCODE_DOMAIN_LEN) == gbid.substr(0,DEVCODE_DOMAIN_LEN))
        {
            DLLINT codeseq = String2Ull(gbid.substr(10, 10));
            std::unique_lock<std::mutex> lck(m_CodeSeqMutex);
            int devType = GetDeviceType(gbid);
            if(devType > 0)
            {
                if(mDevCodeSeq[devType])
                {
                    LogOut(BLL, L_INFO, "AddDevCodeID add gbid codeseq:%llu", codeseq);
                    mDevCodeSeq[devType]->AddHandle_R(codeseq);
                }
            }
        }
    };
	void DelDevCodeID(const std::string &gbid, int reson)
    {
        if(gbid.size() != 20)
		    return;
        {
            LogOut(BLL, L_INFO, "CDevCodeMng::DelDevCodeID del gbid :%s, reson:%d", gbid.c_str(), reson);
            std::unique_lock<std::mutex> lck(m_DevCodeMutex);
            auto iter = m_GBIDList.find(gbid);
            if(iter != m_GBIDList.end())
            {
                m_GBIDList.erase(gbid);
            }
        }
        if(strLocalGBID.size() == 20 && gbid.size() == 20 && strLocalGBID.substr(0, DEVCODE_DOMAIN_LEN) == gbid.substr(0, DEVCODE_DOMAIN_LEN))
        {
            DLLINT codeseq = String2Ull(gbid.substr(10, 10));
            std::unique_lock<std::mutex> lck(m_CodeSeqMutex);
            int devType = GetDeviceType(gbid);
            LogOut(BLL, L_INFO, "DelDevCodeID release seq :%s, debtype:%d", gbid.c_str(), devType);
            if(devType > 0)
            {
                if(mDevCodeSeq[devType])
                    mDevCodeSeq[devType]->ReleasHandle(codeseq);
            }
        }
    };
	bool IsDevCodeExist(const std::string &gbid)
    {
        if(gbid.size() != 20)
            return true;
        std::unique_lock<std::mutex> lck(m_DevCodeMutex);
        if(m_GBIDList.find(gbid) == m_GBIDList.end())
        {
            LogOut(BLL, L_INFO, "CDevCodeMng::IsDevCodeExist gbid not exist:%s", gbid.c_str());
            return false;
        }
        LogOut(BLL, L_INFO, "CDevCodeMng::IsDevCodeExist gbid exist:%s", gbid.c_str());
        return true;
    };
	std::string CreateDevCode(const int &DevType)
    {
        if(DevType <= em_node_unknown || DevType >= em_node_type_max)
		    return "";
	
        DLLINT nCodeSeq = 0;
        int gbDevType = 0;
        if(DevType == NVR_CODE)
        {
            nCodeSeq = 0x46555F00; //0x46555F00 == 1180000000
            gbDevType = 118;
        }
        else if(DevType == IPC_CODE)
        {
            nCodeSeq = 0x4EAD9A00; //0x4EAD9A00 == 1320000000
            gbDevType = 132;
        }
        else if(DevType == HARDWARE_MEDIASERVER)
        {
            nCodeSeq = 0x7F8DCF00; //0x7F8DCF00 == 2140000000
            gbDevType = 214;
        }
        else if(DevType == CMS_CODE)
        {
            nCodeSeq = 0x77359400; //0x77359400 == 2000000000
            gbDevType = 200;
        }
        else if(DevType == UNIT_CODE)
        {
            nCodeSeq = 0x80BEFC00; //0x80BEFC00 == 2160000000
            gbDevType = 216;
        }
        else if(DevType == DVR_CODE)
        {
            nCodeSeq = 0x42294180; //0x42294180 == 1110000000
            gbDevType = 111;
        }
        else if(DevType == MEDIADECODESVR_CODE)
        {
            nCodeSeq = 0x43F30500; //0x43F30500 == 1140000000
            gbDevType = 114;
        }
        else if(DevType == MEDIAENCODESVR_CODE)
        {
            nCodeSeq = 0x435A6E80; //0x435A6E80 == 1130000000
            gbDevType = 113;
        }
        else if(DevType == CAMERA_CODE)
        {
            nCodeSeq = 0x4E150380; //0x4E150380 == 1310000000
            gbDevType = 131;
        }
        else if(DevType == GROUP_CODE)
        {
            nCodeSeq = 0x80266580; //0x80266580 == 2150000000
            gbDevType = 215;
        }
        else
        {
            nCodeSeq = 0x81579280; //0x45BCC880 == 1170000000
            gbDevType = 217;
        }
        

        std::unique_lock<std::mutex> lck(m_CodeSeqMutex);
        DLLINT nCode = 0;
        if(mDevCodeSeq[gbDevType])
            nCode = mDevCodeSeq[gbDevType]->GetHandle_R();
        if(nCode > 0)
        {
            if(strLocalGBID.size() == 20)
            {
                std::string newGBID = strLocalGBID.substr(0, 10) + std::to_string(nCode);
                LogOut(BLL, L_INFO, "CDevCodeMng::CreateDevCode gbid :%s", newGBID.c_str());
                return newGBID;
            }
        }
        return "";
    };
	void SetLocalGBID(const std::string &localgbid)
    {
        if(localgbid.empty() || localgbid.size() != 20)
            return;
        strLocalGBID = localgbid;
        if(strLocalGBID.size() == 20)
        {
            DLLINT codeseq = String2Ull(strLocalGBID.substr(10, 10));
            std::unique_lock<std::mutex> lck(m_CodeSeqMutex);
            int devType = GetDeviceType(strLocalGBID);
            LogOut(BLL, L_INFO, "DelDevCodeID release seq :%s, debtype:%d", strLocalGBID.c_str(), devType);
            if(devType > 0)
            {
                if(mDevCodeSeq[devType])
                    mDevCodeSeq[devType]->ReleasHandle(codeseq);
            }
        }
    };
	int GetDeviceType(const std::string gbId)
    {
        if(gbId.size() == 20)
        {
            return String2Int(gbId.substr(10, 3));
        }
        return -1;
    }
};

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
    MyServerConfig& svrCfgi =GetSipServerConfig();
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
    CDevCodeMng mGBIdMng(localGBID.c_str());
    if(!CDbManager::Instance().initDb())
    {
        return -1;
    }
    else
    {
        std::list<std::shared_ptr<JsonNvrDevic>> devList;
        CDbManager::Instance().QueryDeviceInfoList(devList);
        for(auto &it: devList)
        {
            mGBIdMng.AddDevCodeID(it->getGBID());
            it->DevConnect();
            std::list<JsonChildDevic> channelList;
            getJsonNvrChannelList(it, channelList);
            for(auto &item : channelList)
            {
                JsonChildDevic channelInfo("");
                CDbManager::Instance().QuerySubDeviceInfo(it->deviceId, item.getChannel(), channelInfo);
                LogOut(BLL, L_INFO, "channel info nvrid:%s,nvrGBID:%s channel:%d, channelId:%s channelGBID:%s", 
                it->deviceId.c_str(), it->getGBID().c_str(), item.getChannel(), channelInfo.getDeviceId().c_str(), channelInfo.getGBID().c_str());
                if(channelInfo.getDeviceId().empty())
                {
                    std::string gbid = mGBIdMng.CreateDevCode(IPC_CODE);
                    int nRet = CDbManager::Instance().AddSubDeviceInfo(it->deviceId, gbid, item.getChannel(), item.getName());
                    if(nRet == 0)
                    {
                        mGBIdMng.AddDevCodeID(gbid);
                    }
                }
                else
                {
                    mGBIdMng.AddDevCodeID(channelInfo.getGBID());
                }
            }
        }
    }
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
