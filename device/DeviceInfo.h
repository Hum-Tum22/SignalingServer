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
	//ΨId
	std::string uuid;
	std::string deviceId;
	// ͨ������
	int channelCount;
	// ����
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
	//�豸��
	std::string name;
	// ��������
	std::string manufacturer;
	// �ͺ�
	std::string model;
	// �̼��汾
	std::string firmware;
	// ����Э��
	// UDP/TCP
	std::string transport;
	/**
	* ����������ģʽ
	* UDP:udp����
	* TCP-ACTIVE��tcp����ģʽ
	* TCP-PASSIVE��tcp����ģʽ
	*/
	std::string streamMode;
	// wan��ַ_ip
	std::string  ip;
	// wan��ַ_port
	int port;
	// wan��ַ
	std::string  hostAddress;
	// ע��ʱ��
	std::string registerTime;
	//����ʱ��
	std::string keepaliveTime;
	// ע����Ч��
	int expires;
	// ����ʱ��
	std::string createTime;
	// ����ʱ��
	std::string updateTime;
	// �豸ʹ�õ�ý��id, Ĭ��Ϊnull
	std::string mediaServerId;
	// �ַ���, ֧�� UTF-8 �� GB2312
	std::string charset;
	// Ŀ¼�������ڣ�0Ϊ������
	int subscribeCycleForCatalog;
	// �ƶ��豸λ�ö������ڣ�0Ϊ������
	int subscribeCycleForMobilePosition;
	// �ƶ��豸λ����Ϣ�ϱ�ʱ����,��λ:��,Ĭ��ֵ5
	int mobilePositionSubmissionInterval;
	// �����������ڣ�0Ϊ������
	int subscribeCycleForAlarm;
	// �Ƿ���ssrcУ�飬Ĭ�Ϲرգ��������Է�ֹ����
	std::string ssrcCheck;

	//list<GBDeviceChannel> mChannelList;

	int mSn; // �������к�
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
	int catalogtype;//0:��֯���� 1:nvr 2:ipc 3:ҵ����� 4:ϵͳ��Ϣ
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