#include "BaseDevice.h"
#include "../tools/CodeConversion.h"



void BaseDevice::setStatus(int statu)
{
    status = statu;
}
const int BaseDevice::getStatus()
{
    return status;
}
void BaseDevice::setIp(const std::string ip)
{
    mIP = ip;
}
const std::string& BaseDevice::getIp() const
{
    return mIP;
}

void BaseDevice::setPort(const short port)
{
    mPort = port;
}
const short BaseDevice::getPort() const
{
    return mPort;
}

void BaseDevice::setUser(const std::string user)
{
    mUser = user;
}
const std::string& BaseDevice::getUser() const
{
    return mUser;
}

void BaseDevice::setPswd(const std::string pswd)
{
    mPswd = pswd;
}
const std::string& BaseDevice::getPswd() const
{
    return mPswd;
}

void BaseDevice::setName(const std::string name)
{
    mName = name;
}
const std::string& BaseDevice::getName() const
{
    return mName;
}

void BaseDevice::setGBID(const std::string gbid)
{
    mGBID = gbid;
}
const std::string& BaseDevice::getGBID() const
{
    return mGBID;
}
void BaseDevice::setLastUpdate(const std::string updateTime)
{
    mUpdateTime = updateTime;
}
const std::string& BaseDevice::getLastUpdate() const
{
    return mUpdateTime;
}

void BaseDevice::setDevType(const std::string type)
{
    mDevType = type;
}
const std::string& BaseDevice::getDevType() const
{
    return mDevType;
}

void BaseDevice::setProtocol(const std::string protocol)
{
    mProtocol = protocol;
}
const std::string& BaseDevice::getProtocol() const
{
    return mProtocol;
}
void BaseDevice::updateDeviceInfo(const BaseDevice* devInfo)
{
    if(!devInfo)
    {
        return;
    }
    if(!devInfo->getDevType().empty() && mDevType != devInfo->getDevType())
    {
        mDevType = devInfo->getDevType();
    }
    if(!devInfo->getProtocol().empty() && mProtocol != devInfo->getProtocol())
    {
        mProtocol = devInfo->getProtocol();
    }
    if(!devInfo->getIp().empty() && mIP != devInfo->getIp())
    {
        mIP = devInfo->getIp();
    }
    if(devInfo->getPort() > 0 && mPort != devInfo->getPort())
    {
        mPort = devInfo->getPort();
    }
    if(!devInfo->getUser().empty() && mUser != devInfo->getUser())
    {
        mUser = devInfo->getUser();
    }
    if(!devInfo->getPswd().empty() && mPswd != devInfo->getPswd())
    {
        mPswd = devInfo->getPswd();
    }
    if(!devInfo->getName().empty() && mName != devInfo->getName())
    {
        mName = devInfo->getName();
    }
}




void BaseChildDevice::setName(std::string nm)
{
    name = nm;
}
const std::string BaseChildDevice::getName()
{
    return name;
}
void BaseChildDevice::setStatus(int statu)
{
    status = statu;
}
const int BaseChildDevice::getStatus()
{
    return status;
}
void BaseChildDevice::setParentId(std::string pId)
{
    parentId = pId;
}
const std::string BaseChildDevice::getParentId()
{
    return parentId;
}
void BaseChildDevice::setChildIp(std::string Ip)
{
    ChildIp = Ip;
}
const std::string BaseChildDevice::getChildIp()
{
    return ChildIp;
}
void BaseChildDevice::setGBID(std::string gbid)
{
    mGBID = gbid;
}
const std::string BaseChildDevice::getGBID()
{
    return mGBID;
}
void BaseChildDevice::setChannel(int chl)
{
    channel = chl;
}
const int BaseChildDevice::getChannel()
{
    return channel;
}
void BaseChildDevice::setLastPtzCmd(int cmd)
{
    ptzCmd = cmd;
}
const int BaseChildDevice::getLastPtzCmd()
{
    return ptzCmd;
}

void BaseChildDevice::setLastUpdate(const std::string updateTime)
{
    mUpdateTime = updateTime;
}

const std::string& BaseChildDevice::getLastUpdate() const
{
    return mUpdateTime;
}

CatalogItem BaseChildDevice::GetCatalogItem(std::string myId)
{
    CatalogItem item;
    item.DeviceID = getDeviceId();
    if (parentId.empty())
    {
        item.ParentID = myId;
    }
    else
    {
        item.ParentID = parentId;
    }
    item.Name = name.empty() ? "" : Utf8ToGbk(name);

    item.Manufacturer = "VSK";//当为设备时,设备厂商(必选)
    item.Model = "";//当为设备时,设备型号(必选)
    item.Owner = "";//当为设备时,设备归属(必选)
    item.CivilCode = item.DeviceID.substr(0, 6);//行政区域(必选)
    item.Block = "";//警区(可选)
    item.Address = "";//当为设备时,安装地址(必选)
    item.Parental = 0;//当为设备时,是否有子设备(必选)1有,0没有
    item.SafetyWay = 0;//信令安全模式(可选)缺省为0; 0:不采用;2:S/MIME 签名方式;3:S/MIME加密签名同时采用方式; 4:数字摘要方式
    item.RegisterWay = 1;//注册方式(必选)缺省为1;1:符合IETFRFC3261标准的认证注册模式; 2:基于口令的双向认证注册模式; 3:基于数字证书的双向认证注册模式
    item.CertNum = 0;//证书序列号(有证书的设备必选)
    item.Certifiable = 0;//证书有效标识(有证书的设备必选)缺省为0;证书有效标识:0:无效 1:有效
    item.ErrCode = 0;//无效原因码(有证书且证书无效的设备必选)
    item.EndTime = "";//证书终止有效期(有证书的设备必选)
    item.Secrecy = 0;//保密属性(必选)缺省为0;0:不涉密,1:涉密
    item.IPAddress = ChildIp;//设备/区域/系统IP地址(可选)
    item.Port = 0;//设备/区域/系统端口(可选)
    item.Password = "";
    item.Status = status ? "ON" : "OFF";
    item.Longitude = 0;//经度(可选)
    item.Latitude = 0;//纬度(可选)
    return item;
}