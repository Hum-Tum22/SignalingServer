#pragma once

#include <iostream>
#include <memory>
#include <list>
#include <vector>
#include "DeviceChannel.h"

//using namespace std;
class Device
{
public:
	enum protocal
	{
		DEV_ACCESS_GB28181,
	};
	virtual std::string getUuId();
	virtual void setUuId(std::string uid);
	virtual int getChannelCount();
	virtual void setChannelCount(int chlcount);
	virtual int getOnline();
	virtual void setOnline(int onl);
	//friend class SipServerDeviceInfo;
	virtual protocal getDevAccessProtocal();
	virtual void setDevAccessProtocal(protocal pro);
	virtual std::string getDeviceId();
	virtual void setDeviceId(const std::string& devid);
protected:
	//唯Id
	std::string uuid;
	std::string deviceId;
	// 通道个数
	int channelCount;
	// 在线
	int online;
	protocal proType;
	std::list<std::shared_ptr<IDeviceChannel>> mChannelList;
};
class SipServerDeviceInfo :public Device
{
public:
	enum CatalogDataStatus {
		ready, runIng, end
	};
private:
	//设备名
	std::string name;
	// 生产厂商
	std::string manufacturer;
	// 型号
	std::string model;
	// 固件版本
	std::string firmware;
	// 传输协议
	// UDP/TCP
	std::string transport;
	/**
	* 数据流传输模式
	* UDP:udp传输
	* TCP-ACTIVE：tcp主动模式
	* TCP-PASSIVE：tcp被动模式
	*/
	std::string streamMode;
	// wan地址_ip
	std::string  ip;
	// wan地址_port
	int port;
	// wan地址
	std::string  hostAddress;
	// 注册时间
	std::string registerTime;
	//心跳时间
	std::string keepaliveTime;
	// 注册有效期
	int expires;
	// 创建时间
	std::string createTime;
	// 更新时间
	std::string updateTime;
	// 设备使用的媒体id, 默认为null
	std::string mediaServerId;
	// 字符集, 支持 UTF-8 与 GB2312
	std::string charset;
	// 目录订阅周期，0为不订阅
	int subscribeCycleForCatalog;
	// 移动设备位置订阅周期，0为不订阅
	int subscribeCycleForMobilePosition;
	// 移动设备位置信息上报时间间隔,单位:秒,默认值5
	int mobilePositionSubmissionInterval;
	// 报警订阅周期，0为不订阅
	int subscribeCycleForAlarm;
	// 是否开启ssrc校验，默认关闭，开启可以防止串流
	std::string ssrcCheck;

	//list<GBDeviceChannel> mChannelList;

	int mSn; // 命令序列号
	int mTotal;
	int mCurCount;
	std::string mLastTime;
	std::string mErrorMsg;
	CatalogDataStatus mCatalogStatus;
public:
	SipServerDeviceInfo();
	SipServerDeviceInfo(const SipServerDeviceInfo& devobj);

	std::string getName();
	void setName(std::string nm);

	std::string getManufacturer();
	void setManufacturer(std::string manuf);

	std::string getModel();
	void setModel(std::string mod);

	std::string getFirmware();
	void setFirmware(std::string firm);

	std::string getTransport();
	void setTransport(std::string trans);

	std::string getStreamMode();
	void setStreamMode(std::string strammode);

	std::string getIp();
	void setIp(std::string strip);

	int getPort();
	void setPort(int iport);

	std::string getHostAddress();
	void setHostAddress(std::string haddress);

	std::string getRegisterTime();
	void setRegisterTime(std::string regtime);

	std::string getKeepaliveTime();
	void setKeepaliveTime(std::string keeptime);

	int getExpires();
	void setExpires(int iexpires);

	std::string getCreateTime();
	void setCreateTime(std::string ctreattime);

	std::string getUpdateTime();
	void setUpdateTime(std::string updatetm);

	std::string getMediaServerId();
	void setMediaServerId(std::string mediaSvId);

	std::string getCharset();
	void setCharset(std::string chars);

	int getSubscribeCycleForCatalog();
	void setSubscribeCycleForCatalog(int subcatalog);

	int getSubscribeCycleForMobilePosition();
	void setSubscribeCycleForMobilePosition(int subMbPosition);

