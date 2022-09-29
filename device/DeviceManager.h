#ifndef DEVICE_MANAGER_H_
#define DEVICE_MANAGER_H_
#include "DeviceInfo.h"
#include "DeviceDbManager.h"
#include "../tools/iThreadPool.h"
#include "DeviceInterface.h"
#include <list>
#include <map>
#include <mutex>


class IDeviceMngrSvr : public IDeviceMngrInterface
{
    mutex mMapMutex;
    map<string, std::shared_ptr<Device>> mDeviceMap;
    GBDeviceMapper mGBDeviceMapper;
    //map<string, IDeviceMngr*> mIDevMngrMap;
    //std::shared_ptr<GBDeviceOnlineTask> task;
private:
    std::shared_ptr<Device> getDevice(const string& deviceId);
public:
    IDeviceMngrSvr();
    /*virtual IDeviceMngr() = 0;
	~IDeviceMngr();*/
    void initialize();
    GBDeviceMapper& GetGBDeviceMapper();
    /**
        * 设备上线
        * @param device 设备信息
        */
    virtual void online(Device *device);

    /**
        * 设备下线
        * @param deviceId 设备编号
        */
    virtual void offline(string deviceId);

    ///**
    //    * 移除移动位置订阅
    //    * @param deviceId 设备ID
    //    * @return 同步状态
    //    */
    //SyncStatus getChannelSyncStatus(string deviceId);

    ///**
    //    * 查看是否仍在同步
    //    * @param deviceId 设备ID
    //    * @return 布尔
    //    */
    virtual bool isSyncRunning(string deviceId);

    ///**
    //    * 通道同步
    //    * @param device 设备信息
    //    */
    virtual void sync(std::shared_ptr<Device> device);

    ///**
    //    * 查询设备信息
    //    * @param deviceId 设备编号
    //    * @return 设备信息
    //    */
    virtual std::shared_ptr<Device> queryDevice(string deviceId);

    ///**
    //    * 获取所有在线设备
    //    * @return 设备列表
    //    */
    virtual list<std::shared_ptr<Device>> getAllOnlineDevice();
    virtual list<std::shared_ptr<Device>> getAllOffLineDevice();

    ///**
    //    * 判断是否注册已经失效
    //    * @param device 设备信息
    //    * @return 布尔
    //    */
    //bool expire(Device device);

    ///**
    //    * 检查设备状态
    //    * @param device 设备信息
    //    */
    //void checkDeviceStatus(Device device);

    ///**
    //    * 根据IP和端口获取设备信息
    //    * @param host IP
    //    * @param port 端口
    //    * @return 设备信息
    //    */
    //Device getDeviceByHostAndPort(string host, int port);

    ///**
    //    * 更新设备
    //    * @param device 设备信息
    //    */
    void updateDevice(Device* device);
    void clearCatchByDeviceId(string devId);
    void updateDeviceInfoCatch(string devId, Device* device);
    void GetAllDeviceList(list<std::shared_ptr<Device>>& devlist);
    std::shared_ptr<Device> GetDeviceChannelList(string devId, list<std::shared_ptr<IDeviceChannel>>& channellist);
    int  GetAllChannelCount();
    //int  GetAllChannelCount();
};

//class GBDeviceMngr : public IDeviceMngr
//{
//    GBDeviceMapper mIDevMapper;
//    friend class SqlitDb;
//public:
//    virtual void online(Device device);
//    virtual void offline(string deviceId);
//    /**
//        * 添加目录订阅
//        * @param device 设备信息
//        * @return 布尔
//        */
//    bool addCatalogSubscribe(Device device);
//
//    ///**
//    //    * 移除目录订阅
//    //    * @param device 设备信息
//    //    * @return 布尔
//    //    */
//    //bool removeCatalogSubscribe(Device device);
//
//    ///**
//    //    * 添加移动位置订阅
//    //    * @param device 设备信息
//    //    * @return 布尔
//    //    */
//    //bool addMobilePositionSubscribe(Device device);
//
//    ///**
//    //    * 移除移动位置订阅
//    //    * @param device 设备信息
//    //    * @return 布尔
//    //    */
//    //bool removeMobilePositionSubscribe(Device device);
//};
//class OnvifDeviceMngr : public IDeviceMngr
//{
//public:
//    virtual void online(Device device);
//    virtual void offline(string deviceId);
//    /**
//        * 添加目录订阅
//        * @param device 设备信息
//        * @return 布尔
//        */
//    bool addCatalogSubscribe(Device device);
//
//    /**
//        * 移除目录订阅
//        * @param device 设备信息
//        * @return 布尔
//        */
//    bool removeCatalogSubscribe(Device device);
//
//    /**
//        * 添加移动位置订阅
//        * @param device 设备信息
//        * @return 布尔
//        */
//    bool addMobilePositionSubscribe(Device device);
//
//    /**
//        * 移除移动位置订阅
//        * @param device 设备信息
//        * @return 布尔
//        */
//    bool removeMobilePositionSubscribe(Device device);
//};

IDeviceMngrSvr& GetIDeviceMngr();
#endif