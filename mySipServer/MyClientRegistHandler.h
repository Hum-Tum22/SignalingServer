#ifndef _MYCLIENTREGISTHANDLER_H_
#define _MYCLIENTREGISTHANDLER_H_
#include "resip/dum/ClientRegistration.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/RegistrationHandler.hxx"
#include "rutil/Data.hxx"
#include "rutil/RWMutex.hxx"

class CMyUac
{
	resip::ClientRegistrationHandle mRegHandle;
	resip::Uri mTargetUri;
	unsigned int mRegistrationRetryDelayTime;
};
class CMyClientRegistHandler:public resip::ClientRegistrationHandler
{
protected:
	resip::DialogUsageManager& mDum;
	bool mShuttingdown;
	int authmethod;
	unsigned int mRegistrationRetryDelayTime;

	void  RegistCms();
public:
	CMyClientRegistHandler(resip::DialogUsageManager& dum);
	CMyClientRegistHandler(resip::DialogUsageManager& dum, int autype = 0);
	~CMyClientRegistHandler(void);

	using ClientRegistrationHandler::onFlowTerminated;
	virtual void onSuccess(resip::ClientRegistrationHandle h, const resip::SipMessage& response);
	virtual void onFailure(resip::ClientRegistrationHandle h, const resip::SipMessage& response);
	virtual void onRemoved(resip::ClientRegistrationHandle h, const resip::SipMessage& response);
	virtual int  onRequestRetry(resip::ClientRegistrationHandle h, int retryMinimum, const resip::SipMessage& msg);

	unsigned int Regist(/*GB28181SIPServConf upcmsinfo,RegistParam reg_param*/);
	int  RegistState(resip::Data Cmsid);

	void RegistClose(resip::Data CmsId);
	void ShoutDown();

	void OnSendHeartResultState(resip::Data CmsId, int errorcode);
	//
	void SendHeart();
	int threadstate ;
	static void ThreadFun(void *p);
	void Heart();
};
#endif
