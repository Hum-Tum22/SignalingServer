#if !defined(basicClientCall_hxx)
#define basicClientCall_hxx

#include <resip/dum/AppDialogSet.hxx>

#include <resip/dum/SubscriptionHandler.hxx>
#include <resip/dum/RedirectHandler.hxx>
#include <resip/dum/InviteSessionHandler.hxx>
#include <resip/dum/DialogSetHandler.hxx>
#include <resip/dum/DialogUsageManager.hxx>

#include "UserAgentMgr.h"
#include "tools/CTask.h"
#include "http.h"

namespace resip
{

class UaClientCall : public AppDialogSet 
{
public:
    Data ssrc;
    ServerInviteSessionHandle mh;
    Data connectip;
    Data connectport;
    Data app;
	SdpContents AlegResSdp;
	//int tcpOrUdp;
private:
public:
    UaClientCall(UaMgr& userAgent);
   virtual ~UaClientCall();
   
   virtual void initiateCall(const Uri& target, shared_ptr<UserProfile> profile);
   virtual void terminateCall();
   virtual void timerExpired();

protected:
   friend class UaMgr;

   // This API must return an appropriate user profile after inspecting the incoming INVITE
   virtual shared_ptr<UserProfile> selectUASUserProfile(const SipMessage&);

   // Invite Session Handler /////////////////////////////////////////////////////
   virtual void onNewSession(resip::ClientInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
   virtual void onNewSession(resip::ServerInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
   virtual void onFailure(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg);
   virtual void onEarlyMedia(resip::ClientInviteSessionHandle, const resip::SipMessage&, const resip::SdpContents&);
   virtual void onProvisional(resip::ClientInviteSessionHandle, const resip::SipMessage& msg);
   virtual void onConnected(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg);
   virtual void onConnected(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onConnectedConfirmed(InviteSessionHandle, const SipMessage& msg);
   virtual void onStaleCallTimeout(resip::ClientInviteSessionHandle);
   virtual void onTerminated(resip::InviteSessionHandle h, resip::InviteSessionHandler::TerminatedReason reason, const resip::SipMessage* msg);
   virtual void onRedirected(resip::ClientInviteSessionHandle, const resip::SipMessage& msg);
   virtual void onAnswer(resip::InviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents&);
   virtual void onOffer(resip::InviteSessionHandle handle, const resip::SipMessage& msg, const resip::SdpContents& offer);
   virtual void onOfferRequired(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onOfferRejected(resip::InviteSessionHandle, const resip::SipMessage* msg);
   virtual void onOfferRequestRejected(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onRemoteSdpChanged(resip::InviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents& sdp);
   virtual void onInfo(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onInfoSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onInfoFailure(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onRefer(resip::InviteSessionHandle, resip::ServerSubscriptionHandle, const resip::SipMessage& msg);
   virtual void onReferAccepted(resip::InviteSessionHandle, resip::ClientSubscriptionHandle, const resip::SipMessage& msg);
   virtual void onAckReceived(InviteSessionHandle, const SipMessage& msg);
   virtual void onReferRejected(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onReferNoSub(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onMessage(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onMessageSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onMessageFailure(resip::InviteSessionHandle, const resip::SipMessage& msg);
   virtual void onForkDestroyed(resip::ClientInviteSessionHandle);
   virtual void onReadyToSend(InviteSessionHandle, SipMessage& msg);
   virtual void onFlowTerminated(InviteSessionHandle);

   // DialogSetHandler  //////////////////////////////////////////////
   virtual void onTrying(resip::AppDialogSetHandle, const resip::SipMessage& msg);
   virtual void onNonDialogCreatingProvisional(resip::AppDialogSetHandle, const resip::SipMessage& msg);

   // ClientSubscriptionHandler ///////////////////////////////////////////////////
   virtual void onUpdatePending(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder);
   virtual void onUpdateActive(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder);
   virtual void onUpdateExtension(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder);
   virtual void onNotifyNotReceived(resip::ClientSubscriptionHandle h);
   virtual void onTerminated(resip::ClientSubscriptionHandle h, const resip::SipMessage* notify);
   virtual void onNewSubscription(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify);
   virtual int  onRequestRetry(resip::ClientSubscriptionHandle h, int retrySeconds, const resip::SipMessage& notify);

   // RedirectHandler /////////////////////////////////////////////////////////////
   virtual void onRedirectReceived(AppDialogSetHandle h, const SipMessage& msg);

private:       
   UaMgr &mUserAgent;
   resip::InviteSessionHandle mInviteSessionHandle;
   unsigned int mTimerExpiredCounter;
   bool mPlacedCall;
   resip::InviteSessionHandle mInviteSessionHandleReplaced;

   // UAC forked call handling helper members
   bool isUACConnected();
   bool isStaleFork(const resip::DialogId& dialogId);
   resip::DialogId mUACConnectedDialogId;

   void makeOffer(SdpContents& offer);
   void makeRequestOffer(SdpContents& offer, std::string myId);
public:
	bool makeBLeg(std::string channelId);
public:
	class UacInviteVideoInfo
	{
	public:
		typedef enum _requeststate
		{
			_RES_START,
			_RES_GET1XX,
			_RES_GETSDP,
			_RES_CONNECT,
			_RES_ACK,
			_RES_FAILED
		};
		SdpContents m_DstSdp;//目标返回的 sdp
		//unsigned long m_resid;
		int rtpType;	// 0:udp,1:tcp active, 2:tcp pass
		std::string sdpIp;
		std::string ssrc;
		std::string sessionName;
		InviteSessionHandle mInviteSessionHandle;
		int state;
		time_t startime;
		std::string devId;
		std::string devIp;
		int devPort;
		std::string streamId;
		int rtpPort;
		SdpContents m_sendsdp;//记录自己发送的 主要获取发送时候的media 信息对应的端口
		UacInviteVideoInfo(/*unsigned long tresid*/) :state(_RES_START), devPort(0)//, m_resid(tresid)
		{
			time(&startime);
		}
		~UacInviteVideoInfo()
		{
			//if (m_RtpClint)
			{
				//delete m_RtpClint;
			}
		}
	};
	class UasInviteRtimeVideoInfo
	{
	public:
		ServerInviteSessionHandle mInviteSessionHandle;
		unsigned long RtpServrHandle;
		int HaveProvide;//0,1:get sdp,2,no get sdp should  offer sdp
		SdpContents ClentSdp;//
		SdpContents SerSdp;
		time_t hearttime;
		int localtport;
		std::string channelID;
		std::string devId;
		std::string serverID;
		std::string DataTransMoudle;
		UasInviteRtimeVideoInfo() :RtpServrHandle(0), HaveProvide(0), localtport(0)
		{
			time(&hearttime);
		}
	};
	UacInviteVideoInfo mMyUacInviteVideoInfo;
	UasInviteRtimeVideoInfo mMyUasInviteVideoInfo;

	std::condition_variable m_CallTask;
};
class RequestStreamTask: public ownTask::CTask
{
	std::string devId;
	std::string devIp;
	int devPort;
	std::string streamId;
	UaMgr& mUserAgent;
	int rtpPort;
	int rtpType;
public:
	RequestStreamTask(std::string dId, std::string dIp, int dPort, std::string channelId, UaMgr& userAgent, int iRtpPort)
		:devId(dId),devIp(dIp), devPort(dPort), streamId(channelId), mUserAgent(userAgent), rtpPort(iRtpPort), rtpType(0){}
	bool TaskRun();
	bool TaskClose() { return false; };
};
class PushRtpStream :public ownTask::CTask
{
	std::string devId;
	std::string channelId;
	std::string strSsrc;
	std::string stream_Id;
	std::string strDstIp;
	int connectPort;
	int localPort;
	//UaClientCall* mAlegCall;
public:
	PushRtpStream(std::string dId, std::string channelId, std::string ssrc, int cport, int lPort)
		:devId(dId), channelId(channelId), strSsrc(ssrc), connectPort(cport), localPort(lPort)
	{
		stream_Id = devId;
		stream_Id += "_";
		stream_Id += channelId;
	}
	bool TaskRun();
	bool TaskClose() { return false; };
};
}
#endif

/* ====================================================================

 Copyright (c) 2011, SIP Spectrum, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are 
 met:

 1. Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer. 

 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution. 

 3. Neither the name of SIP Spectrum nor the names of its contributors 
    may be used to endorse or promote products derived from this 
    software without specific prior written permission. 

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ==================================================================== */
