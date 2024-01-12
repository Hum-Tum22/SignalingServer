#pragma once
#include <list>
#include <map>
#include <mutex>
#include "resip/dum/ServerSubscription.hxx"
#include "../device/DeviceInfo.h"

struct SubscriptionInfo
{
	resip::Data user;
	resip::ServerSubscriptionHandle m_ssph;
	resip::Data m_EventType;
};
struct AlarmSubscriptionCondition
{
	resip::Data Devid;
	int  iAlarmType;
};
struct AlarmSubscriptionInfo :public SubscriptionInfo
{
	AlarmSubscriptionInfo() :GetAlarmNumState(0) {}
	std::list<AlarmSubscriptionCondition> DevIdList;
	int GetAlarmNumState;//0:ALL,1 仅限于 DevIdList 范围内的 报警
	int isDevIdSubscrip(resip::Data DevID);
	int isDevIdSubscrip(resip::Data DevID1, resip::Data DevID2);
};
struct CatalogSubScription :public  SubscriptionInfo
{
	std::string DeviceID;
	std::string StartTime;
	std::string EndTime;
};



class CSubscriptionMrg
{
public:
	typedef enum
	{
		EVENT_ON,
		EVENT_OFF,
		EVENT_VLOST,
		EVENT_DEFECT,
		EVENT_ADD,
		EVENT_DEL,
		EVENT_UPDATE
	}NOTIFY_EVENT;
protected:
	// The key is user id,
	std::map<resip::Data, AlarmSubscriptionInfo> AlarmSubscriptionMap;
	std::mutex mAlarmMtx;

	std::map<resip::Data, CatalogSubScription> CatalogSubscriptionMap;
	std::mutex mCatalogSubMapMtx;
public:
	CSubscriptionMrg(void);
	~CSubscriptionMrg(void);
	static CSubscriptionMrg& Instance();

	static std::string eventToString(NOTIFY_EVENT);

	//bool NotifyAlarm(AlarmInfo& stAlarmInfo);
	bool NotifyCatalog(const std::string& content, const std::string& devid, const std::string& parentid);
	bool NotifyCatalogByHandle(resip::ServerSubscriptionHandle &m_ssph, const std::string& content);

	bool AddCatalogSubscription(resip::Data user, resip::ServerSubscriptionHandle m_ssph, resip::Data eventtype, const GB28181XmlMsg&);
	void DeleteSubscription(resip::ServerSubscriptionHandle ssph, resip::Data& ev);
	void DeleteSubscription(resip::ServerSubscriptionHandle ssph);

	bool existsSubscription(resip::Data user, resip::ServerSubscriptionHandle ssph, resip::Data eventtype);
};