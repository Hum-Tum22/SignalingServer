#pragma once

#include <functional>

using ULHandle = unsigned long;
class BaseDevice
{
public:
    using Ptr = std::shared_ptr<BaseDevice>;
    using previewCb = std::function<void(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)>;
    using PbCbData = std::function<void(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)>;
    using PbCbEnd = std::function<void(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)>;
    using DownloadData = std::function<void(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)>;
    using DownloadEnd = std::function<void(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)>;
    typedef enum
    {
        JSON_NVR,
        SRC_DEVTYPE_GB28181,
    }DEVTYPE_X;
    BaseDevice(const char*Id, DEVTYPE_X type) :devType(type), deviceId(Id){};
    virtual ~BaseDevice() {};
    DEVTYPE_X devType;
    const std::string deviceId;

    virtual bool DevConnect() = 0;
    virtual bool DevDisConnect() = 0;

    virtual ULHandle Dev_Preview(int channelId, int streamType, void* fun, void* pUser, int& err) = 0;
    virtual void Dev_StopPreview(ULHandle, int& err) = 0;

    virtual ULHandle Dev_PlayBack(int channelId, long start, long end, PbCbData VideoTranCallBack, PbCbEnd fun, void* pUser, int& err) = 0;
    virtual void Dev_StopPlayBack(ULHandle, int& err) = 0;

    virtual ULHandle Dev_Download(int channelId, long start, long end, DownloadData VideoTranCallBack, DownloadEnd fun, void* pUser, int& err) = 0;
    virtual void Dev_StopDownload(ULHandle, int& err) = 0;

    virtual void Dev_PTZCtrl(int channelId, uint32_t PTZCommand, uint32_t Argument, int& err) = 0;

    virtual void Dev_ListIPC(char*Buffer, uint32_t BufSize, int& err) = 0;
};


class BaseChildDevice
{
public:
    BaseChildDevice(const char* id) :deviceId(id), mParentDev(NULL){};
    virtual ~BaseChildDevice() {};
    const std::string& getDeviceId() { return deviceId; };

    void setParentDev(BaseDevice::Ptr dev) { mParentDev = dev; };
    const BaseDevice::Ptr getParentDev() { return mParentDev; };

private:
    const std::string deviceId;
    BaseDevice::Ptr mParentDev;
};