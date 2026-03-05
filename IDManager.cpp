#include "IDManager.h"
#include "SelfLog.h"
#include "TypeConversion.h"

CDevCodeMng& CDevCodeMng::Instance()
{
    static CDevCodeMng g_GBIDMng("");
    return g_GBIDMng;
}

CDevCodeMng::CDevCodeMng(std::string localGbId):strLocalGBID(localGbId)
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
CDevCodeMng::~CDevCodeMng()
{
    for(auto &it : mDevCodeSeq)
    {
        delete it.second;
    }
};
void CDevCodeMng::AddDevCodeID(const std::string &gbid)
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
                mDevCodeSeq[devType]->AddHandle_R(codeseq);
            }
            else
            {
                LogOut(BLL, L_ERROR, "AddDevCodeID add gbid codeseq error:%llu %s", codeseq, gbid.c_str());
            }
        }
        else
        {
            LogOut(BLL, L_ERROR, "AddDevCodeID devType error:%d %s", devType, gbid.c_str());
        }
    }
    else
    {
        LogOut(BLL, L_ERROR, "AddDevCodeID domain error:%s", gbid.c_str());
    }
};
void CDevCodeMng::DelDevCodeID(const std::string &gbid, int reson)
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
bool CDevCodeMng::IsDevCodeExist(const std::string &gbid)
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
std::string CDevCodeMng::CreateDevCode(const int &DevType)
{
    if(DevType <= em_node_unknown || DevType >= em_node_type_max)
        return "";

    DLLINT nCodeSeq = 0;
    int gbDevType = 0;
    if(DevType == NVR_CODE)
    {
        nCodeSeq = 0x46555F00 + 1; //0x46555F00 == 1180000000
        gbDevType = 118;
    }
    else if(DevType == IPC_CODE)
    {
        nCodeSeq = 0x4EAD9A00 + 1; //0x4EAD9A00 == 1320000000
        gbDevType = 132;
    }
    else if(DevType == HARDWARE_MEDIASERVER)
    {
        nCodeSeq = 0x7F8DCF00 + 1; //0x7F8DCF00 == 2140000000
        gbDevType = 214;
    }
    else if(DevType == CMS_CODE)
    {
        nCodeSeq = 0x77359400 + 1; //0x77359400 == 2000000000
        gbDevType = 200;
    }
    else if(DevType == UNIT_CODE)
    {
        nCodeSeq = 0x80BEFC00 + 1; //0x80BEFC00 == 2160000000
        gbDevType = 216;
    }
    else if(DevType == DVR_CODE)
    {
        nCodeSeq = 0x42294180 + 1; //0x42294180 == 1110000000
        gbDevType = 111;
    }
    else if(DevType == MEDIADECODESVR_CODE)
    {
        nCodeSeq = 0x43F30500 + 1; //0x43F30500 == 1140000000
        gbDevType = 114;
    }
    else if(DevType == MEDIAENCODESVR_CODE)
    {
        nCodeSeq = 0x435A6E80 + 1; //0x435A6E80 == 1130000000
        gbDevType = 113;
    }
    else if(DevType == CAMERA_CODE)
    {
        nCodeSeq = 0x4E150380 + 1; //0x4E150380 == 1310000000
        gbDevType = 131;
    }
    else if(DevType == GROUP_CODE)
    {
        nCodeSeq = 0x80266580 + 1; //0x80266580 == 2150000000
        gbDevType = 215;
    }
    else
    {
        nCodeSeq = 0x81579280 + 1; //0x45BCC880 == 1170000000
        gbDevType = 217;
    }
    

    DLLINT nCode = 0;
    {
        std::unique_lock<std::mutex> lck(m_CodeSeqMutex);
        if(mDevCodeSeq[gbDevType])
        {
            nCode = mDevCodeSeq[gbDevType]->GetGBCodeSeq_R(nCodeSeq);
            // LogOut(BLL, L_INFO, "CDevCodeMng::CreateDevCode start seq :%llu nCode:%llu", nCodeSeq, nCode);
        }
    }
    if(nCode > 0)
    {
        if(strLocalGBID.size() == 20)
        {
            std::string newGBID = strLocalGBID.substr(0, 10) + std::to_string(nCode);
            LogOut(BLL, L_INFO, "CDevCodeMng::CreateDevCode gbid :%s", newGBID.c_str());
            std::unique_lock<std::mutex> lck(m_DevCodeMutex);
            auto iter = m_GBIDList.find(newGBID);
            if(iter == m_GBIDList.end())
            {
                m_GBIDList.emplace(newGBID);
                LogOut(BLL, L_INFO, "AddDevCodeID add gbid :%s", newGBID.c_str());
            }
            return newGBID;
        }
    }
    return "";
};
void CDevCodeMng::SetLocalGBID(const std::string &localgbid)
{
    if(localgbid.empty() || localgbid.size() != 20 || strLocalGBID == localgbid)
        return;
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
    strLocalGBID = localgbid;
    AddDevCodeID(localgbid);
};
int CDevCodeMng::GetDeviceType(const std::string gbId)
{
    if(gbId.size() == 20)
    {
        return String2Int(gbId.substr(10, 3));
    }
    return -1;
}