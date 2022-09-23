#include "DeviceInfo.h"
#include <string>

string Device::getUuId()
{
	return uuid;
}
void Device::setUuId(string uid)
{
	uuid = uid;
}
int Device::getChannelCount()
{
	return channelCount;
}
void Device::setChannelCount(int chlcount)
{
	channelCount = chlcount;
}

int Device::getOnline()
{
	return online;
}
void Device::setOnline(int onl)
{
	online = onl;
}
Device::protocal Device::getDevAccessProtocal()
{
	return proType;
}
void Device::setDevAccessProtocal(Device::protocal pro)
{
	proType = pro;
}
string Device::getDeviceId()
{
	return deviceId;
}
void Device::setDeviceId(const string& devid)
{
	deviceId = devid;
}

SipServerDeviceInfo::SipServerDeviceInfo():port(0)
, expires(0)
, subscribeCycleForCatalog(0)
, subscribeCycleForMobilePosition(0)
, mobilePositionSubmissionInterval(5)
, subscribeCycleForAlarm(0)
, mCatalogStatus(CatalogDataStatus::end)
{
	uuid = "";
	channelCount = 0;
	online = 0;
}
SipServerDeviceInfo::SipServerDeviceInfo(const SipServerDeviceInfo& devobj):name(devobj.name)
, manufacturer(devobj.manufacturer)
, model(devobj.model)
, firmware(devobj.firmware)
, transport(devobj.transport)
, streamMode(devobj.streamMode)
, ip(devobj.ip)
, port(devobj.port)
, hostAddress(devobj.hostAddress)
, registerTime(devobj.registerTime)
, keepaliveTime(devobj.keepaliveTime)
, expires(devobj.expires)
, createTime(devobj.createTime)
, updateTime(devobj.updateTime)
, mediaServerId(devobj.mediaServerId)
, charset(devobj.charset)
, subscribeCycleForCatalog(devobj.subscribeCycleForCatalog)
, subscribeCycleForMobilePosition(devobj.subscribeCycleForMobilePosition)
, mobilePositionSubmissionInterval(devobj.mobilePositionSubmissionInterval)
, subscribeCycleForAlarm(devobj.subscribeCycleForAlarm)
, ssrcCheck(devobj.ssrcCheck)
, mCatalogStatus(devobj.mCatalogStatus)
{
	uuid = devobj.uuid;
	channelCount = devobj.channelCount;
	online = devobj.online;
	deviceId = devobj.deviceId;
	mChannelList = devobj.mChannelList;
}

string SipServerDeviceInfo::getName()
{
	return name;
}
void SipServerDeviceInfo::setName(string nm)
{
	name = nm;
}

string SipServerDeviceInfo::getManufacturer()
{
	return manufacturer;
}
void SipServerDeviceInfo::setManufacturer(string manuf)
{
	manufacturer = manuf;
}

string SipServerDeviceInfo::getModel()
{
	return model;
}
void SipServerDeviceInfo::setModel(string mod)
{
	model = mod;
}

string SipServerDeviceInfo::getFirmware()
{
	return firmware;
}
void SipServerDeviceInfo::setFirmware(string firm)
{
	firmware = firm;
}

string SipServerDeviceInfo::getTransport()
{
	return transport;
}
void SipServerDeviceInfo::setTransport(string trans)
{
	transport = trans;
}

string SipServerDeviceInfo::getStreamMode()
{
	return streamMode;
}
void SipServerDeviceInfo::setStreamMode(string strammode)
{
	streamMode = strammode;
}

string SipServerDeviceInfo::getIp()
{
	return ip;
}
void SipServerDeviceInfo::setIp(string strip)
{
	ip = strip;
}

int SipServerDeviceInfo::getPort()
{
	return port;
}
void SipServerDeviceInfo::setPort(int iport)
{
	port = iport;
}

string SipServerDeviceInfo::getHostAddress()
{
	return hostAddress;
}
void SipServerDeviceInfo::setHostAddress(string haddress)
{
	hostAddress = haddress;
}

string SipServerDeviceInfo::getRegisterTime()
{
	return registerTime;
}
void SipServerDeviceInfo::setRegisterTime(string regtime)
{
	registerTime = regtime;
}

string SipServerDeviceInfo::getKeepaliveTime()
{
	return keepaliveTime;
}
void SipServerDeviceInfo::setKeepaliveTime(string keeptime)
{
	keepaliveTime = keeptime;
}

int SipServerDeviceInfo::getExpires()
{
	return expires;
}
void SipServerDeviceInfo::setExpires(int iexpires)
{
	expires = iexpires;
}

