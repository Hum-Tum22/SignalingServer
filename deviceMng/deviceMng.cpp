#include "deviceMng.h"
#include "JsonDevice.h"
#include "SelfLog.h"
#include "IDManager.h"

DeviceMng& DeviceMng::Instance()
{
    static DeviceMng* g_DeviceMng = new DeviceMng();
    return *g_DeviceMng;
}
DeviceMng::~DeviceMng()
{
    {
        GMUTEX lock(childMtx);
        mChildMap.clear();
    }
    {
        GMUTEX lock(devMtx);
        /*for (auto &iter : mDeviceMap)
        {
            delete iter.second;
            iter.second = NULL;
        }*/
        mDeviceMap.clear();
    }

}
void DeviceMng::setSelfId(const std::string& myId)
{
    selfId = myId;
}
const std::string DeviceMng::getSelfId()
{
    return selfId;
}
void DeviceMng::addDevice(BaseDevice::Ptr dev)
{
    LogOut(BLL, L_DEBUG, "addDevice id:%s", dev->deviceId.c_str());
    GMUTEX lock(devMtx);
    mDeviceMap[dev->deviceId] = dev;
}
void DeviceMng::removeDevice(std::string Id)
{
    GMUTEX lock(devMtx);
    LogOut(BLL, L_DEBUG, "addDevice id:%s", Id.c_str());
    CDevCodeMng::Instance().DelDevCodeID(mDeviceMap[Id]->getGBID(), 1);
    mDeviceMap.erase(Id);
}
BaseDevice::Ptr DeviceMng::findDevice(const std::string Id)
{
    GMUTEX lock(devMtx);
    auto it = mDeviceMap.find(Id);
    if (it != mDeviceMap.end())
    {
        return it->second;
    }
    return NULL;
}

void DeviceMng::addChildDevice(std::shared_ptr<BaseChildDevice> child)
{
    GMUTEX lock(childMtx);
    mChildMap[child->getDeviceId()] = child;
}
void DeviceMng::removeChildDevice(std::string Id)
{
    GMUTEX lock(childMtx);
    CDevCodeMng::Instance().DelDevCodeID(mChildMap[Id]->getGBID(), 2);
    mChildMap.erase(Id);
}
std::shared_ptr<BaseChildDevice> DeviceMng::findChildDevice(const std::string Id)
{
    GMUTEX lock(childMtx);
    auto it = mChildMap.find(Id);
    if (it != mChildMap.end())
    {
        return it->second;
    }
    return nullptr;
}
std::shared_ptr<BaseChildDevice> DeviceMng::findChildDeviceByGBID(const std::string gbId)
{
    GMUTEX lock(childMtx);
    for(auto& it : mChildMap)
    {
        if (it.second && it.second->getGBID() == gbId)
        {
            return it.second;
        }
    }
    return nullptr;
}
void DeviceMng::getChildDevice(const std::string& Id, std::vector<std::shared_ptr<BaseChildDevice>>& vcList)
{
    if (Id.empty())
    {
        GMUTEX lock(childMtx);
        for (auto& it : mChildMap)
        {
            vcList.push_back(it.second);
        }
    }
    else
    {
        if (selfId == Id)
        {
            getChildDevice("", vcList);
        }
        else
        {
            std::map<std::string, std::shared_ptr<BaseChildDevice>> ChildMap;
            {
                GMUTEX lock(childMtx);
                ChildMap = mChildMap;
            }
            for (auto& it : ChildMap)
            {
                if (it.second && it.second->getParentDev()->deviceId == Id)
                {
                    vcList.push_back(it.second);
                    getChildDevice(it.second->getDeviceId(), vcList);
                }
            }
        }
    }
}
std::shared_ptr<BaseChildDevice> DeviceMng::findChildDeviceByCCTVDeviceId(const std::string Id)
{
    if (!Id.empty())
    {
        LogOut(BLL, L_DEBUG, "findChildDeviceByCCTVDeviceId find id:%s", Id.c_str());
        GMUTEX lock(childMtx);
        for (auto& it : mChildMap)
        {
            if (it.second && it.second->getGBID() == Id)
            {
                return it.second;
            }
        }
    }
    return nullptr;
}
void DeviceMng::addVirtualOrganization(VirtualOrganization vo)
{
    GMUTEX lock(childMtx);
    mVoMap[vo.DeviceID] = vo;
}
void DeviceMng::removeVirtualOrganization(std::string Id)
{
    GMUTEX lock(childMtx);
    mVoMap.erase(Id);
}
VirtualOrganization* DeviceMng::findVirtualOrganization(std::string Id)
{
    GMUTEX lock(childMtx);
    auto it = mVoMap.find(Id);
    if (it != mVoMap.end())
    {
        return &it->second;
    }
    return NULL;
}
void DeviceMng::getVirtualOrganization(const std::string& Id, std::vector<VirtualOrganization>& vcList)
{
    if (Id.empty())
    {
        GMUTEX lock(childMtx);
        for (auto& it : mVoMap)
        {
            vcList.push_back(it.second);
        }
    }
    else
    {
        if (selfId == Id)
        {
            getVirtualOrganization("", vcList);
        }
        else
        {
            GMUTEX lock(childMtx);
            for (auto& it : mVoMap)
            {
                if (it.second.ParentID == Id)
                {
                    vcList.push_back(it.second);
                    getVirtualOrganization(it.second.DeviceID, vcList);
                }
            }
        }
    }
}