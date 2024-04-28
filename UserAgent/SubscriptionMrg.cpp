#include "SubscriptionMrg.h"
#include "uainfo.h"
#include "SipServerConfig.h"
#include "../tools/ThreadPool.h"
#include "SipServer.h"
#include "http.h"

#include <resip/stack/PlainContents.hxx>
#include <rutil/DnsUtil.hxx>
#include <rutil/Log.hxx>
#include <rutil/Logger.hxx>

#include "myjsondef.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"

using namespace resip;
#define RESIPROCATE_SUBSYSTEM Subsystem::TEST


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
	DebugLog(<< " CSubscriptionMrg start notify devid:" << devid);
	try
	{
		std::vector<resip::ServerSubscriptionHandle> vSubs;
		{
			CUSTORLOCKGUARD clocker(mCatalogSubMapMtx);
			std::map<resip::Data, CatalogSubScription >::iterator iter = CatalogSubscriptionMap.begin();
			for (; iter != CatalogSubscriptionMap.end(); iter++)
			{

				if (iter->second.DeviceID.empty())
				{
					if (iter->second.m_ssph.isValid())
					{
						vSubs.push_back(iter->second.m_ssph);
						/*PlainContents tContent(content.c_str(), Mime("Application", "MANSCDP+xml"));
						iter->second.m_ssph->send(iter->second.m_ssph->update(&tContent));*/
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
							vSubs.push_back(iter->second.m_ssph);
							/*PlainContents tContent(content.c_str(), Mime("Application", "MANSCDP+xml"));
							iter->second.m_ssph->send(iter->second.m_ssph->update(&tContent));*/
						}
					}
					else if (iter->second.DeviceID == devid || iter->second.DeviceID == parentid)
					{
						if (iter->second.m_ssph.isValid())
						{
							/*PlainContents tContent(content.c_str(), Mime("Application", "MANSCDP+xml"));
							iter->second.m_ssph->send(iter->second.m_ssph->update(&tContent));*/
						}
					}
				}
			}
		}
		DebugLog(<< " CSubscriptionMrg --- notify devid:" << devid);
		for (auto& it : vSubs)
		{
			if (it.isValid())
			{
				PlainContents tContent(content.c_str(), Mime("Application", "MANSCDP+xml"));
				it->send(it->update(&tContent));
			}
		}
		DebugLog(<< " CSubscriptionMrg end notify devid:" << devid);
		return true;
	}
	catch (...)
	{
	}
	DebugLog(<< " CSubscriptionMrg end 1 notify devid:" << devid);
	return false;
}

