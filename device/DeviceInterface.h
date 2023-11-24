#pragma once

class IDeviceMngrInterface
{
public:
    virtual void online(Device* device) = 0;
    virtual void offline(std::string deviceId) = 0;
    virtual bool isSyncRunning(std::string deviceId) = 0;
    virtual void sync(std::shared_ptr<Device> device) = 0;
    virtual std::shared_ptr<Device> queryDevice(std::string deviceId) = 0;

    ///**
    //    * 移除移动位置订阅
    //    * @param deviceId 设备ID
    //    * @return 同步状态
    //    */
    //SyncStatus getChannelSyncStatus(string deviceId);

    ///**
    //    * 获取所有在线设备
    //    * @return 设备列表
    //    */
    virtual std::list<std::shared_ptr<Device>> getAllOnlineDevice() = 0;
    virtual std::list<std::shared_ptr<Device>> getAllOffLineDevice() = 0;

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
    /*void updateDevice(Device* device);
    void clearCatchByDeviceId(string devId);
    void updateDeviceInfoCatch(string devId, Device* device);
    void GetAllDeviceList(list<Device*>& devlist);
    int  GetAllChannelCount();*/
};