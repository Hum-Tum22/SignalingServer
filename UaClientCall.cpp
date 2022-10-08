#include "UaClientCall.h"

#include <resip/stack/SdpContents.hxx>
#include <resip/stack/PlainContents.hxx>
#include <resip/stack/SipMessage.hxx>
#include <resip/stack/ShutdownMessage.hxx>
#include <resip/stack/SipStack.hxx>
#include <resip/dum/ClientAuthManager.hxx>
#include <resip/dum/ClientInviteSession.hxx>
#include <resip/dum/ClientSubscription.hxx>
#include <resip/dum/DialogUsageManager.hxx>
#include <resip/dum/InviteSessionHandler.hxx>
#include <resip/dum/MasterProfile.hxx>
#include <resip/dum/ServerInviteSession.hxx>
#include <resip/dum/AppDialog.hxx>
#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/AppDialogSetFactory.hxx>
#include <rutil/Log.hxx>
#include <rutil/Logger.hxx>
#include <rutil/Random.hxx>
#include <rutil/WinLeakCheck.hxx>

#include <sstream>
#include <time.h>
#include "http.h"
#include "device/DeviceManager.h"
#include "tools/ownString.h"
#include "tools/iThreadPool.h"

#include "myjsondef.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"
#include "SipServer.h"

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

using namespace resip;
using namespace std;

static unsigned int CallTimerTime = 30;  // Time between call timer
static unsigned int CallTimeCounterToByeOn = 6;  // BYE the call after the call timer has expired 6 times

namespace resip
{
class CallTimer : public resip::DumCommand
{
   public:
      CallTimer(UaMgr& userAgent, UaClientCall* call) : mUserAgent(userAgent), mCall(call) {}
      CallTimer(const CallTimer& rhs) : mUserAgent(rhs.mUserAgent), mCall(rhs.mCall) {}
      ~CallTimer() {}

      void executeCommand() { mUserAgent.onCallTimeout(mCall); }

      resip::Message* clone() const { return new CallTimer(*this); }
      EncodeStream& encode(EncodeStream& strm) const { strm << "CallTimer:"; return strm; }
      EncodeStream& encodeBrief(EncodeStream& strm) const { return encode(strm); }

   private:
       UaMgr& mUserAgent;
      UaClientCall* mCall;
};
}

UaClientCall::UaClientCall(UaMgr& userAgent)
: AppDialogSet(userAgent.getDialogUsageManager()),
  mUserAgent(userAgent),
  mTimerExpiredCounter(0),
  mPlacedCall(false),
  mUACConnectedDialogId(Data::Empty, Data::Empty, Data::Empty)
{
   mUserAgent.registerCall(this);
}

UaClientCall::~UaClientCall()
{
    mUserAgent.unregisterCall(this);
    if (mMyUasInviteVideoInfo.localtport > 0)
    {
        mUserAgent.FreeRptPort(mMyUasInviteVideoInfo.localtport);
        sipserver::SipServer* pSvr = GetServer();
        ostringstream ss;
        ss << "http://" << (pSvr?pSvr->zlmHost:"127.0.0.1") << ":" << (pSvr?pSvr->zlmHttpPort:8080) << "/index/api/stopSendRtp?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&vhost=__defaultVhost__&app=rtp&stream="
            << mMyUasInviteVideoInfo.devId << "_" << mMyUasInviteVideoInfo.channelID;
        string strReponse = GetRequest(ss.str());
    }
    if (mMyUacInviteVideoInfo.rtpPort > 0)
    {
        if (mInviteSessionHandle.isValid())
        {
            mInviteSessionHandle->end();
        }
        mUserAgent.FreeRptPort(mMyUacInviteVideoInfo.rtpPort);
        mUserAgent.CloseStreamStreamId(mMyUacInviteVideoInfo.streamId);
    }
}

void 
UaClientCall::initiateCall(const Uri& target, shared_ptr<UserProfile> profile)
{
   SdpContents offer;
   makeOffer(offer);
   shared_ptr<SipMessage> invite = mUserAgent.getDialogUsageManager().makeInviteSession(NameAddr(target), profile, &offer, this);
   mUserAgent.getDialogUsageManager().send(invite);
   mPlacedCall = true;
}

void 
UaClientCall::terminateCall()
{
   AppDialogSet::end(); 
}

void 
UaClientCall::timerExpired()
{
   mTimerExpiredCounter++;
   if(mTimerExpiredCounter < CallTimeCounterToByeOn)
   {
      // First few times, send a message to the other party
      if(mInviteSessionHandle.isValid())
      {
         PlainContents plain("test message");
         mInviteSessionHandle->message(plain);
      }
   }
   else 
   {
      // Then hangup
      terminateCall();
   }

   // start timer for next one
   unique_ptr<ApplicationMessage> timer(new CallTimer(mUserAgent, this));
   mUserAgent.mStack.post(std::move(timer), CallTimerTime, &mUserAgent.getDialogUsageManager());
}

