//#include "stdafx.h"
#include "MyClientRegistHandler.h"
#include "../SipExtensionInfo.h"


#define RESIPROCATE_SUBSYSTEM Subsystem::REPRO

static unsigned int MaxRegistrationRetryTime = 1800;              // RFC5626 section 4.5 default
static unsigned int BaseRegistrationRetryTimeAllFlowsFailed = 30; // RFC5626 section 4.5 default
//static unsigned int BaseRegistrationRetryTime = 90;               // RFC5626 section 4.5 default

using namespace resip;
//using namespace repro;
using namespace std;
CMyClientRegistHandler::CMyClientRegistHandler(resip::DialogUsageManager& dum) :mDum(dum), authmethod(0), mShuttingdown(false)
{
	mDum.setClientRegistrationHandler(this);
}
CMyClientRegistHandler::CMyClientRegistHandler(resip::DialogUsageManager& dum, int autype) : mDum(dum), authmethod(autype), mShuttingdown(false)
{
	mDum.setClientRegistrationHandler(this);
	threadstate = 0;
	_beginthread(ThreadFun, 0, this);
}
CMyClientRegistHandler::~CMyClientRegistHandler(void)
{
	mShuttingdown = true;
	ShoutDown();
}
void CMyClientRegistHandler::onSuccess(resip::ClientRegistrationHandle h, const resip::SipMessage& response)
{
	InfoLog(<< "onSuccess(ClientRegistrationHandle): msg=" << response.brief());
	if (mShuttingdown)
	{
		h->end();
		return;
	}
	//if (mRegHandle.getId() == 0)  // Note: reg handle id will only be 0 on first successful registration
	//{
	//	// Check if we should try to form a test subscription
	//	if (!mSubscribeTarget.host().empty())
	//	{
	//		auto sub = mDum->makeSubscription(NameAddr(mSubscribeTarget), mProfile, "basicClientTest");
	//		mDum->send(std::move(sub));
	//	}

	//	// Check if we should try to form a test call
	//	if (!mCallTarget.host().empty())
	//	{
	//		BasicClientCall* newCall = new BasicClientCall(*this);
	//		newCall->initiateCall(mCallTarget, mProfile);
	//	}
	//}
	//mRegHandle = h;
	//mRegistrationRetryDelayTime = 0;  // reset
}
void CMyClientRegistHandler::onFailure(resip::ClientRegistrationHandle h, const resip::SipMessage& response)
{
	InfoLog(<< "onFailure(ClientRegistrationHandle): msg=" << response.brief());
	//mRegHandle = h;
	if (mShuttingdown)
	{
		h->end();
	}
}
void CMyClientRegistHandler::onRemoved(resip::ClientRegistrationHandle h, const resip::SipMessage& response)
{
	InfoLog(<< "onRemoved(ClientRegistrationHandle): msg=" << response.brief());
	//mRegHandle = h;
}
int  CMyClientRegistHandler::onRequestRetry(resip::ClientRegistrationHandle h, int retryMinimum, const resip::SipMessage& msg)
{
	//mRegHandle = h;
	if (mShuttingdown)
	{
		return -1;
	}

	if (mRegistrationRetryDelayTime == 0)
	{
		mRegistrationRetryDelayTime = BaseRegistrationRetryTimeAllFlowsFailed; // We only have one flow in this test app
	}

	// Use back off procedures of RFC 5626 section 4.5
	mRegistrationRetryDelayTime = resipMin(MaxRegistrationRetryTime, mRegistrationRetryDelayTime * 2);

	// return an evenly distributed random number between 50% and 100% of mRegistrationRetryDelayTime
	int retryTime = Helper::jitterValue(mRegistrationRetryDelayTime, 50, 100);
	InfoLog(<< "onRequestRetry(ClientRegistrationHandle): msg=" << msg.brief() << ", retryTime=" << retryTime);

	return retryTime;
}
unsigned int CMyClientRegistHandler::Regist(/*GB28181SIPServConf upcmsinfo,RegistParam reg_param*/)
{
	/*unsigned int index = 0;
	resip::WriteLock w(mUpCmsMapMutex);
	MyUpCmsState *pcmsstate =  GetCmsByCmsID(upcmsinfo.m_CMSID,index);
	if (pcmsstate !=NULL)
	{
		if (upcmsinfo.m_CMSIP != pcmsstate->CmsInfo.m_CMSIP || upcmsinfo.m_cmsport != pcmsstate->CmsInfo.m_cmsport || upcmsinfo.m_password != pcmsstate->CmsInfo.m_password)
		{
			pcmsstate->CmsInfo = upcmsinfo;
		}
		pcmsstate->RegParam = reg_param;
		if(pcmsstate->i_Progress == 0 || pcmsstate->i_Progress == 1 || pcmsstate->i_State == 200)
		{
			return index;
		}
		mDum->getMasterProfile()->setDigestCredential(upcmsinfo.m_Domain, upcmsinfo.m_MyId, upcmsinfo.m_password);
		mDum->getMasterProfile()->setDigestCredential(upcmsinfo.m_CMSIP, upcmsinfo.m_MyId, upcmsinfo.m_password);
		
		try
		{
			if (pcmsstate->mh.isValid())
			{
				pcmsstate->mh->stopRegistering();
			}
		}
		catch(...)
		{
			
		}
		pcmsstate->m_DialogSet = new CMyAppDialogSet(*mDum,Data(index));
		pcmsstate->i_State =0;
		pcmsstate->i_Progress = 0;
		sendRegisterMessage(*pcmsstate);
	}
	else
	{
		mDum->getMasterProfile()->setDigestCredential(upcmsinfo.m_Domain, upcmsinfo.m_MyId, upcmsinfo.m_password);
		mDum->getMasterProfile()->setDigestCredential(upcmsinfo.m_CMSIP, upcmsinfo.m_MyId, upcmsinfo.m_password);
		MyUpCmsState TemDiogSet(upcmsinfo,reg_param);
		index = GetCmsIdleIndex();

		TemDiogSet.m_DialogSet = new CMyAppDialogSet(*mDum,Data(index));
		MyUpCmsInfoMap[index] =TemDiogSet;
		sendRegisterMessage(TemDiogSet);
	}*/
	return 0;//index;
}

