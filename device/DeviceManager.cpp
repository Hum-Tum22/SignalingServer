
#include "DeviceManager.h"
#include "../tools/genuuid.h"
#include "../tools/m_Time.h"
#include "CatalogData.h"
#include "../GBTask.h"

std::shared_ptr<Device> IDeviceMngrSvr::getDevice(const string& deviceId)
{
    auto it = mDeviceMap.find(deviceId);
    if (it != mDeviceMap.end())
        return it->second;
    return NULL;
}
IDeviceMngrSvr::IDeviceMngrSvr()
{
    mGBDeviceMapper.IniTable();
    //mGBDeviceChannelMapper.
    initialize();
}
//IDeviceMngrSvr::~IDeviceMngrSvr()
//{
//}
void IDeviceMngrSvr::initialize()
{
    list<SipServerDeviceInfo> GbDeviceList = mGBDeviceMapper.getDevices();
    for (auto& it : GbDeviceList)
    {
        mDeviceMap[it.getDeviceId()] = std::make_shared<SipServerDeviceInfo>(it);
    }
}
GBDeviceMapper& IDeviceMngrSvr::GetGBDeviceMapper()
{
    return mGBDeviceMapper;
}
void IDeviceMngrSvr::online(Device *device)
{
    if (device == NULL)
        return;
    //logger.info("[设备上线] deviceId：{}->{}:{}", device.getDeviceId(), device.getIp(), device.getPort());
    //Device deviceInRedis = redisCatchStorage.getDevice(device.getDeviceId());
    std::shared_ptr<GBDeviceOnlineTask> task;
    OnvifDeviceInfo* pOnvifDev = dynamic_cast<OnvifDeviceInfo*>(device);
    if (device->getDevAccessProtocal() == Device::DEV_ACCESS_GB28181)
    {
        SipServerDeviceInfo* deviceInfo = NULL;
        std::shared_ptr<Device> shdDev = getDevice(device->getDeviceId());
        if (shdDev)
        {
            deviceInfo = dynamic_cast<SipServerDeviceInfo*>(shdDev.get());
            CDateTime nowtm;
            deviceInfo->setRegisterTime(nowtm.tmFormat());
            deviceInfo->setUpdateTime(nowtm.tmFormat());
            mGBDeviceMapper.update(device);
        }
        else
        {
            deviceInfo = dynamic_cast<SipServerDeviceInfo*>(device);
            if (deviceInfo)
            {
                std::shared_ptr<SipServerDeviceInfo> devinfo = std::make_shared<SipServerDeviceInfo>(*deviceInfo);
                devinfo->setUuId(imuuid::uuidgen());
                CDateTime nowtm;
                devinfo->setRegisterTime(nowtm.tmFormat());
                devinfo->setUpdateTime(nowtm.tmFormat());
                devinfo->setCreateTime(nowtm.tmFormat());

                mDeviceMap.insert(std::pair<string, std::shared_ptr<Device>>(devinfo->getDeviceId(), devinfo));
                mGBDeviceMapper.add(devinfo.get());
            }
        }
        
        //std::cout << "ownAny::Any_cast<std::string>(taskResult.get()) :" << ownAny::Any_cast<std::string>(taskResult.get()) << std::endl;
        if (deviceInfo)
        {
            //ownThreadPool::myThreadPool& thdpool = ownThreadPool::GetThreadPool();
            Uri target;
            target.user() = deviceInfo->getDeviceId().c_str();
            target.host() = deviceInfo->getIp().c_str();
            target.port() = deviceInfo->getPort();
            task = std::make_shared<GBDeviceOnlineTask>(deviceInfo->getDeviceId(), target);
            //GBDeviceOnlineTask* ttask = new GBDeviceOnlineTask(pGBDev->getDeviceId(), target);
            //bool taskResult = thdpool.submitTask(task);
            //SipServerDeviceInfo* pGBDev = dynamic_cast<SipServerDeviceInfo*>(device);
        }
    }
    //std::cout << "task :" << task << std::endl;
}
void IDeviceMngrSvr::offline(string deviceId)
{
    SipServerDeviceInfo device = mGBDeviceMapper.getDeviceByDeviceId(deviceId);
    if (!device.getUuId().empty())
    {
        //String registerExpireTaskKey = registerExpireTaskKeyPrefix + deviceId;
        //dynamicTask.stop(registerExpireTaskKey);
            device.setOnline(0);
            //redisCatchStorage.updateDevice(device);
            mGBDeviceMapper.update(&device);
            // 离线释放所有ssrc
            /*List<SsrcTransaction> ssrcTransactions = streamSession.getSsrcTransactionForAll(deviceId, null, null, null);
            if (ssrcTransactions != null && ssrcTransactions.size() > 0) {
                for (SsrcTransaction ssrcTransaction : ssrcTransactions) {
                    mediaServerService.releaseSsrc(ssrcTransaction.getMediaServerId(), ssrcTransaction.getSsrc());
                    mediaServerService.closeRTPServer(deviceId, ssrcTransaction.getChannelId(), ssrcTransaction.getStream());
                    streamSession.remove(deviceId, ssrcTransaction.getChannelId(), ssrcTransaction.getStream());
                }
            }*/
            // 移除订阅
            //removeCatalogSubscribe(device);
            //removeMobilePositionSubscribe(device);
    }
    
}
bool IDeviceMngrSvr::isSyncRunning(string deviceId)
{
    auto catalogData = mDeviceMap.find(deviceId);
    if (catalogData != mDeviceMap.end())
    {
        std::shared_ptr<SipServerDeviceInfo> pGBDev = std::static_pointer_cast<SipServerDeviceInfo>(catalogData->second);
        if (pGBDev)
        {
            return !(pGBDev->getCatalogStatus() == SipServerDeviceInfo::end);
        }
    }
    return true;
}
void IDeviceMngrSvr::sync(std::shared_ptr<Device> device)
{
    if (device)
    {
        if (device->getDevAccessProtocal() == Device::DEV_ACCESS_GB28181)
        {
            std::shared_ptr<SipServerDeviceInfo> pGBDev = std::static_pointer_cast<SipServerDeviceInfo>(device);
            if (pGBDev)
            {
                if (isSyncRunning(pGBDev->getDeviceId()))
                    return;
                //pGBDev->setStatus(SipServerDeviceInfo::ready);
                //同步数据库
            }
        }
    }
}
std::shared_ptr<Device> IDeviceMngrSvr::queryDevice(string deviceId)
{
    auto iter = mDeviceMap.find(deviceId);
    if (iter != mDeviceMap.end())
        return iter->second;
    return NULL;
}
list<std::shared_ptr<Device>> IDeviceMngrSvr::getAllOnlineDevice()
{
    list<std::shared_ptr<Device>> deviceList;
    return deviceList;
}
list<std::shared_ptr<Device>> IDeviceMngrSvr::getAllOffLineDevice()
{
    list<std::shared_ptr<Device>> deviceList;
    return deviceList;
}
void IDeviceMngrSvr::updateDevice(Device* device)
{
    SipServerDeviceInfo* pGBDev = dynamic_cast<SipServerDeviceInfo*>(device);
    //OnvifDeviceInfo* pOnvifDev = dynamic_cast<OnvifDeviceInfo*>(device);
    if (pGBDev)
    {   
        mGBDeviceMapper.update(pGBDev);
    }

}
void IDeviceMngrSvr::clearCatchByDeviceId(string devId)
{
    mDeviceMap.erase(devId);
}
void IDeviceMngrSvr::updateDeviceInfoCatch(string devId, Device* device)
{
    auto iter = mDeviceMap.find(devId);
    if (iter != mDeviceMap.end())
    {
        if (iter->second->getUuId() == device->getUuId())
        {

        }
    }
    //mDeviceMap[pGBDev->getDeviceId()] = pGBDev;
}
void IDeviceMngrSvr::GetAllDeviceList(list<std::shared_ptr<Device>>& devlist)
{
    for (auto& it : mDeviceMap)
    {
        devlist.push_back(it.second);
    }
}
std::shared_ptr<Device> IDeviceMngrSvr::GetDeviceChannelList(string devId, list<std::shared_ptr<IDeviceChannel>>& channellist)
{
    auto iter = mDeviceMap.find(devId);
    if (iter != mDeviceMap.end())
    {
        if (iter->second->getDevAccessProtocal() == Device::DEV_ACCESS_GB28181)
        {
            std::shared_ptr<SipServerDeviceInfo> devInfo = std::static_pointer_cast<SipServerDeviceInfo>(iter->second);
            if (devInfo)
            {
                channellist = devInfo->getChannelList();
            }
        }
        return iter->second;
    }
    return NULL;
}
int IDeviceMngrSvr::GetAllChannelCount()
{
    return mGBDeviceMapper.GetGBDeviceChannelMapper().getAllChannelCount();
}
IDeviceMngrSvr *g_devmgr = new IDeviceMngrSvr();
IDeviceMngrSvr& GetIDeviceMngr()
{
    return *g_devmgr;
}

//void GBDeviceMngr::online(Device device)
//{
//	//mIDevMapper.on
//}
//void GBDeviceMngr::offline(string deviceId)
//{
//}
//bool GBDeviceMngr::addCatalogSubscribe(Device device)
//{
//	GBDeviceMngr* pdb = dynamic_cast<GBDeviceMngr*>(g_devmgr);
//	return false;
//}
//
//
//void OnvifDeviceMngr::online(Device device)
//{
//}
//void OnvifDeviceMngr::offline(string deviceId)
//{
//}