bool CSubscriptionMrg::NotifyCatalogByHandle(resip::ServerSubscriptionHandle &ssph, const std::string& content)
{
	if (ssph.isValid())
	{
		PlainContents tContent(content.c_str(), Mime("Application", "MANSCDP+xml"));
		ssph->send(ssph->update(&tContent));
	}
}
bool CSubscriptionMrg::AddCatalogSubscription(resip::Data user, resip::ServerSubscriptionHandle m_ssph, resip::Data eventtype, const GB28181XmlMsg& catalog)
{
	DebugLog(<< " CSubscriptionMrg start add user:" << user);
	try
	{
		if (eventtype == "Catalog" || eventtype == "catalagitem")
		{
			resip::ServerSubscriptionHandle delssph;
			{
				DebugLog(<< " CSubscriptionMrg --- 1 add handle:" << m_ssph.get());
				CUSTORLOCKGUARD clocker(mCatalogSubMapMtx);
				DebugLog(<< " CSubscriptionMrg --- 2 add handle:" << m_ssph.get());
				auto item = CatalogSubscriptionMap.find(user);
				if (item != CatalogSubscriptionMap.end())
				{
					DebugLog(<< " CSubscriptionMrg --- 3 add handle:" << m_ssph.get());
					if (item->second.m_ssph.isValid() && item->second.m_ssph == m_ssph)
					{
						CatalogSubscriptionMap[user].DeviceID = catalog.DeviceID.c_str();
						CatalogQueryMsg* pCatalogQueryMsg = (CatalogQueryMsg*)catalog.pPoint;
						if (pCatalogQueryMsg)
						{
							CatalogSubscriptionMap[user].EndTime = pCatalogQueryMsg->StartTime.c_str();
							CatalogSubscriptionMap[user].StartTime = pCatalogQueryMsg->EndTime.c_str();
						}
						DebugLog(<< " CSubscriptionMrg end 2 add user:" << user);
						return true;
					}
					else
					{
						if (item->second.m_ssph.isValid())
						{
							delssph = item->second.m_ssph;
						}
						DebugLog(<< " CSubscriptionMrg --- 4 add handle:" << m_ssph.get());
						CatalogSubscriptionMap.erase(user);
						DebugLog(<< " CSubscriptionMrg --- 5 add handle:" << m_ssph.get());
					}
				}
			}
			DebugLog(<< " CSubscriptionMrg --- 6 add handle:" << m_ssph.get());
			if (delssph.isValid())
			{
				delssph->end();
			}
			DebugLog(<< " CSubscriptionMrg --- 7 add handle:" << m_ssph.get());
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
			DebugLog(<< " CSubscriptionMrg end add user:" << user);
			return true;
		}
	}
	catch (...)
	{
	}
	DebugLog(<< " CSubscriptionMrg end 1 add user:" << user);
	return false;
}
void CSubscriptionMrg::DeleteSubscription(resip::ServerSubscriptionHandle ssph, resip::Data& ev)
{
	DebugLog(<< " CSubscriptionMrg end delete event:" << ev);
	try
	{
		std::vector<resip::ServerSubscriptionHandle> vSubs;
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
					vSubs.push_back(ssph);
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
					vSubs.push_back(ssph);
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
			vSubs.push_back(ssph);
		}
		for (auto& it : vSubs)
		{
			if (it.isValid())
			{
				it->end();
			}
		}
	}
	catch (...)
	{

	}
	DebugLog(<< " CSubscriptionMrg end delete event:" << ev);
}
void CSubscriptionMrg::DeleteSubscription(resip::ServerSubscriptionHandle &ssph)
{
	DebugLog(<< " CSubscriptionMrg start delete handle:" << ssph.get());
	try
	{
		{
			CUSTORLOCKGUARD alocker(mAlarmMtx);
			DebugLog(<< " CSubscriptionMrg start 1111 delete handle:" << ssph.get());
			auto iter = AlarmSubscriptionMap.begin();
			while (iter != AlarmSubscriptionMap.end())
			{
				DebugLog(<< " CSubscriptionMrg start 2 delete handle:" << ssph.get());
				auto tempItem = std::next(iter);
				DebugLog(<< " CSubscriptionMrg start 3 delete handle:" << ssph.get());
				if (iter->second.m_ssph == ssph)
				{
					DebugLog(<< " CSubscriptionMrg start 4 delete handle:" << ssph.get());
					AlarmSubscriptionMap.erase(iter);
					DebugLog(<< " CSubscriptionMrg start 5 delete handle:" << ssph.get());
				}
				iter = tempItem;
			}
		}
		
		{
			DebugLog(<< " CSubscriptionMrg --- 1 delete handle:" << ssph.get());
			CUSTORLOCKGUARD clocker(mCatalogSubMapMtx);
			DebugLog(<< " CSubscriptionMrg --- 2 delete handle:" << ssph.get());
			auto iterC = CatalogSubscriptionMap.begin();
			while (iterC != CatalogSubscriptionMap.end())
			{
				DebugLog(<< " CSubscriptionMrg --- 3 delete handle:" << ssph.get());
				auto tempItem = std::next(iterC);
				DebugLog(<< " CSubscriptionMrg --- 4 delete handle:" << ssph.get());
				if (iterC->second.m_ssph == ssph)
				{
					DebugLog(<< " CSubscriptionMrg --- 5 delete handle:" << ssph.get());
					CatalogSubscriptionMap.erase(iterC);
					DebugLog(<< " CSubscriptionMrg --- 6 delete handle:" << ssph.get());
				}
				iterC = tempItem;
			}
		}
		DebugLog(<< " CSubscriptionMrg --- end 7 delete handle:" << ssph.get());
		if (ssph.isValid())
		{
			ssph->end();
		}
		DebugLog(<< " CSubscriptionMrg --- end 8 delete handle:" << ssph.get());
	}
	catch (...)
	{

	}
	DebugLog(<< " CSubscriptionMrg end delete handle:" << ssph.get());
}
void CSubscriptionMrg::removeSubscriptionHandle(resip::ServerSubscriptionHandle ssph)
{
	DebugLog(<< " CSubscriptionMrg start remove handle:" << ssph.get());
	try
	{
		{
			CUSTORLOCKGUARD alocker(mAlarmMtx);
			DebugLog(<< " CSubscriptionMrg start 1111 remove handle:" << ssph.get());
			auto iter = AlarmSubscriptionMap.begin();
			while (iter != AlarmSubscriptionMap.end())
			{
				DebugLog(<< " CSubscriptionMrg start 2 remove handle:" << ssph.get());
				auto tempItem = std::next(iter);
				DebugLog(<< " CSubscriptionMrg start 3 remove handle:" << ssph.get());
				if (iter->second.m_ssph == ssph)
				{
					DebugLog(<< " CSubscriptionMrg start 4 remove handle:" << ssph.get());
					AlarmSubscriptionMap.erase(iter);
					DebugLog(<< " CSubscriptionMrg start 5 remove handle:" << ssph.get());
				}
				iter = tempItem;
			}
		}
		
		{
			DebugLog(<< " CSubscriptionMrg --- 1 remove handle:" << ssph.get());
			CUSTORLOCKGUARD clocker(mCatalogSubMapMtx);
			DebugLog(<< " CSubscriptionMrg --- 2 remove handle:" << ssph.get());
			auto iterC = CatalogSubscriptionMap.begin();
			while (iterC != CatalogSubscriptionMap.end())
			{
				DebugLog(<< " CSubscriptionMrg --- 3 remove handle:" << ssph.get());
				auto tempItem = std::next(iterC);
				DebugLog(<< " CSubscriptionMrg --- 4 remove handle:" << ssph.get());
				if (iterC->second.m_ssph == ssph)
				{
					DebugLog(<< " CSubscriptionMrg --- 5 remove handle:" << ssph.get());
					CatalogSubscriptionMap.erase(iterC);
					DebugLog(<< " CSubscriptionMrg --- 6 remove handle:" << ssph.get());
				}
				iterC = tempItem;
			}
		}
		DebugLog(<< " CSubscriptionMrg --- end 7 remove handle:" << ssph.get());
		// if (ssph.isValid())
		// {
		// 	ssph->end();
		// }
		DebugLog(<< " CSubscriptionMrg --- end 8 remove handle:" << ssph.get());
	}
	catch (...)
	{

	}
	DebugLog(<< " CSubscriptionMrg end remove handle:" << ssph.get());
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
bool CSubscriptionMrg::existsSubscription(resip::Data user, resip::ServerSubscriptionHandle ssph, resip::Data eventtype)
{
	try
	{
		if (eventtype == "Catalog" || eventtype == "catalagitem")
		{
			CUSTORLOCKGUARD clocker(mCatalogSubMapMtx);
			auto it = CatalogSubscriptionMap.find(user);
			if (it != CatalogSubscriptionMap.end())
			{
				if (it->second.m_ssph == ssph)
				{
					return true;
				}
			}
			/*auto iterC = CatalogSubscriptionMap.begin();
			while (iterC != CatalogSubscriptionMap.end())
			{
				auto tempItem = std::next(iterC);
				if (iterC->second.m_ssph == ssph)
				{
					CatalogSubscriptionMap.erase(iterC);
					ssph->end();
				}
				iterC = tempItem;
			}*/
		}
		else if (eventtype == "Alarm" || eventtype == "e_Alarm")
		{
			CUSTORLOCKGUARD alocker(mAlarmMtx);
			auto it = AlarmSubscriptionMap.find(user);
			if (it != AlarmSubscriptionMap.end())
			{
				if (it->second.m_ssph == ssph)
				{
					return true;
				}
			}
			
		}
		else if (eventtype == "MobilePosition")
		{

		}
		else if (eventtype == "presence")
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