int  CMyClientRegistHandler::RegistState(Data Cmsid)
{
	/*resip::ReadLock r(mUpCmsMapMutex);
	map<unsigned int ,MyUpCmsState>::iterator iter= MyUpCmsInfoMap.begin();
	for (;iter != MyUpCmsInfoMap.end();iter++)
	{
		if (iter->second.CmsInfo.m_CMSID == Cmsid)
		{
			return iter->second.i_State;
		}
	}*/
	return 0;
}

void CMyClientRegistHandler::RegistClose(Data CmsId)
{
	/*resip::WriteLock w(mUpCmsMapMutex);
	map<unsigned int ,MyUpCmsState>::iterator iter= MyUpCmsInfoMap.begin();
	for (;iter != MyUpCmsInfoMap.end();iter++)
	{
		if (iter->second.CmsInfo.m_CMSID == CmsId)
		{
			if (iter->second.mh.isValid())
			{
				iter->second.mh->removeMyBindingsCommand(true);
			}
			MyUpCmsInfoMap.erase(iter->first);
			break;
		}
	}*/
}


void CMyClientRegistHandler::ShoutDown()
{
	/*resip::WriteLock w(mUpCmsMapMutex);
	map<unsigned int ,MyUpCmsState>::iterator iter= MyUpCmsInfoMap.begin();
	for (;iter != MyUpCmsInfoMap.end();iter++)
	{
		if (iter->second.mh.isValid())
		{
			iter->second.mh->end();
		}
		else
		{
			if (iter->second.m_DialogSet)
			{
				iter->second.m_DialogSet->end();
			}
		}
		
	}
	MyUpCmsInfoMap.clear();
	LogOut("GB", L_DEBUG, " client regist shoutdown thread statue:%d", threadstate);
	if (threadstate !=0)
	{
		threadstate =2;
		while (threadstate!=0)
		{
			Sleep(50);
		}
	}*/
}


