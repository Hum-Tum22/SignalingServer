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
//分类 名称 说明
//请求命令
//Control 表示一个控制的动作
//Query 表示一个查询的动作
//Notify 表示一个通知的动作
//应答命令 Response 表示一个请求动作的应答
typedef enum
{
	XML_CMD_NAME_UNKNOWN,
	//请求命令
	XML_CMD_NAME_CONTROL,//表示一个控制的动作
	XML_CMD_NAME_QUERY,//表示一个查询的动作
	XML_CMD_NAME_NOTIFY,//表示一个通知的动作
	//应答命令
	XML_CMD_NAME_RESPONSE,//表示一个请求动作的应答
}XmlCmdName;

//deviceIDType 设备编码类型
//statusType 状态类型
//resultType 结果类型
//PTZType 控制码类型
//recordType 录像控制类型
//guardType 布防 / 撤防控制类型
//itemType 设备目录项类型
//itemFileType 文件目录项类型
typedef enum
{
	XML_CMDTYPE_UNKNOWN,

	//控制命令
	//DeviceControl 设备控制
	XML_CMDTYPE_DEVICE_CONTROL,
	//DeviceConfig 设备配置
	XML_CMDTYPE_DEVICE_CONFIG,
	//查询命令
	//DeviceStatus 设备状态查询
	XML_CMDTYPE_DEVICE_STATUS,
	//Catalog 设备目录查询
	XML_CMDTYPE_CATALOG,
	//DeviceInfo 设备信息查询
	XML_CMDTYPE_DEVICE_INFO,
	//RecordInfo 文件目录检索
	XML_CMDTYPE_RECORDINFO,
	//Alarm 报警查询
	XML_CMDTYPE_ALARM,
	//ConfigDownload 设备配置查询
	XML_CMDTYPE_CONFIG_DOWNLOAD,
	//PresetQuery 预置位查询
	XML_CMDTYPE_PRESET_QUERY,
	//MobilePosition 移动设备位置数据查询
	XML_CMDTYPE_MOBILE_POSITION,

	//通知命令
	//Keepalive 设备状态信息报送
	XML_CMDTYPE_NOTIFY_KEEPALIVE,
	//Alarm 报警通知
	XML_CMDTYPE_NOTIFY_ALARM,
	//MediaStatus 媒体通知
	XML_CMDTYPE_NOTIFY_MEDIA_STATUS,
	//Broadcast 广播通知
	XML_CMDTYPE_NOTIFY_BROADCAST,
	//MobilePosition 移动设备位置数据通知
	XML_CMDTYPE_NOTIFY_MOBILE_POSITION,
	//Catalog 目录通知
	XML_CMDTYPE_NOTIFY_CATALOG,

	//应答命令
	//DeviceControl 设备控制应答
	XML_CMDTYPE_RESPONSE_DEV_CONTROL,
	//Alarm 报警通知应答
	XML_CMDTYPE_RESPONSE_ALARM,
	//Catalog 设备目录信息查询应答
	XML_CMDTYPE_RESPONSE_CATALOG_ITEM,
	//Catalog 目录信息查询收到应答
	XML_CMDTYPE_RESPONSE_SUB_CATALOG,
	//Catalog 目录收到应答received
	XML_CMDTYPE_RESPONSE_CATALOG_NOTIFY_RECEIVED,
	//DeviceInfo 设备信息查询应答
	XML_CMDTYPE_RESPONSE_DEV_INFO,
	//DeviceStatus 设备状态信息查询应答
	XML_CMDTYPE_RESPONSE_DEV_STATUS,
	//RecordInfo 文件目录检索应答
	XML_CMDTYPE_RESPONSE_RECORD_INFO,
	//DeviceConfig 设备配置应答
	XML_CMDTYPE_RESPONSE_DEV_CONFIG,
	//ConfigDownload 设备配置查询应答
	XML_CMDTYPE_RESPONSE_CONFIG_DOWNLOAD,
	//PresetQuery 设备预置位查询应答
	XML_CMDTYPE_RESPONSE_PRESET_QUERY,
	//Broadcast 语音广播应答
	XML_CMDTYPE_RESPONSE_BROADCAST,

}XmlCmdType;

typedef enum
{
	XML_CONTROLCMD_UNKNOWN,
	//PTZCmd 球机/云台控制命令
	XML_CONTROLCMD_PTZ,
	//TeleBoot 远程启动控制命令
	XML_CONTROLCMD_TELEBOOT,
	//RecordCmd 录像控制命令
	XML_CONTROLCMD_RECORD,
	//GuardCmd 报警布防/撤防命令
	XML_CONTROLCMD_GUARD,
	//AlarmCmd 报警复位命令
	XML_CONTROLCMD_ALARM,
	//IFameCmd 强制关键帧命令,设备收到此命令应立刻发送一个IDR帧
	XML_CONTROLCMD_IFRAME,
	//DragZoomIn 拉框放大控制命令
	XML_CONTROLCMD_DRAGZOOMIN,
	//DragZoomOut 拉框缩小控制命令
	XML_CONTROLCMD_DRAGZOOMOUT,
	//HomePosition 看守位控制命令
	XML_CONTROLCMD_HOMEPOSITION
}XmlControlCmd;
typedef struct
{
	std::string value;
	int ControlPriority;
}PtzControlInfo;
typedef struct
{
	std::string value;
}BootControlInfo;
typedef struct
{
	std::string value;
}RecordControlInfo;
typedef struct
{
	std::string value;
}GuardControlInfo;
typedef struct
{
	std::string value;
}AlarmControlInfo;
typedef struct
{
	std::string value;
}IFameControlInfo;
typedef struct
{
	std::string value;
}DragZoomControlInfo;
typedef struct
{
	std::string value;
}HomePositionControlInfo;

typedef struct
{
	std::string value;
}DeviceConfigInfo;

struct GB28181XmlMsg
{
	XmlCmdName cmdname;
	XmlCmdType cmdtype;
	XmlControlCmd controlCmd;
	unsigned int sn;
	std::string DeviceID;
	void* pPoint;//the point to some memery just like this

	GB28181XmlMsg();
	virtual ~GB28181XmlMsg();
};