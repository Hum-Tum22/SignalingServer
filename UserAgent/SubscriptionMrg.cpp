#include "SubscriptionMrg.h"
#include "uainfo.h"
#include "SipServerConfig.h"

#include <resip/stack/PlainContents.hxx>

int AlarmSubscriptionInfo::isDevIdSubscrip(resip::Data DevID)
{
	std::list<AlarmSubscriptionCondition>::iterator it = DevIdList.begin();
	for (; it != DevIdList.end(); it++)
	{
		if ((*it).Devid == DevID)
		{
			return 1;
		}
	}
	return 0;
}
int AlarmSubscriptionInfo::isDevIdSubscrip(resip::Data DevID1, resip::Data DevID2)
{

	std::list<AlarmSubscriptionCondition>::iterator it = DevIdList.begin();
	for (; it != DevIdList.end(); it++)
	{
		if ((*it).Devid == DevID1 || (*it).Devid == DevID2)
		{
			return 1;
		}
	}
	return 0;
}

CSubscriptionMrg& CSubscriptionMrg::Instance()
{
	static CSubscriptionMrg g_subscriptionmrg;
	return g_subscriptionmrg;
}
CSubscriptionMrg::CSubscriptionMrg(void)
{
}

CSubscriptionMrg::~CSubscriptionMrg(void)
{
}

