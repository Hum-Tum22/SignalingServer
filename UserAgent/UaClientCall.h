#if !defined(basicClientCall_hxx)
#define basicClientCall_hxx

#include <resip/dum/AppDialogSet.hxx>

#include <resip/dum/SubscriptionHandler.hxx>
#include <resip/dum/RedirectHandler.hxx>
#include <resip/dum/InviteSessionHandler.hxx>
#include <resip/dum/DialogSetHandler.hxx>
#include <resip/dum/DialogUsageManager.hxx>

#include "../asio/include/asio.hpp"

#include "UserAgentMgr.h"
#include "../tools/CTask.h"
#include "http.h"

#include "../media/rtp/receiver_rtp.h"
#include "../media/ps-file-source.h"



namespace resip
{

class UaClientCall : public AppDialogSet 
{
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

   void createRtpServer(short port, const char* peerIp, short peerPort);
   rtp_receiver* rtp_ctx;

   PSFileSource* psSource;
private:
	asio::io_service io_service;
private:       
   UaMgr &mUserAgent;
   resip::InviteSessionHandle mInviteSessionHandle;
   unsigned int mTimerExpiredCounter;
   bool mPlacedCall;
   resip::InviteSessionHandle mInviteSessionHandleReplaced;
   int mSessionState;

   // UAC forked call handling helper members
   bool isUACConnected();
   bool isStaleFork(const resip::DialogId& dialogId);
   resip::DialogId mUACConnectedDialogId;

   void makeOffer(SdpContents& offer);
   void makeMyOffer(SdpContents& offer, std::string myId);
   void closeMediaStream();
public:
	bool makeBLeg();
	//收到一个invite off请求处理
	void ReceiveInviteOffRequest(resip::InviteSessionHandle handle, const resip::SipMessage& msg, const resip::SdpContents& offer);
	resip::InviteSessionHandle& getInviteSessionHandle();

public:

	static void __stdcall VskX86NvrRtPreDataCb(uint32_t handle, const uint8_t* data, size_t dataSize, void* pUser);

public:
	std::condition_variable m_CallEvt;
	//sdp info
	SdpContents mResponseSdp;
	SdpContents mRequestSdp;
	Data sessionName;
	Data remoteIp;
	unsigned short remotePort;
	unsigned long startTime;
	unsigned long stopTime;
	int rtpType;			//0:udp,1:tcp active,2:tcp pass
	Data ssrc;

	//dev info
	std::string devId;
	std::string devIp;
	int devPort;
	std::string myId;
	std::string remoteId;
	//stream info
	std::string streamId;
	std::string channelId;
	Data app;
	//local info
	unsigned short myRtpPort;
	std::string mySdpIp;

	typedef enum _CallState
	{
		CALL_UAC_RES_START,
		CALL_UAC_NEW_GET1XX,			
		CALL_UAC_GET1XX_PROV,			//onProvisional
		CALL_UAC_ANSWER_200OK,
		CALL_UAC_CONNECTED,
		CALL_UAC_FAILURE,
		CALL_UAC_TERMINATED,

		CALL_UAS_NEW,
		CALL_UAS_RECEIVE_OFFER,
		CALL_UAS_CONNECTED,
		CALL_UAS_CONNECTED_CONFIRMED,
		CALL_UAS_REJECT,
		CALL_UAS_TERMINATED,

		CALL_MY_MEDIA_OK,
		CALL_MEDIA_WAIT,
		CALL_MEDIA_READY,
		CALL_MEDIA_ERROR,
		CALL_MEDIA_TIMEOUT,
		CALL_MEDIA_STREAM_CLOSED,
		CALL_NOT_FOUND,
	}CALL_STATE;

	std::condition_variable m_CallTask;
};
class RequestStreamTask: public ownTask::CTask
{
	std::string devId;
	std::string devIp;
	int devPort;
	std::string channelId;
	std::string streamId;
	UaMgr& mUserAgent;
	int rtpPort;
	int rtpType;
	UaClientCall *pmAlegCall;

	std::string sessionName;
	unsigned long startTime;
	unsigned long stopTime;
public:
	RequestStreamTask(std::string dId, std::string dIp, int dPort, std::string channelId, UaMgr& userAgent, int iRtpPort, int rtptype, UaClientCall* pAlegcall = NULL,
		std::string sesName = "Play", unsigned long stime = 0, unsigned long etime = 0);
	bool TaskRun();
	bool TaskClose() { return false; };
};
class PushRtpStream :public ownTask::CTask
{
	std::string devId;
	std::string devIp;
	int devPort;
	std::string channelId;
	std::string strSsrc;
	std::string stream_Id;
	std::string strDstIp;
	int connectPort;
	int localPort;
	//UaClientCall* mAlegCall;
public:
	PushRtpStream(std::string dId, std::string dIp, int dPort, std::string channelId, std::string ssrc, int cport, int lPort)
		:devId(dId), channelId(channelId), strSsrc(ssrc), connectPort(cport), localPort(lPort)
	{
		//stream_Id = std::str_format("%s_%s", devId.c_str(),channelId.c_str());
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