void CMyClientRegistHandler::SendHeart()
{
	if(authmethod > 0)
	{
		return;
	}
	//resip::WriteLock w(mUpCmsMapMutex);
	//map<unsigned int ,MyUpCmsState>::iterator iter =  MyUpCmsInfoMap.begin();
	//for (;iter != MyUpCmsInfoMap.end();iter ++)
	//{
	//	if(iter->second.i_State !=200)
	//	{
	//		continue;
	//	}
	//	if ((iter->second.lastSendHearttime > iter->second.lastsendheartoktime && iter->second.lastSendHearttime - iter->second.lastsendheartoktime > 3 * 60)
	//		|| iter->second.sendhearterrorcount >= iter->second.RegParam.hearttimeoutcount)
	//	{
	//		iter->second.i_State = HEARTERROR;//连续3次心跳消息未收到响应或连续3次心跳超时
	//		continue;
	//	}
	//	time_t curtime= time(NULL);
	//	if(curtime - iter->second.lastSendHearttime > iter->second.RegParam.heartsendcycle)
	//	{
	//		Uri toaor;
	//		toaor.host() =iter->second.CmsInfo.m_CMSIP ;
	//		toaor.port()=iter->second.CmsInfo.m_cmsport.convertInt();
	//		toaor.user() = iter->second.CmsInfo.m_CMSID;
	//		Uri faor;
	//		faor.host() = CTools::GetlocolIpv4Ip();
	//		faor.port()=iter->second.CmsInfo.locoalport;
	//		faor.user() = iter->second.CmsInfo.m_MyId;
	//		if (G_SipMrg())
	//		{
	//			ERROR_GJ errorcode;
	//			Data chontent;
	//			{
	//				DataStream ds(chontent);
	//				ds<<"<?xml version=\"1.0\" ?>\n"
	//				<<"<Notify>\n"
 //					<<"<CmdType>" << "Keepalive" <<"</CmdType>\n"
 //					<<"<SN>" << G_SipMrg()->CreateMessageSN() <<"</SN>\n"
	//				<<"<DeviceID>"<< iter->second.CmsInfo.m_MyId<<"</DeviceID>\n"
	//				<<"<Status>OK</Status>\n"
	//				<<"</Notify>\n";
	//				ds.flush();
	//			}
	//			LogOut("GB", L_DEBUG, "*********************send keepalive******************");
	//			unsigned long  handle  = G_SipMrg()->SipSendMessage(faor, toaor,chontent.c_str(),chontent.size(), KEEP_ALIVE, errorcode,0);
	//			iter->second.lastSendHearttime= curtime;
	//		}
	//	}
	//}
}
void CMyClientRegistHandler::ThreadFun(void *p)
{
	CMyClientRegistHandler *mrh = (CMyClientRegistHandler *)p;
	mrh->threadstate =1;
	int count = 0;
	while(mrh->threadstate==1)
	{
		if (count++ > 10)
		{
			mrh->SendHeart();
			mrh->RegistCms();
			count=0;
		}
		//usleep(1000 * 1000);
	}
	mrh->threadstate =0;
}