//bool CSubscriptionMrg::NotifyAlarm(AlarmInfo& stAlarmInfo)
//{
//	//过滤那些用户是需要报警信息的
//	if (AlarmSubscriptionMap.size() <= 0)
//	{
//		return false;
//	}
//	//查找设备信息
//	try
//	{
//		// 如果加锁的话 会出多条信息只能发送，
//		// 当前使用的 协议栈使用的是单线程模式 也没什么区别
//		/*Data xmlAlarm = Data::Empty;
//		IntelLock locer(&m_ASM_Mutex);
//		std::map<Data, AlarmSubscriptionInfo >::iterator iter = AlarmSubscriptionMap.begin();
//		for (; iter != AlarmSubscriptionMap.end(); iter++)
//		{
//			if (iter->second.GetAlarmNumState == 0)
//			{
//				if (iter->second.m_ssph.isValid())
//				{
//					if (xmlAlarm == Data::Empty)
//					{
//						CXMLPraser xmplpras;
//						xmplpras.MakeAlarmNotifyHeard(CreateMsgID());
//						xmplpras.AddAlarmInfoToXmlString(stAlarmInfo);
//						xmplpras.PrintData(xmlAlarm);
//					}
//					PlainContents tContent(xmlAlarm, Mime("Application", "MANSCDP+xml"));
//					iter->second.m_ssph->send(iter->second.m_ssph->update(&tContent));
//				}
//			}
//			else if (iter->second.isDevIdSubscrip(stAlarmInfo.DevId, stAlarmInfo.SubId))
//			{
//				if (xmlAlarm == Data::Empty)
//				{
//					CXMLPraser xmplpras;
//					xmplpras.MakeAlarmNotifyHeard(CreateMsgID());
//					xmplpras.AddAlarmInfoToXmlString(stAlarmInfo);
//					xmplpras.PrintData(xmlAlarm);
//				}
//				PlainContents tContent(xmlAlarm, Mime("Application", "MANSCDP+xml"));
//				if (iter->second.m_ssph.isValid())
//					iter->second.m_ssph->send(iter->second.m_ssph->update(&tContent));
//			}
//		}*/
//	}
//	catch (...)
//	{
//
//	}
//	return 1;
//}
bool CSubscriptionMrg::NotifyCatalog(const std::string& content, const std::string& devid, const std::string& parentid)
{
	try
	{
		CUSTORLOCKGUARD clocker(mCatalogSubMapMtx);
		std::map<resip::Data, CatalogSubScription >::iterator iter = CatalogSubscriptionMap.begin();
		for (; iter != CatalogSubscriptionMap.end(); iter++)
		{

			if (iter->second.DeviceID.empty())
			{
				if (iter->second.m_ssph.isValid())
				{
					PlainContents tContent(content.c_str(), Mime("Application", "MANSCDP+xml"));
					iter->second.m_ssph->sendCommand(iter->second.m_ssph->update(&tContent));
				}
			}
			else
			{
				MyServerConfig& svr = GetSipServerConfig();
				std::string myId = svr.getConfigData("GBID", "34020000002000000001", true).c_str();
				if (iter->second.DeviceID == myId)
				{
					if (iter->second.m_ssph.isValid())
					{
						PlainContents tContent(content.c_str(), Mime("Application", "MANSCDP+xml"));
						iter->second.m_ssph->sendCommand(iter->second.m_ssph->update(&tContent));
					}
				}
				else if (iter->second.DeviceID == devid || iter->second.DeviceID == parentid)
				{
					if (iter->second.m_ssph.isValid())
					{
						PlainContents tContent(content.c_str(), Mime("Application", "MANSCDP+xml"));
						iter->second.m_ssph->sendCommand(iter->second.m_ssph->update(&tContent));
					}
				}
			}
		}
		return true;
	}
	catch (...)
	{
	}
	return false;
}
bool CSubscriptionMrg::AddCatalogSubscription(resip::Data user, resip::ServerSubscriptionHandle m_ssph, resip::Data eventtype, const GB28181XmlMsg& catalog)
{
	try
	{
		if (eventtype == "Catalog" || eventtype == "catalagitem")
		{
			CatalogSubScription catalogSub;

			catalogSub.DeviceID = catalog.DeviceID.c_str();
			CatalogQueryMsg* pCatalogQueryMsg = (CatalogQueryMsg*)catalog.pPoint;
			if (pCatalogQueryMsg)
			{
				catalogSub.EndTime = pCatalogQueryMsg->StartTime.c_str();
				catalogSub.StartTime = pCatalogQueryMsg->EndTime.c_str();
			}
			catalogSub.m_ssph = m_ssph;
			catalogSub.user = user;
			catalogSub.m_EventType = eventtype;

			CUSTORLOCKGUARD clocker(mCatalogSubMapMtx);
			CatalogSubscriptionMap[user] = catalogSub;
			return true;
		}
	}
	catch (...)
	{
	}
	return false;
}
void CSubscriptionMrg::DeleteSubscription(resip::ServerSubscriptionHandle ssph, resip::Data& ev)
{
	try
	{
		if (ev == "Catalog" || ev == "catalagitem")
		{
			CUSTORLOCKGUARD clocker(mCatalogSubMapMtx);
			auto iterC = CatalogSubscriptionMap.begin();
			while (iterC != CatalogSubscriptionMap.end())
			{
				auto tempItem = std::next(iterC);
				if (iterC->second.m_ssph == ssph)
				{
					CatalogSubscriptionMap.erase(iterC);
					ssph->end();
				}
				iterC = tempItem;
			}
		}
		else if (ev == "Alarm" || ev == "e_Alarm")
		{
			CUSTORLOCKGUARD alocker(mAlarmMtx);
			auto iter = AlarmSubscriptionMap.begin();
			while (iter != AlarmSubscriptionMap.end())
			{
				auto tempItem = std::next(iter);
				if (iter->second.m_ssph == ssph)
				{
					AlarmSubscriptionMap.erase(iter);
					ssph->end();
				}
				iter = tempItem;
			}
		}
		else if (ev == "MobilePosition")
		{

		}
		else if(ev == "presence")
		{

		}
		else
		{
			ssph->end();
		}
	}
	catch (...)
	{

	}
}
void CSubscriptionMrg::DeleteSubscription(resip::ServerSubscriptionHandle ssph)
{
	try
	{
		{
			CUSTORLOCKGUARD alocker(mAlarmMtx);
			auto iter = AlarmSubscriptionMap.begin();
			while (iter != AlarmSubscriptionMap.end())
			{
				auto tempItem = std::next(iter);
				if (iter->second.m_ssph == ssph)
				{
					AlarmSubscriptionMap.erase(iter);
					ssph->end();
				}
				iter = tempItem;
			}
		}
		
		{
			CUSTORLOCKGUARD clocker(mCatalogSubMapMtx);
			auto iterC = CatalogSubscriptionMap.begin();
			while (iterC != CatalogSubscriptionMap.end())
			{
				auto tempItem = std::next(iterC);
				if (iterC->second.m_ssph == ssph)
				{
					CatalogSubscriptionMap.erase(iterC);
					ssph->end();
				}
				iterC = tempItem;
			}
		}
	}
	catch (...)
	{

	}
}
std::string CSubscriptionMrg::eventToString(NOTIFY_EVENT ev)
{
	switch (ev)
	{
	case EVENT_ON:
		return "ON";
	case EVENT_OFF:
		return "OFF";
	case EVENT_VLOST:
		return "VLOST";
	case EVENT_DEFECT:
		return "DEFECT";
	case EVENT_ADD:
		return "ADD";
	case EVENT_DEL:
		return "DEL";
	case EVENT_UPDATE:
		return "UPDATE";
	default:
		return "";
	}
}