shared_ptr<UserProfile>
UaClientCall::selectUASUserProfile(const SipMessage& msg)
{
   return mUserAgent.getIncomingUserProfile(msg);
}

bool 
UaClientCall::isUACConnected()
{
   return !mUACConnectedDialogId.getCallId().empty();
}

bool 
UaClientCall::isStaleFork(const DialogId& dialogId)
{
   return (!mUACConnectedDialogId.getCallId().empty() && dialogId != mUACConnectedDialogId);
}

void 
UaClientCall::makeOffer(SdpContents& offer)
{
   static Data txt("v=0\r\n"
                   "o=- 0 0 IN IP4 0.0.0.0\r\n"
                   "s=basicClient\r\n"
                   "c=IN IP4 0.0.0.0\r\n"  
                   "t=0 0\r\n"
                   "m=audio 8000 RTP/AVP 0 101\r\n"
                   "a=rtpmap:0 pcmu/8000\r\n"
                   "a=rtpmap:101 telephone-event/8000\r\n"
                   "a=fmtp:101 0-15\r\n");

   static HeaderFieldValue hfv(txt.data(), txt.size());
   static Mime type("application", "sdp");
   static SdpContents offerSdp(hfv, type);

   offer = offerSdp;

   // Set sessionid and version for this offer
   uint64_t currentTime = Timer::getTimeMicroSec();
   offer.session().origin().getSessionId() = currentTime;
   offer.session().origin().getVersion() = currentTime;  
}
bool UaClientCall::makeBLeg(std::string channelId)
{
    //判断通道在存
    IDeviceMngrSvr& DevMgr = GetIDeviceMngr();
    SipServerDeviceInfo devuinfo;
    list<GBDeviceChannel> chlist = DevMgr.GetGBDeviceMapper().GetGBDeviceChannelMapper().queryChannelByChannelId(channelId.c_str());
    for (auto& it : chlist)
    {
        if (it.getStatus())
        {
            devuinfo = DevMgr.GetGBDeviceMapper().getDeviceByDeviceId(it.getDeviceId());
            break;
        }
    }
    if (!devuinfo.getDeviceId().empty())
    {
        mMyUasInviteVideoInfo.devId = devuinfo.getDeviceId();
        //判断流存在
        ownThreadPool::myThreadPool& tPool = ownThreadPool::GetThreadPool();
        tPool.submitTask(std::make_shared<RequestStreamTask>(devuinfo.getDeviceId(), devuinfo.getIp(), devuinfo.getPort(), channelId, mUserAgent, this, mUserAgent.GetAvailableRtpPort()));
        return true;
    }
    return false;
}
////////////////////////////////////////////////////////////////////////////////
// InviteSessionHandler      ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void
UaClientCall::onNewSession(ClientInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
   InfoLog(<< "onNewSession(ClientInviteSessionHandle): msg=" << msg.brief());
   mInviteSessionHandle = h->getSessionHandle();  // Note:  each forked leg will update mInviteSession - need to set mInviteSessionHandle for final answering leg on 200
   if(mInviteSessionHandleReplaced.isValid())
   {
       // See comment in flowTerminated for an explanation of this logic
       ((UaClientCall*)mInviteSessionHandleReplaced->getAppDialogSet().get())->terminateCall();
   }
   mMyUacInviteVideoInfo.state = UacInviteVideoInfo::_RES_GET1XX;
   mMyUacInviteVideoInfo.mInviteSessionHandle = h->getSessionHandle();
   cout << "***************  ******************* 1 " << msg << endl;
}

void
UaClientCall::onNewSession(ServerInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
   InfoLog(<< "onNewSession(ServerInviteSessionHandle):  msg=" << msg.brief());
   mInviteSessionHandle = h->getSessionHandle();
   //UaClientCall* p = this;
   // First check if this INVITE is to replace an existing session
   if(msg.exists(h_Replaces))
   {
      pair<InviteSessionHandle, int> presult;
      presult = mDum.findInviteSession(msg.header(h_Replaces));
      if(!(presult.first == InviteSessionHandle::NotValid())) 
      {         
         UaClientCall* callToReplace = dynamic_cast<UaClientCall *>(presult.first->getAppDialogSet().get());
         InfoLog(<< "onNewSession(ServerInviteSessionHandle): replacing existing call");

         // Copy over flag that indicates if we placed the call or not
         mPlacedCall = callToReplace->mPlacedCall;

         if(mPlacedCall)
         {
            // Restart Call Timer
            unique_ptr<ApplicationMessage> timer(new CallTimer(mUserAgent, this));
            mUserAgent.mStack.post(std::move(timer), CallTimerTime, &mUserAgent.getDialogUsageManager());
         }

         // Session to replace was found - end old session
         callToReplace->end();
      }
      else
      {
          // Session to replace not found - reject it
          h->reject(481 /* Call/Transaction Does Not Exist */);
      }
   }
   else
   {
       if (msg.isFromWire())
       {
           mMyUasInviteVideoInfo.mInviteSessionHandle = h;
           //鉴权处理
           
       }
   }
   cout << "***************  ******************* 2\n" << msg << endl;
}