void CMyClientRegistHandler::RegistCms()
{
	//if(authmethod == 0)
	//{
	//	resip::WriteLock w(mUpCmsMapMutex);
	//	map<unsigned int, MyUpCmsState>::iterator iter = MyUpCmsInfoMap.begin();
	//	for (; iter != MyUpCmsInfoMap.end(); iter++)
	//	{
	//		if (iter->second.sendhearterrorcount >= iter->second.RegParam.hearttimeoutcount 
	//			|| time(NULL) - iter->second.lastSendHearttime > iter->second.RegParam.heartsendcycle * iter->second.RegParam.hearttimeoutcount)
	//		{
	//			if(iter->second.i_Progress == 0 || iter->second.i_Progress == 1)//retry
	//			{
	//				continue;
	//			}
	//			if (iter->second.mh.isValid())
	//			{
	//				iter->second.mh->stopRegistering();
	//			}
	//			iter->second.i_State = 0;
	//			iter->second.i_Progress = 0;
	//			mDum->getMasterProfile()->setDigestCredential(iter->second.CmsInfo.m_CMSIP, iter->second.CmsInfo.m_MyId, iter->second.CmsInfo.m_password);
	//			if(iter->second.CmsInfo.m_CMSID.size() > 8)
	//				mDum->getMasterProfile()->setDigestCredential(iter->second.CmsInfo.m_CMSID.substr(0, 8), iter->second.CmsInfo.m_MyId, iter->second.CmsInfo.m_password);
	//			
	//			
	//			iter->second.m_DialogSet = new CMyAppDialogSet(*mDum, Data(iter->first));
	//			
	//			sendRegisterMessage(iter->second);
	//			
	//			LogOut("GB", L_INFO, "timeout Regist index:%d, new dialog:%p,upcmsid:%s", iter->first, iter->second.m_DialogSet,iter->second.CmsInfo.m_CMSID.c_str());
	//			iter->second.sendhearterrorcount = 0;
	//			iter->second.lastSendHearttime = time(NULL);
	//			
	//		}
	//	}
	//}
	//else
	//{
	//	resip::WriteLock w(mUpCmsMapMutex);
	//	map<unsigned int, MyUpCmsState>::iterator iter = MyUpCmsInfoMap.begin();
	//	for (; iter != MyUpCmsInfoMap.end(); iter++)
	//	{
	//		if (iter->second.i_State != 200
	//			&& time(NULL) - iter->second.lastSendHearttime > 60)
	//		{
	//			if(iter->second.i_State == 0 || iter->second.i_Progress == 1)//retry
	//			{
	//				continue;
	//			}
	//			if (iter->second.mh.isValid())
	//			{
	//				iter->second.mh->stopRegistering();
	//			}
	//			iter->second.i_State = 0;
	//			iter->second.i_Progress = 0;
	//			mDum->getMasterProfile()->setDigestCredential(iter->second.CmsInfo.m_CMSIP, iter->second.CmsInfo.m_MyId, iter->second.CmsInfo.m_password);
	//			if(iter->second.CmsInfo.m_CMSID.size() > 8)
	//				mDum->getMasterProfile()->setDigestCredential(iter->second.CmsInfo.m_CMSID.substr(0, 8), iter->second.CmsInfo.m_MyId, iter->second.CmsInfo.m_password);
	//			
	//			
	//			iter->second.m_DialogSet = new CMyAppDialogSet(*mDum, Data(iter->first));
	//			
	//			sendRegisterMessage(iter->second);
	//			
	//			LogOut("GB", L_INFO, "timeout Regist index:%d, new dialog:%p,upcmsid:%s", iter->first, iter->second.m_DialogSet,iter->second.CmsInfo.m_CMSID.c_str());
	//			iter->second.sendhearterrorcount = 0;
	//			iter->second.lastSendHearttime = time(NULL);
	//			
	//		}
	//	}
	//}
}

void CMyClientRegistHandler::OnSendHeartResultState(Data CmsId, int errorcode)
{
	unsigned int  index = 0;
	/*resip::WriteLock w(mUpCmsMapMutex);
	MyUpCmsState *pcmsstate = GetCmsByCmsID(CmsId, index);
	if (pcmsstate)
	{
		if (errorcode == 200)
		{
			pcmsstate->sendhearterrorcount = 0;
			pcmsstate->lastsendheartoktime = time(NULL);
		}
		else
		{
			pcmsstate->sendhearterrorcount++;
		}
	}*/
}

