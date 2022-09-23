#pragma once

#include <iostream>
#include <memory>
#include <list>
#include <vector>
#include "DeviceChannel.h"

using namespace std;
class Device
{
public:
	enum protocal
	{
		DEV_ACCESS_GB28181,
	};
	virtual string getUuId();
	virtual void setUuId(string uid);
	virtual int getChannelCount();
	virtual void setChannelCount(int chlcount);
	virtual int getOnline();
	virtual void setOnline(int onl);
	//friend class SipServerDeviceInfo;
	virtual protocal getDevAccessProtocal();
	virtual void setDevAccessProtocal(protocal pro);
	virtual string getDeviceId();
	virtual void setDeviceId(const string& devid);
protected:
	//唯Id
	string uuid;
	string deviceId;
	// 通道个数
	int channelCount;
	// 在线
	int online;
	protocal proType;
	list<std::shared_ptr<IDeviceChannel>> mChannelList;
};
class SipServerDeviceInfo :public Device
{
public:
	enum CatalogDataStatus {
		ready, runIng, end
	};
private:
	//设备名
	string name;
	// 生产厂商
	string manufacturer;
	// 型号
	string model;
	// 固件版本
	string firmware;
	// 传输协议
	// UDP/TCP
	string transport;
	/**
	* 数据流传输模式
	* UDP:udp传输
	* TCP-ACTIVE：tcp主动模式
	* TCP-PASSIVE：tcp被动模式
	*/
	string streamMode;
	// wan地址_ip
	string  ip;
	// wan地址_port
	int port;
	// wan地址
	string  hostAddress;
	// 注册时间
	string registerTime;
	//心跳时间
	string keepaliveTime;
	// 注册有效期
	int expires;
	// 创建时间
	string createTime;
	// 更新时间
	string updateTime;
	// 设备使用的媒体id, 默认为null
	string mediaServerId;
	// 字符集, 支持 UTF-8 与 GB2312
	string charset;
	// 目录订阅周期，0为不订阅
	int subscribeCycleForCatalog;
	// 移动设备位置订阅周期，0为不订阅
	int subscribeCycleForMobilePosition;
	// 移动设备位置信息上报时间间隔,单位:秒,默认值5
	int mobilePositionSubmissionInterval;
	// 报警订阅周期，0为不订阅
	int subscribeCycleForAlarm;
	// 是否开启ssrc校验，默认关闭，开启可以防止串流
	string ssrcCheck;

	//list<GBDeviceChannel> mChannelList;

	int mSn; // 命令序列号
	int mTotal;
	int mCurCount;
	string mLastTime;
	string mErrorMsg;
	CatalogDataStatus mCatalogStatus;
public:
	SipServerDeviceInfo();
	SipServerDeviceInfo(const SipServerDeviceInfo& devobj);

	string getName();
	void setName(string nm);

	string getManufacturer();
	void setManufacturer(string manuf);

	string getModel();
	void setModel(string mod);

	string getFirmware();
	void setFirmware(string firm);

	string getTransport();
	void setTransport(string trans);

	string getStreamMode();
	void setStreamMode(string strammode);

	string getIp();
	void setIp(string strip);

	int getPort();
	void setPort(int iport);

	string getHostAddress();
	void setHostAddress(string haddress);

	string getRegisterTime();
	void setRegisterTime(string regtime);

	string getKeepaliveTime();
	void setKeepaliveTime(string keeptime);

	int getExpires();
	void setExpires(int iexpires);

	string getCreateTime();
	void setCreateTime(string ctreattime);

	string getUpdateTime();
	void setUpdateTime(string updatetm);

	string getMediaServerId();
	void setMediaServerId(string mediaSvId);

	string getCharset();
	void setCharset(string chars);

	int getSubscribeCycleForCatalog();
	void setSubscribeCycleForCatalog(int subcatalog);

	int getSubscribeCycleForMobilePosition();
	void setSubscribeCycleForMobilePosition(int subMbPosition);

	int getMobilePositionSubmissionInterval();
	void setMobilePositionSubmissionInterval(int mbPositionInterval);

	int getSubscribeCycleForAlarm();
	void setSubscribeCycleForAlarm(int subalarm);

	string isSsrcCheck();
	void setSsrcCheck(string isssrc);

	list<std::shared_ptr<IDeviceChannel>> getChannelList();
	void setChannelList(list<GBDeviceChannel*> channelList);

	void updateChannelList(list<GBDeviceChannel*> channelList);

	int getSn();
	void setSn(int sn);

	int getTotal();
	void setTotal(int total);

	string getLastTime();
	void setLastTime(string lastTime);

	string getErrorMsg();
	void setErrorMsg(string errorMsg);

	CatalogDataStatus getCatalogStatus();
	void setCatalogStatus(CatalogDataStatus status);

	void addResponseCatalog(GBDeviceChannel* channel);
	void ResponseUpdateCatalog(GBDeviceChannel* channel);

	int getCatalogNum();
	std::shared_ptr<IDeviceChannel> GetGBChannel(string channelId);
};
class OnvifDeviceInfo :public Device
{
public:
	OnvifDeviceInfo() {};
};
typedef struct
{
	string DeviceID;
}CatalogQueryMsg;
typedef struct
{
	string DeviceID;
	string status;
}KeepAliveMsg;
typedef struct
{
	string DeviceID;
	string StartTime;
	string EndTime;
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
	string  DeviceID;
	string  Name;
	string  Manufacturer;
	string  Model;
	string  Owner;
	string  CivilCode;
	string  Block;
	string  Address;
	int	    Parental;
	string  ParentID;
	int	    SafetyWay;
	int     RegisterWay;
	string  CertNum;
	int     Certifiable;
	int     ErrCode;
	string  EndTime;
	int     Secrecy;
	string  IPAddress;
	int		Port;
	string  Password;
	int		Status;
	double  Longitude;
	double  Latitude;
}ResponseCatalogMsg;
struct ResponseCatalogList
{
	unsigned int allnum;
	std::string   fatherdevid;
	int catalogtype;//0:组织机构 1:nvr 2:ipc 3:业务分组 4:系统信息
	vector<ResponseCatalogMsg> m_devVect;
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