void
UaClientCall::onFailure(ClientInviteSessionHandle h, const SipMessage& msg)
{
   WarningLog(<< "onFailure: msg=" << msg.brief());

   if (msg.isResponse()) 
   {
       //关闭自己申请的流
      switch(msg.header(h_StatusLine).statusCode()) 
      {
         case 408:
         case 503:
            if(!msg.isFromWire())
            {
               // Try another flow? 
            }
         default:
            break;
      }
   }
   cout << "***************  ******************* 3\n" << msg << endl;
}

void
UaClientCall::onEarlyMedia(ClientInviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
   InfoLog(<< "onEarlyMedia: msg=" << msg.brief() << ", sdp=" << sdp);
   cout << "***************  ******************* 4\n" << msg << endl;
}

void
UaClientCall::onProvisional(ClientInviteSessionHandle h, const SipMessage& msg)
{
   InfoLog(<< "onProvisional: msg=" << msg.brief());

   if(isStaleFork(h->getDialogId()))
   {
      // If we receive a response from a stale fork (ie. after someone sends a 200), then we want to ignore it
      InfoLog(<< "onProvisional: from stale fork, msg=" << msg.brief());
      return;
   }
   InfoLog(<< "onProvisional: msg=" << msg.brief());
   cout << "***************  ******************* 5\n" << msg << endl;
}

void
UaClientCall::onConnected(ClientInviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 6 " << msg << endl;
   InfoLog(<< "onConnected: msg=" << msg.brief());
   if(!isUACConnected())
   {
      // It is possible in forking scenarios to get multiple 200 responses, if this is 
      // our first 200 response, then this is the leg we accept, store the connected DialogId
      mUACConnectedDialogId = h->getDialogId();
      // Note:  each forked leg will update mInviteSessionHandle (in onNewSession call) - need to set mInviteSessionHandle for final answering leg on 200
      mInviteSessionHandle = h->getSessionHandle();  

      // start call timer
      unique_ptr<ApplicationMessage> timer(new CallTimer(mUserAgent, this));
      mUserAgent.mStack.post(std::move(timer), CallTimerTime, &mUserAgent.getDialogUsageManager());

      //开启一个rtpserver接收rtp数据
      sipserver::SipServer* pSvr = GetServer();
      ostringstream ss;
      ss << "http://" << (pSvr? pSvr->zlmHost:"127.0.0.1") << ":" << (pSvr? pSvr->zlmHttpPort:8080) << "/index/api/openRtpServer?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&port="
          << mMyUacInviteVideoInfo.rtpPort << "&enable_tcp=0&stream_id="
          << mMyUacInviteVideoInfo.devId << "_" << mMyUacInviteVideoInfo.streamId;
      ss.flush();
      string strReponse = GetRequest(ss.str());
      mMyUacInviteVideoInfo.state = UacInviteVideoInfo::_RES_CONNECT;
   }
   else
   {
      // We already have a connected leg - end this one with a BYE
      h->end();
   }
   cout << "*************** onConnected ***************************\n"
       << msg
       << "***********************************************\n" << endl;
}
void
UaClientCall::onConnected(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 7\n" << msg << endl;
   InfoLog(<< "onConnected: msg=" << msg.brief());
   cout << "*************** onConnected 1 ***************************\n"
       << msg
       << "***********************************************\n" << endl;
}

void UaClientCall::onConnectedConfirmed(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 8\n" << msg << endl;
    InfoLog(<< "onConnectedConfirmed: msg=" << msg.brief());
    if (msg.isRequest())
    {
        if (msg.method() == ACK)
        {
            ownThreadPool::myThreadPool& tPool = ownThreadPool::GetThreadPool();
            tPool.submitTask(std::make_shared<PushRtpStream>(mMyUasInviteVideoInfo.devId, mMyUasInviteVideoInfo.channelID, ssrc.c_str(), connectport.convertInt(), mUserAgent.GetAvailableRtpPort()));
        }
    }
    
    
    cout << "*************** onConnectedConfirmed ***************************\n"
        << msg
        << "***********************************************\n" << endl;
}
void
UaClientCall::onStaleCallTimeout(ClientInviteSessionHandle h)
{
    cout << "***************  ******************* 9\n" << endl;
   WarningLog(<< "onStaleCallTimeout");
}

