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
	//ΨId
	string uuid;
	string deviceId;
	// ͨ������
	int channelCount;
	// ����
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
	//�豸��
	string name;
	// ��������
	string manufacturer;
	// �ͺ�
	string model;
	// �̼��汾
	string firmware;
	// ����Э��
	// UDP/TCP
	string transport;
	/**
	* ����������ģʽ
	* UDP:udp����
	* TCP-ACTIVE��tcp����ģʽ
	* TCP-PASSIVE��tcp����ģʽ
	*/
	string streamMode;
	// wan��ַ_ip
	string  ip;
	// wan��ַ_port
	int port;
	// wan��ַ
	string  hostAddress;
	// ע��ʱ��
	string registerTime;
	//����ʱ��
	string keepaliveTime;
	// ע����Ч��
	int expires;
	// ����ʱ��
	string createTime;
	// ����ʱ��
	string updateTime;
	// �豸ʹ�õ�ý��id, Ĭ��Ϊnull
	string mediaServerId;
	// �ַ���, ֧�� UTF-8 �� GB2312
	string charset;
	// Ŀ¼�������ڣ�0Ϊ������
	int subscribeCycleForCatalog;
	// �ƶ��豸λ�ö������ڣ�0Ϊ������
	int subscribeCycleForMobilePosition;
	// �ƶ��豸λ����Ϣ�ϱ�ʱ����,��λ:��,Ĭ��ֵ5
	int mobilePositionSubmissionInterval;
	// �����������ڣ�0Ϊ������
	int subscribeCycleForAlarm;
	// �Ƿ���ssrcУ�飬Ĭ�Ϲرգ��������Է�ֹ����
	string ssrcCheck;

	//list<GBDeviceChannel> mChannelList;

	int mSn; // �������к�
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
	int catalogtype;//0:��֯���� 1:nvr 2:ipc 3:ҵ����� 4:ϵͳ��Ϣ
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