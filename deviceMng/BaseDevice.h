#pragma once
#include <memory>
#include <functional>
#include <iostream>

#include "../uainfo.h"

using ULHandle = unsigned long;
class BaseDevice
{
public:
    using Ptr = std::shared_ptr<BaseDevice>;
    /*using previewCb = std::function<void(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)>;
    using PbCbData = std::function<void(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)>;
    using PbCbEnd = std::function<void(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)>;
    using DownloadData = std::function<void(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)>;
    using DownloadEnd = std::function<void(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)>;*/
    typedef enum
    {
        JSON_NVR,
        SRC_DEVTYPE_GB28181,
    }DEVTYPE_X;
    BaseDevice(const char*Id, DEVTYPE_X type) :devType(type), deviceId(Id){};
    virtual ~BaseDevice() {};
    DEVTYPE_X devType;
    std::string deviceId;
    ULHandle mLoginId;

    virtual void setStatus(int statu);
    virtual const int getStatus();

    virtual void setIp(const std::string ip);
    virtual const std::string& getIp() const;

    virtual void setPort(const short port);
    virtual const short getPort() const;

    virtual void setUser(const std::string user);
    virtual const std::string& getUser() const;

    virtual void setPswd(const std::string pswd);
    virtual const std::string& getPswd() const;

    virtual void setName(const std::string name);
    virtual const std::string& getName() const;

    virtual void setGBID(const std::string gbid);
    virtual const std::string& getGBID() const;

    virtual void setLastUpdate(const std::string updateTime);
    virtual const std::string& getLastUpdate() const;

    virtual void setDevType(const std::string type);
    virtual const std::string& getDevType() const;

    virtual void setProtocol(const std::string protocol);
    virtual const std::string& getProtocol() const;

    virtual void updateDeviceInfo(const BaseDevice* info);

protected:
    std::string mIP;
    short mPort;
    std::string mName;
    std::string mUser;
    std::string mPswd;
    std::string mGBID;
    std::string mUpdateTime;
    std::string mProtocol;
    std::string mDevType;
    int status;
};


class BaseChildDevice
{
public:
    BaseChildDevice(const char* id) :deviceId(id), mParentDev(NULL){};
    virtual ~BaseChildDevice() {};
    const std::string& getDeviceId() { return deviceId; };

    void setId(const std::string id) { deviceId = id; };
    void setParentDev(BaseDevice::Ptr dev) { mParentDev = dev; };
    const BaseDevice::Ptr getParentDev() { return mParentDev; };

    virtual void setName(std::string nm);
    virtual const std::string getName();

    virtual void setStatus(int statu);
    virtual const int getStatus();

    virtual void setParentId(std::string nm);
    virtual const std::string getParentId();

    virtual void setChildIp(std::string Ip);
    virtual const std::string getChildIp();

    virtual void setGBID(std::string gbid);
    virtual const std::string getGBID();

    virtual void setChannel(int chl);
    virtual const int getChannel();
    virtual void setLastPtzCmd(int chl);
    virtual const int getLastPtzCmd();

    virtual void setLastUpdate(const std::string updateTime);
    virtual const std::string& getLastUpdate() const;

    virtual CatalogItem GetCatalogItem(std::string myId);
private:
    std::string deviceId;
    BaseDevice::Ptr mParentDev;

    std::string name;
    int status;
    std::string parentId;
    std::string ChildIp;
    std::string mGBID;
    std::string mUpdateTime;
    int channel;
    int ptzCmd;
};