string SipServerDeviceInfo::getCreateTime()
{
	return createTime;
}
void SipServerDeviceInfo::setCreateTime(string ctreattime)
{
	createTime = ctreattime;
}

string SipServerDeviceInfo::getUpdateTime()
{
	return updateTime;
}
void SipServerDeviceInfo::setUpdateTime(string updatetm)
{
	updateTime = updatetm;
}

string SipServerDeviceInfo::getMediaServerId()
{
	return mediaServerId;
}
void SipServerDeviceInfo::setMediaServerId(string mediaSvId)
{
	mediaServerId = mediaSvId;
}

string SipServerDeviceInfo::getCharset()
{
	return charset;
}
void SipServerDeviceInfo::setCharset(string chars)
{
	charset = chars;
}

int SipServerDeviceInfo::getSubscribeCycleForCatalog()
{
	return subscribeCycleForCatalog;
}
void SipServerDeviceInfo::setSubscribeCycleForCatalog(int subcatalog)
{
	subscribeCycleForCatalog = subcatalog;
}

int SipServerDeviceInfo::getSubscribeCycleForMobilePosition()
{
	return subscribeCycleForMobilePosition;
}
void SipServerDeviceInfo::setSubscribeCycleForMobilePosition(int subMbPosition)
{
	subscribeCycleForMobilePosition = subMbPosition;
}

int SipServerDeviceInfo::getMobilePositionSubmissionInterval()
{
	return mobilePositionSubmissionInterval;
}
void SipServerDeviceInfo::setMobilePositionSubmissionInterval(int mbPositionInterval)
{
	mobilePositionSubmissionInterval = mbPositionInterval;
}

int SipServerDeviceInfo::getSubscribeCycleForAlarm()
{
	return subscribeCycleForAlarm;
}
void SipServerDeviceInfo::setSubscribeCycleForAlarm(int subalarm)
{
	subscribeCycleForAlarm = subalarm;
}

string SipServerDeviceInfo::isSsrcCheck()
{
	return ssrcCheck;
}
void SipServerDeviceInfo::setSsrcCheck(string isssrc)
{
	ssrcCheck = isssrc;
}
list<std::shared_ptr<IDeviceChannel>> SipServerDeviceInfo::getChannelList()
{
	return  mChannelList;
}
void SipServerDeviceInfo::setChannelList(list<GBDeviceChannel*> channelList)
{
	//mChannelList = channelList;
}
void SipServerDeviceInfo::updateChannelList(list<GBDeviceChannel*> channelList)
{
	/*for (auto it = mChannelList.begin(); it != mChannelList.end(); it++)
	{
		delete* it;
		mChannelList.erase(it++);
	}
	for (auto& iter : channelList)
	{
		mChannelList.push_back(iter);
	}*/
}
int SipServerDeviceInfo::getSn()
{
	return mSn;
}
void SipServerDeviceInfo::setSn(int sn)
{
	mSn = sn;
}
int SipServerDeviceInfo::getTotal()
{
	return mTotal;
}
void SipServerDeviceInfo::setTotal(int total)
{
	mTotal = total;
	mCurCount = 0;
}

string SipServerDeviceInfo::getLastTime() {
	return mLastTime;
}
void SipServerDeviceInfo::setLastTime(string lastTime)
{
	mLastTime = lastTime;
}
string SipServerDeviceInfo::getErrorMsg() {
	return mErrorMsg;
}
void SipServerDeviceInfo::setErrorMsg(string errorMsg)
{
	mErrorMsg = errorMsg;
}

SipServerDeviceInfo::CatalogDataStatus SipServerDeviceInfo::getCatalogStatus()
{
	return mCatalogStatus;
}
void SipServerDeviceInfo::setCatalogStatus(SipServerDeviceInfo::CatalogDataStatus status)
{
	mCatalogStatus = status;
}
void SipServerDeviceInfo::addResponseCatalog(GBDeviceChannel* channel)
{
	mChannelList.push_back(std::make_shared<GBDeviceChannel>(*channel));
	mCurCount++;
}
void SipServerDeviceInfo::ResponseUpdateCatalog(GBDeviceChannel* channel)
{
	//for()
}
int SipServerDeviceInfo::getCatalogNum()
{
	return mCurCount;
}
std::shared_ptr<IDeviceChannel> SipServerDeviceInfo::GetGBChannel(string channelId)
{
	for (auto& it : mChannelList)
	{
		if (proType == Device::DEV_ACCESS_GB28181)
		{
			std::shared_ptr<GBDeviceChannel> chl = std::static_pointer_cast<GBDeviceChannel>(it);
			if (chl && chl->getChannelId() == channelId)
				return it;
		}
		
	}
	return NULL;
}