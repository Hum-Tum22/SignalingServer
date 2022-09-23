#ifndef MY_B2BSVR_MGR_H
#define MY_B2BSVR_MGR_H

namespace b2bsvr
{
	class CB2BSvr;
}
#include "UserAgentMgr.h"
#include "UaMessageMgr.h"
using namespace resip;

namespace b2bsvr
{

	struct  clientpagehandlerparam
	{
		time_t lastusertime;
		ClientPagerMessageHandle cpmh;
	};
	struct TempMessageResend
	{
		resip::SipMessage msg_h;
		Data MessageBody;
	};

	struct SubscribeCtr
	{
		NameAddr touser;
		Data     eventtype;
		Data     pContent;
		unsigned int     ui_time;
		//ClientSubscriptionHandle m_chandle;
		//CClientAppDialogSet *m_AppDlgSet;
	};

	class ReSendMessageTask;

	/*class MyB2BSvrMgr
	{
	public:
		MyB2BSvrMgr();
		~MyB2BSvrMgr();
	private:
		UaMgr* m_UaMgr;
	};*/
}
#endif