void
UaClientCall::onTerminated(InviteSessionHandle h, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
{
    cout << "***************  ******************* 10\n" << endl;
   Data reasonData;
   string strUrl;
   switch(reason)
   {
   case InviteSessionHandler::RemoteBye:
      reasonData = "received a BYE from peer";
      {
          if (isUACConnected())
          {
          }
          else
          {
              mUserAgent.CloseStreamStreamId(mMyUasInviteVideoInfo.channelID);
          }
      }
      break;
   case InviteSessionHandler::RemoteCancel:
      reasonData = "received a CANCEL from peer";
      break;   
   case InviteSessionHandler::Rejected:
      reasonData = "received a rejection from peer";
      break;
   case InviteSessionHandler::LocalBye:
      reasonData = "ended locally via BYE";
      break;
   case InviteSessionHandler::LocalCancel:
      reasonData = "ended locally via CANCEL";
      break;
   case InviteSessionHandler::Replaced:
      reasonData = "ended due to being replaced";
      break;
   case InviteSessionHandler::Referred:
      reasonData = "ended due to being referred";
      break;
   case InviteSessionHandler::Error:
      reasonData = "ended due to an error";
      break;
   case InviteSessionHandler::Timeout:
      reasonData = "ended due to a timeout";
      break;
   default:
      assert(false);
      break;
   }

   if(isStaleFork(h->getDialogId()))
   {
      // If we receive a response from a stale fork (ie. after someone sends a 200), then we want to ignore it
      if(msg)
      {
         InfoLog(<< "onTerminated: from stale fork, reason=" << reasonData << ", msg=" << msg->brief());
      }
      else
      {
         InfoLog(<< "onTerminated: from stale fork, reason=" << reasonData);
      }
      return;
   }

   if(msg)
   {
      InfoLog(<< "onTerminated: reason=" << reasonData << ", msg=" << msg->brief());
      cout << "*************** onTerminated ***************************\n"
          << *msg
          << "***********************************************\n" << endl;
   }
   else
   {
      InfoLog(<< "onTerminated: reason=" << reasonData);
   }
}

void
UaClientCall::onRedirected(ClientInviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 11\n" << msg << endl;
   // DUM will recurse on redirect requests, so nothing to do here
   InfoLog(<< "onRedirected: msg=" << msg.brief());
}

void
UaClientCall::onAnswer(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
    cout << "***************  ******************* 12 " << msg << endl;
   if(isStaleFork(h->getDialogId()))
   {
      // If we receive a response from a stale fork (ie. after someone sends a 200), then we want to ignore it
      InfoLog(<< "onAnswer: from stale fork, msg=" << msg.brief() << ", sdp=" << sdp);
      return;
   }
   InfoLog(<< "onAnswer: msg=" << msg.brief() << ", sdp=" << sdp);

   // Process Answer here
   //mMyUacInviteVideoInfo.state
}

void
UaClientCall::onOffer(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
    cout << "***************  ******************* 13\n" << msg << endl;
   if(isStaleFork(h->getDialogId()))
   {
      // If we receive a response from a stale fork (ie. after someone sends a 200), then we want to ignore it
      InfoLog(<< "onOffer: from stale fork, msg=" << msg.brief() << ", sdp=" << sdp);
      return;
   }
   InfoLog(<< "onOffer: msg=" << msg.brief() << ", sdp=" << sdp);

   // Provide Answer here - for test client just echo back same SDP as received for now

   if (msg.isClientTransaction()) // my is uac
   {
       InfoLog(<< "my is uac" << msg.brief());
   }
   else //my is uas
   {
       InfoLog(<< "my is uas" << msg.brief());

       //流存在  直接返回;不存在则申请

       if (sdp.session().OtherAttrHelper().exists("y"))
       {
           ssrc = *sdp.session().OtherAttrHelper().getValues("y").begin();
       }
       const Data &sessionname = sdp.session().name();
       if (sessionname == "Play")
       {
           app = "rtp";
       }
       else if (sessionname == "Playback")
       {
           app = "rtp";
       }
       connectip = sdp.session().origin().getAddress();
       connectport = Data((uint32_t)(*sdp.session().media().begin()).port());
       const SdpContents::Session::MediumContainer& rmedialist = sdp.session().media();
       for (auto& iter : rmedialist)
       {
            connectport = Data(iter.port());
            tcpOrUdp = (iter.name() == "RTP/AVP" ? 0 : 1);
            break;
       }
       std::vector<std::string> subjectArray;
       if (msg.exists(h_Subject))
       {
           subjectArray = std::SipSubjectSplit(msg.header(h_Subject).value().c_str());
       }

       AlegResSdp = sdp;
       SdpContents::Session::MediumContainer &medialist = AlegResSdp.session().media();
       for (auto &iter : medialist)
       {
           if (iter.exists("recvonly"))
           {
               iter.clearAttribute("recvonly");
               iter.addAttribute("sendonly");
           }
           iter.setPort(7000);
           break;
       }
       Data strsdp = AlegResSdp.getBodyData();
       AlegResSdp.session().connection().setAddress("192.168.1.38");
       AlegResSdp.session().origin().setAddress("192.168.1.38");

       if (subjectArray.size() == 4)
       {
           mMyUasInviteVideoInfo.channelID = subjectArray[0];
           mMyUasInviteVideoInfo.serverID = subjectArray[2];
           mMyUasInviteVideoInfo.DataTransMoudle = subjectArray[1];
           if (mUserAgent.IsStreamExist(subjectArray[0]))
           {
               //直接回复
               h->provideAnswer(AlegResSdp);
               ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(h.get());
               if (uas && !uas->isAccepted())
               {
                   uas->accept();
               }
           }
           else
           {
               //拉流
               bool chlOnline = makeBLeg(subjectArray[0].c_str());
               if (chlOnline)
               {
                   /*h->provideAnswer(ssdp);
                   ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(h.get());
                   if (uas && !uas->isAccepted())
                   {
                       uas->accept();
                   }*/
               }
               else
               {
                   WarningCategory warning;
                   warning.hostname() = "192.168.1.230";
                   warning.code() = 488;
                   warning.text() = "channel not exist";
                   h->reject(488, &warning);
               }
           }
       }
       else
       {
            //参数错误
           WarningCategory warning;
           warning.hostname() = "192.168.1.230";
           warning.code() = 488;
           warning.text() = "h_Subject parameter error";
           h->reject(488, &warning);
       }
       

        //拉流
       /*bool chlOnline = false;
        if (subjectArray.size() == 4)
        {
            mMyUasInviteVideoInfo.channelID = subjectArray[0];
            mMyUasInviteVideoInfo.serverID = subjectArray[2];
            mMyUasInviteVideoInfo.DataTransMoudle = subjectArray[1];
            chlOnline = makeBLeg(subjectArray[0].c_str());
        }
        if (chlOnline)
        {
            h->provideAnswer(ssdp);
            ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(h.get());
            if (uas && !uas->isAccepted())
            {
                uas->accept();
            }
        }
        else
        {
            WarningCategory warning;
            warning.hostname() = "192.168.1.230";
            warning.code() = 488;
            warning.text() = "h_Subject parameter error";
            h->reject(488, &warning);
        }*/
   }
  
   cout << "*************** onOffer ***************************\n"
       << msg
       << "***********************************************\n" << endl;
}

void
UaClientCall::onOfferRequired(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 14\n" << msg << endl;
   if(isStaleFork(h->getDialogId()))
   {
      // If we receive a response from a stale fork (ie. after someone sends a 200), then we want to ignore it
      InfoLog(<< "onOfferRequired: from stale fork, msg=" << msg.brief());
      return;
   }
   InfoLog(<< "onOfferRequired: msg=" << msg.brief());

   // Provide Offer Here
   SdpContents offer;
   makeOffer(offer);

   h->provideOffer(offer);
}

void
UaClientCall::onOfferRejected(InviteSessionHandle h, const SipMessage* msg)
{
    cout << "***************  ******************* 15\n" << msg << endl;
   if(isStaleFork(h->getDialogId()))
   {
      // If we receive a response from a stale fork (ie. after someone sends a 200), then we want to ignore it
      if(msg)
      {
         WarningLog(<< "onOfferRejected: from stale fork, msg=" << msg->brief());
      }
      else
      {
         WarningLog(<< "onOfferRejected: from stale fork");
      }
      return;
   }
   if(msg)
   {
      WarningLog(<< "onOfferRejected: msg=" << msg->brief());
   }
   else
   {
      WarningLog(<< "onOfferRejected");
   }
}

void
UaClientCall::onOfferRequestRejected(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 16\n" << msg << endl;
   InfoLog(<< "onOfferRequestRejected: msg=" << msg.brief());
   // This is called when we are waiting to resend a INVITE with no sdp after a glare condition, and we 
   // instead receive an inbound INVITE or UPDATE
}

void
UaClientCall::onRemoteSdpChanged(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
    cout << "***************  ******************* 17\n" << msg << endl;
   /// called when a modified SDP is received in a 2xx response to a
   /// session-timer reINVITE. Under normal circumstances where the response
   /// SDP is unchanged from current remote SDP no handler is called
   /// There is not much we can do about this.  If session timers are used then they are managed seperately per leg
   /// and we have no real mechanism to notify the other peer of new SDP without starting a new offer/answer negotiation
   InfoLog(<< "onRemoteSdpChanged: msg=" << msg << ", sdp=" << sdp);

   // Process SDP Answer here
}

void
UaClientCall::onInfo(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 18\n" << msg << endl;
   InfoLog(<< "onInfo: msg=" << msg.brief());

   // Handle message here
   h->acceptNIT();
}

void
UaClientCall::onInfoSuccess(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 19\n" << msg << endl;
   InfoLog(<< "onInfoSuccess: msg=" << msg.brief());
}

void
UaClientCall::onInfoFailure(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 20\n" << msg << endl;
   WarningLog(<< "onInfoFailure: msg=" << msg.brief());
}

void
UaClientCall::onRefer(InviteSessionHandle h, ServerSubscriptionHandle ss, const SipMessage& msg)
{
    cout << "***************  ******************* 21\n" << msg << endl;
   InfoLog(<< "onRefer: msg=" << msg.brief());

   // Handle Refer request here
}

void
UaClientCall::onReferAccepted(InviteSessionHandle h, ClientSubscriptionHandle csh, const SipMessage& msg)
{
    cout << "***************  ******************* 22\n" << msg << endl;
   InfoLog(<< "onReferAccepted: msg=" << msg.brief());
}
void UaClientCall::onAckReceived(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 23\n" << msg << endl;
    InfoLog(<< "onOffer: " << msg);

    cout << "*************** onReferAccepted ***************************\n"
        << msg
        << "***********************************************\n" << endl;
}
void
UaClientCall::onReferRejected(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 24\n" << msg << endl;
   WarningLog(<< "onReferRejected: msg=" << msg.brief());
}

void
UaClientCall::onReferNoSub(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 25\n" << msg << endl;
   InfoLog(<< "onReferNoSub: msg=" << msg.brief());

   // Handle Refer request with (no-subscription indication) here
}

void
UaClientCall::onMessage(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 26\n" << msg << endl;
   InfoLog(<< "onMessage: msg=" << msg.brief());

   // Handle message here
   h->acceptNIT();

   if(!mPlacedCall)
   {
      // If we didn't place the call - answer the message with another message
      PlainContents plain("test message answer");
      h->message(plain);
   }
}

void
UaClientCall::onMessageSuccess(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 27\n" << msg << endl;
   InfoLog(<< "onMessageSuccess: msg=" << msg.brief());
}

void
UaClientCall::onMessageFailure(InviteSessionHandle h, const SipMessage& msg)
{
    cout << "***************  ******************* 28\n" << msg << endl;
   WarningLog(<< "onMessageFailure: msg=" << msg.brief());
}

void
UaClientCall::onForkDestroyed(ClientInviteSessionHandle h)
{
    cout << "***************  ******************* 29\n" << endl;
   InfoLog(<< "onForkDestroyed:");
}

void 
UaClientCall::onReadyToSend(InviteSessionHandle h, SipMessage& msg)
{
    cout << "***************  ******************* 30\n" << msg << endl;
}

void 
UaClientCall::onFlowTerminated(InviteSessionHandle h)
{
    cout << "***************  ******************* 31\n" << endl;
   if(h->isConnected())
   {
      NameAddr inviteWithReplacesTarget;
      if(h->remoteTarget().uri().exists(p_gr))
      {
         // If remote contact is a GRUU then use it
         inviteWithReplacesTarget.uri() = h->remoteTarget().uri();
      }
      else
      {
         //.Use remote AOR
         inviteWithReplacesTarget.uri() = h->peerAddr().uri();
      }
      InfoLog(<< "UaClientCall::onFlowTerminated: trying INVITE w/replaces to " << inviteWithReplacesTarget);
      // The flow terminated - try an Invite (with Replaces) to recover the call
      UaClientCall *replacesCall = new UaClientCall(mUserAgent);      

      // Copy over flag that indicates wether original call was placed or received
      replacesCall->mPlacedCall = mPlacedCall;  

      // Note:  We want to end this call since it is to be replaced.  Normally the endpoint
      // receiving the INVITE with replaces would send us a BYE for the session being replaced.
      // However, since the old flow is dead, we will never see this BYE.  We need this call to
      // go away somehow, however we cannot just end it directly here via terminateCall.
      // Since the flow to other party is likely fine - if we terminate this call now the BYE 
      // is very likely to make it to the far end, before the above INVITE - if this happens then 
      // the replaces logic of the INVITE will have no effect.  We want to delay the release of 
      // this call, by passing our handle to the new INVITE call and have it terminate this call, 
      // once we know the far end has processed our new INVITE.
      replacesCall->mInviteSessionHandleReplaced = mInviteSessionHandle;

      SdpContents offer;
      replacesCall->makeOffer(offer);
      shared_ptr<SipMessage> invite = mUserAgent.getDialogUsageManager().makeInviteSession(inviteWithReplacesTarget, h, getUserProfile(), &offer, replacesCall);
      mUserAgent.getDialogUsageManager().send(invite);
   }
}

////////////////////////////////////////////////////////////////////////////////
// DialogSetHandler ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void 
UaClientCall::onTrying(AppDialogSetHandle h, const SipMessage& msg)
{
   InfoLog(<< "onTrying: msg=" << msg.brief());
   if(isUACConnected()) return;  // Ignore 100's if already connected

   // Handle message here
}

void 
UaClientCall::onNonDialogCreatingProvisional(AppDialogSetHandle h, const SipMessage& msg)
{
   InfoLog(<< "onNonDialogCreatingProvisional: msg=" << msg.brief());
   if(isUACConnected()) return;  // Ignore provionals if already connected

   // Handle message here
}

////////////////////////////////////////////////////////////////////////////////
// ClientSubscriptionHandler ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
UaClientCall::onUpdatePending(ClientSubscriptionHandle h, const SipMessage& msg, bool outOfOrder)
{
   InfoLog(<< "onUpdatePending(ClientSubscriptionHandle): " << msg.brief());
   if (msg.exists(h_Event) && msg.header(h_Event).value() == "refer")
   {
      //process Refer Notify Here
   }
   h->acceptUpdate();
}

void
UaClientCall::onUpdateActive(ClientSubscriptionHandle h, const SipMessage& msg, bool outOfOrder)
{
   InfoLog(<< "onUpdateActive(ClientSubscriptionHandle): " << msg.brief());
   if (msg.exists(h_Event) && msg.header(h_Event).value() == "refer")
   {
      //process Refer Notify Here
   }
   h->acceptUpdate();
}

void
UaClientCall::onUpdateExtension(ClientSubscriptionHandle h, const SipMessage& msg, bool outOfOrder)
{
   InfoLog(<< "onUpdateExtension(ClientSubscriptionHandle): " << msg.brief());
   if (msg.exists(h_Event) && msg.header(h_Event).value() == "refer")
   {
      //process Refer Notify Here
   }
   h->acceptUpdate();
}

void 
UaClientCall::onNotifyNotReceived(resip::ClientSubscriptionHandle h)
{
   InfoLog(<< "onNotifyNotReceived(ClientSubscriptionHandle)");
   h->end();
}

void
UaClientCall::onTerminated(ClientSubscriptionHandle h, const SipMessage* msg)
{
   if(msg)
   {
      InfoLog(<< "onTerminated(ClientSubscriptionHandle): " << msg->brief());
      //Note:  Final notify is sometimes only passed in the onTerminated callback
      if (msg->isRequest() && msg->exists(h_Event) && msg->header(h_Event).value() == "refer")
      {
         //process Refer Notify Here
      }
   }
   else
   {
      InfoLog(<< "onTerminated(ClientSubscriptionHandle)");
   }
}

void
UaClientCall::onNewSubscription(ClientSubscriptionHandle h, const SipMessage& msg)
{
   InfoLog(<< "onNewSubscription(ClientSubscriptionHandle): " << msg.brief());
}

int 
UaClientCall::onRequestRetry(ClientSubscriptionHandle h, int retrySeconds, const SipMessage& msg)
{
   InfoLog(<< "onRequestRetry(ClientSubscriptionHandle): " << msg.brief());
   return -1;
}

void 
UaClientCall::onRedirectReceived(AppDialogSetHandle h, const SipMessage& msg)
{
   InfoLog(<< "onRedirectReceived: msg=" << msg.brief());
}


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
bool RequestStreamTask::TaskRun()
{
    UaClientCall* pUacCall = new UaClientCall(mUserAgent);
    if (pUacCall)
    {
        pUacCall->mMyUacInviteVideoInfo.rtpPort = rtpPort;
        pUacCall->mMyUacInviteVideoInfo.devId = devId;
        pUacCall->mMyUacInviteVideoInfo.streamId = streamId;
        if (mUserAgent.RequestStream(devIp, devPort, streamId, rtpPort, pUacCall))
        {
            for (int i = 0; i < 10 * 5; i++)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                switch (pUacCall->mMyUacInviteVideoInfo.state)
                {
                case UaClientCall::UacInviteVideoInfo::_RES_START:
                    break;
                case UaClientCall::UacInviteVideoInfo::_RES_GET1XX:
                    break;
                case UaClientCall::UacInviteVideoInfo::_RES_GETSDP:
                    break;
                case UaClientCall::UacInviteVideoInfo::_RES_CONNECT:
                {
                    if (mAlegCall && mAlegCall->mMyUasInviteVideoInfo.mInviteSessionHandle.isValid())
                    {
                        mAlegCall->mMyUasInviteVideoInfo.mInviteSessionHandle->provideAnswer(mAlegCall->AlegResSdp);
                        ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(mAlegCall->mMyUasInviteVideoInfo.mInviteSessionHandle.get());
                        if (uas && !uas->isAccepted())
                        {
                            uas->accept();
                            pUacCall->mMyUacInviteVideoInfo.state = UaClientCall::UacInviteVideoInfo::_RES_ACK;
                            //return true;
                        }
                    }
                }
                case UaClientCall::UacInviteVideoInfo::_RES_ACK:
                {
                    //ownThreadPool::myThreadPool& tPool = ownThreadPool::GetThreadPool();
                    //tPool.submitTask(std::make_shared<PushRtpStream>(mAlegCall->mMyUasInviteVideoInfo.devId, mAlegCall->mMyUasInviteVideoInfo.channelID, mAlegCall->ssrc.c_str(), mAlegCall->connectport.convertInt()));
                    return true;
                }
                break;
                default:
                    break;
                }
            }
        }
    }
    else
    {
        if (mAlegCall)
        {
            if (mAlegCall->mMyUasInviteVideoInfo.mInviteSessionHandle.isValid())
            {
                WarningCategory warning;
                warning.hostname() = "192.168.1.230";
                warning.code() = 488;
                warning.text() = "b leg create failed";
                mAlegCall->mMyUasInviteVideoInfo.mInviteSessionHandle->reject(488, &warning);
            }
        }
        
    }
    return true;
};
bool PushRtpStream::TaskRun()
{
    bool isStream = false;
    std::string app;
    std::string streamId;
    rapidjson::Document document;
    sipserver::SipServer* pSvr = GetServer();
    for (int i = 0; i < 2 * 5; i++)
    {
        ostringstream ss;
        ss << "http://" << (pSvr?pSvr->zlmHost:"127.0.0.1") <<":" << (pSvr?pSvr->zlmHttpPort:8080) << "/index/api/isMediaOnline?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&"
            << "schema=rtsp&vhost=__defaultVhost__&app=rtp&stream="
            << stream_Id;
        ss.flush();
        std::string strResponse = GetRequest(ss.str());
        document.Parse((char*)strResponse.c_str());
        if (!document.HasParseError())
        {
            if (document.HasMember("online"))
            {
                if (document["online"].IsBool())
                {
                    isStream = document["online"].GetBool();
                    if (isStream)
                        break;
                }
                else
                {
                    cout << "online not bool type" << endl;
                }
            }
        }
        //"{{ZLMediaKit_URL}}/index/api/isMediaOnline?secret={{ZLMediaKit_secret}}&schema=rtsp&vhost={{defaultVhost}}&app=rtp&stream=34020000001180000800_34020000001320000014"
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    if (isStream)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ostringstream ss;
        ss << "http://" << (pSvr?pSvr->zlmHost:"127.0.0.1") << ":" << (pSvr?pSvr->zlmHttpPort:8080) << "/index/api/startSendRtp?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&vhost=__defaultVhost__&app="
            << "rtp" << "&stream=" << devId << "_" << channelId
            << "&ssrc=" << strSsrc << "&dst_url=192.168.1.232&dst_port=" << connectPort
            << "&is_udp=1&src_port=" << localPort;
        ss.flush();

        string strReponse = GetRequest(ss.str());
        rapidjson::Document document;
        document.Parse((char*)strReponse.c_str());
        if (document.HasParseError())
        {
            return false;
        }
        if (document.HasMember("local_port") && json_check_uint32(document, "local_port") > 0)
        {
            cout << "startSendRtp ok local_port:" << json_check_uint32(document, "local_port") << endl;
        }
        else
        {
            if (document.HasMember("msg"))
            {
                cout << "startSendRtp failed streamdid:" << stream_Id << " msg:" << json_check_string(document, "msg") << endl;
            }
        }
    }
    else
    {
        cout << "getMediaList error streamid:" << stream_Id << endl;
    }
    return true;
};