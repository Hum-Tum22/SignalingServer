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
        * �豸����
        * @param device �豸��Ϣ
        */
    virtual void online(Device *device);

    /**
        * �豸����
        * @param deviceId �豸���
        */
    virtual void offline(string deviceId);

    ///**
    //    * �Ƴ��ƶ�λ�ö���
    //    * @param deviceId �豸ID
    //    * @return ͬ��״̬
    //    */
    //SyncStatus getChannelSyncStatus(string deviceId);

    ///**
    //    * �鿴�Ƿ�����ͬ��
    //    * @param deviceId �豸ID
    //    * @return ����
    //    */
    virtual bool isSyncRunning(string deviceId);

    ///**
    //    * ͨ��ͬ��
    //    * @param device �豸��Ϣ
    //    */
    virtual void sync(std::shared_ptr<Device> device);

    ///**
    //    * ��ѯ�豸��Ϣ
    //    * @param deviceId �豸���
    //    * @return �豸��Ϣ
    //    */
    virtual std::shared_ptr<Device> queryDevice(string deviceId);

    ///**
    //    * ��ȡ���������豸
    //    * @return �豸�б�
    //    */
    virtual list<std::shared_ptr<Device>> getAllOnlineDevice();
    virtual list<std::shared_ptr<Device>> getAllOffLineDevice();

    ///**
    //    * �ж��Ƿ�ע���Ѿ�ʧЧ
    //    * @param device �豸��Ϣ
    //    * @return ����
    //    */
    //bool expire(Device device);

    ///**
    //    * ����豸״̬
    //    * @param device �豸��Ϣ
    //    */
    //void checkDeviceStatus(Device device);

    ///**
    //    * ����IP�Ͷ˿ڻ�ȡ�豸��Ϣ
    //    * @param host IP
    //    * @param port �˿�
    //    * @return �豸��Ϣ
    //    */
    //Device getDeviceByHostAndPort(string host, int port);

    ///**
    //    * �����豸
    //    * @param device �豸��Ϣ
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
//        * ���Ŀ¼����
//        * @param device �豸��Ϣ
//        * @return ����
//        */
//    bool addCatalogSubscribe(Device device);
//
//    ///**
//    //    * �Ƴ�Ŀ¼����
//    //    * @param device �豸��Ϣ
//    //    * @return ����
//    //    */
//    //bool removeCatalogSubscribe(Device device);
//
//    ///**
//    //    * ����ƶ�λ�ö���
//    //    * @param device �豸��Ϣ
//    //    * @return ����
//    //    */
//    //bool addMobilePositionSubscribe(Device device);
//
//    ///**
//    //    * �Ƴ��ƶ�λ�ö���
//    //    * @param device �豸��Ϣ
//    //    * @return ����
//    //    */
//    //bool removeMobilePositionSubscribe(Device device);
//};
//class OnvifDeviceMngr : public IDeviceMngr
//{
//public:
//    virtual void online(Device device);
//    virtual void offline(string deviceId);
//    /**
//        * ���Ŀ¼����
//        * @param device �豸��Ϣ
//        * @return ����
//        */
//    bool addCatalogSubscribe(Device device);
//
//    /**
//        * �Ƴ�Ŀ¼����
//        * @param device �豸��Ϣ
//        * @return ����
//        */
//    bool removeCatalogSubscribe(Device device);
//
//    /**
//        * ����ƶ�λ�ö���
//        * @param device �豸��Ϣ
//        * @return ����
//        */
//    bool addMobilePositionSubscribe(Device device);
//
//    /**
//        * �Ƴ��ƶ�λ�ö���
//        * @param device �豸��Ϣ
//        * @return ����
//        */
//    bool removeMobilePositionSubscribe(Device device);
//};

IDeviceMngrSvr& GetIDeviceMngr();
#endif