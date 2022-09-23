#pragma once

class IDeviceMngrInterface
{
public:
    virtual void online(Device* device) = 0;
    virtual void offline(string deviceId) = 0;
    virtual bool isSyncRunning(string deviceId) = 0;
    virtual void sync(std::shared_ptr<Device> device) = 0;
    virtual std::shared_ptr<Device> queryDevice(string deviceId) = 0;

    ///**
    //    * �Ƴ��ƶ�λ�ö���
    //    * @param deviceId �豸ID
    //    * @return ͬ��״̬
    //    */
    //SyncStatus getChannelSyncStatus(string deviceId);

    ///**
    //    * ��ȡ���������豸
    //    * @return �豸�б�
    //    */
    virtual list<std::shared_ptr<Device>> getAllOnlineDevice() = 0;
    virtual list<std::shared_ptr<Device>> getAllOffLineDevice() = 0;

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
    /*void updateDevice(Device* device);
    void clearCatchByDeviceId(string devId);
    void updateDeviceInfoCatch(string devId, Device* device);
    void GetAllDeviceList(list<Device*>& devlist);
    int  GetAllChannelCount();*/
};