	int getMobilePositionSubmissionInterval();
	void setMobilePositionSubmissionInterval(int mbPositionInterval);

	int getSubscribeCycleForAlarm();
	void setSubscribeCycleForAlarm(int subalarm);

	std::string isSsrcCheck();
	void setSsrcCheck(std::string isssrc);

	std::list<std::shared_ptr<IDeviceChannel>> getChannelList();
	void setChannelList(std::list<std::shared_ptr<IDeviceChannel>> channelList);

	void updateChannelList(std::list<GBDeviceChannel*> channelList);

	int getSn();
	void setSn(int sn);

	int getTotal();
	void setTotal(int total);

	std::string getLastTime();
	void setLastTime(std::string lastTime);

	std::string getErrorMsg();
	void setErrorMsg(std::string errorMsg);

	CatalogDataStatus getCatalogStatus();
	void setCatalogStatus(CatalogDataStatus status);

	void addResponseCatalog(GBDeviceChannel* channel);
	void ResponseUpdateCatalog(GBDeviceChannel* channel);

	int getCatalogNum();
	std::shared_ptr<IDeviceChannel> GetGBChannel(std::string channelId);
};
class OnvifDeviceInfo :public Device
{
public:
	OnvifDeviceInfo() {};
};
typedef struct
{
	std::string DeviceID;
}CatalogQueryMsg;
typedef struct
{
	std::string DeviceID;
	std::string status;
}KeepAliveMsg;
typedef struct
{
	std::string DeviceID;
	std::string StartTime;
	std::string EndTime;
}CatalogSubscriptionMsg;

typedef struct _ResponseCatalogMsg
{
	_ResponseCatalogMsg()
	{
		Parental = 0;
		SafetyWay = 0;
		RegisterWay = 0;
		Certifiable = 0;
		ErrCode = 0;
		Secrecy = 0;
		Port = 0;
		Status = 0;
		Longitude = 0;
		Latitude = 0;
	}
	std::string  DeviceID;
	std::string  Name;
	std::string  Manufacturer;
	std::string  Model;
	std::string  Owner;
	std::string  CivilCode;
	std::string  Block;
	std::string  Address;
	int	    Parental;
	std::string  ParentID;
	int	    SafetyWay;
	int     RegisterWay;
	std::string  CertNum;
	int     Certifiable;
	int     ErrCode;
	std::string  EndTime;
	int     Secrecy;
	std::string  IPAddress;
	int		Port;
	std::string  Password;
	int		Status;
	double  Longitude;
	double  Latitude;
}ResponseCatalogMsg;
struct ResponseCatalogList
{
	unsigned int allnum;
	std::string   fatherdevid;
	int catalogtype;//0:组织机构 1:nvr 2:ipc 3:业务分组 4:系统信息
	std::vector<ResponseCatalogMsg> m_devVect;
	///std::vector<Organization> m_OrgVect;
	//std::vector<BusinessGroupItem> m_GroupVect;
	//CatalogSystemItem sysinfo;
};

typedef enum
{
	XML_CMD_NAME_UNKNOWN,
	XML_CMD_NAME_QUERY,

	XML_CMD_NAME_RESPONSE,



	//Subscription
	XML_CMD_NAME_SUBSCRIPTION,
	XML_CMD_NAME_NOTIFY,
}XmlCmdName;
typedef enum
{
	//request
	XML_CMDTYPE_UNKNOWN,
	XML_CMDTYPE_REQUEST_CATALOG,
	XML_CMDTYPE_REQUEST_DEVICEINFO,

	XML_CMDTYPE_REQUEST_KEEPALIVE,

	//Subscription
	XML_CMDTYPE_REQUEST_CATALOG_SUBSCRIPTION,
	XML_CMDTYPE_REQUEST_ALARM_SUBSCRIPTION,


	//response
	XML_CMDTYPE_RESPONSE_CATALOG
}XmlCmdType;
struct GB28181XmlMsg
{
	XmlCmdName cmdname;
	XmlCmdType cmdtype;
	unsigned int sn;
	std::string DeviceID;
	void* pPoint;//the point to some memery just like this

	GB28181XmlMsg();
	virtual ~GB28181XmlMsg();
};