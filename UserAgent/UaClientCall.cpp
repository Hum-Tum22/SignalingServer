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
#include <rutil/DnsUtil.hxx>

#include <sstream>
#include <regex>
#include <time.h>
#include "http.h"
#include "../device/DeviceManager.h"
#include "../tools/m_Time.h"

#include "myjsondef.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"
#include "../SipServer.h"

#include "../media/rtp-udp-transport.h"
#include "../media/MediaMng.h"
#include "../media/mediaIn/JsonStream.h"

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
    mUACConnectedDialogId(Data::Empty, Data::Empty, Data::Empty), mSessionState(-1)
{
   mUserAgent.registerCall(this);
   psSource = NULL;
}

UaClientCall::~UaClientCall()
{
    mUserAgent.unregisterCall(this);
    if (isUACConnected())
    {
        if (myRtpPort > 0)
        {
            if (mInviteSessionHandle.isValid())
            {
                mInviteSessionHandle->end();
            }
            mUserAgent.FreeRptPort(myRtpPort);
            mUserAgent.CloseStreamStreamId(streamId);
            closeMediaStream();
            myRtpPort = 0;
        }
    }
    else
    {
        if (myRtpPort > 0)
        {
            if (psSource)
            {
                //psSource中使用了MediaStream::Ptr   先释放MediaStream::Ptr
                delete psSource; psSource = NULL;
            }
            printf("ua call close stream:%s\n", streamId.c_str());
            mUserAgent.CloseStreamStreamId(streamId);
            mUserAgent.FreeRptPort(myRtpPort);
            /*sipserver::SipServer* pSvr = GetServer();
            ostringstream ss;
            ss << "http://" << (pSvr ? pSvr->zlmHost : "127.0.0.1") << ":" << (pSvr ? pSvr->zlmHttpPort : 8080) << "/index/api/stopSendRtp?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&vhost=__defaultVhost__&app=rtp&stream="
                << devId << "_" << streamId;
            string strReponse = GetRequest(ss.str());*/
            myRtpPort = 0;
        }
    }
}

void 
UaClientCall::initiateCall(const Uri& target, shared_ptr<UserProfile> profile)
{
    Data myId = profile->getDefaultFrom().uri().user();
    SdpContents offer;
    makeMyOffer(offer, myId.c_str());

    shared_ptr<SipMessage> invite = mUserAgent.getDialogUsageManager().makeInviteSession(NameAddr(target), profile, &offer, this);
    invite->header(h_Subject) = StringCategory(channelId.c_str() + Data(":") + Data(ssrc) + Data(",") + myId + Data(":") + Data("0"));
    invite->header(h_From).uri().user() = myId;
    invite->header(h_From).uri().host() = profile->getDefaultFrom().uri().host();
    invite->header(h_From).uri().port() = profile->getDefaultFrom().uri().port();
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
    bool isTerminate = true;
    if(mTimerExpiredCounter < CallTimeCounterToByeOn)
    {
        // First few times, send a message to the other party
        if (mSessionState < 0 || mSessionState > 0)
        {
            if (mSessionState < 0)
            {
                mSessionState = 0;
            }
            if (mInviteSessionHandle.isValid())
            {
                //PlainContents contents("Keepalive", Mime("Application", "MANSCDP+xml"));
                PlainContents contents("Keepalive");
                mInviteSessionHandle->message(contents);
            }
        }
        else
        {
            //设备不支持 session message 不再send
            mTimerExpiredCounter = 0;
        }
    }
    else 
    {
        // Then hangup
        terminateCall();
        isTerminate = false;
    }
    auto mdaStream = MediaMng::GetInstance().findStream(streamId);
    if (mdaStream)
    {
        if (time(0) - mdaStream->LastFrameTime() > 5)
        {
            terminateCall();
        }
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
void UaClientCall::makeMyOffer(SdpContents& offer, std::string myId)
{
    offer.session().version() = 0;
    offer.session().origin() = resip::SdpContents::Session::Origin(Data(myId), 0/* CreateSessionID(12345)*/, 0, SdpContents::IP4, Data(mySdpIp));
    offer.session().connection() = resip::SdpContents::Session::Connection(SdpContents::IP4, Data(mySdpIp));
    offer.session().name() = Data(sessionName);
    if (sessionName == "Play")
    {
        offer.session().addTime(resip::SdpContents::Session::Time(0, 0));
    }
    else if (sessionName == "Playback")
    {
        offer.session().addTime(resip::SdpContents::Session::Time(startTime, stopTime));
    }
    //offer.session().OtherAttrHelper().addAttribute("y", ssrc.c_str());// = CreateSSRC(name, DesId);// y

    Data dprotocol = "RTP/AVP";
    if (rtpType == 1 || rtpType == 2)
    {
        dprotocol = "TCP/RTP/AVP";
    }
    //list<SDPDATATYPEPORT>::iterator iter = DataTypeList.begin();
    //for (;iter != DataTypeList.end();iter++)
    {
        //Data MediaName = GetMediaNameByType((GB28181_DATATYPE)iter->datatype);
        resip::SdpContents::Session::Medium videoMedium("video", myRtpPort, 0, dprotocol);
        //for (; iter != DataTypeList.end(); iter++)
        {
            videoMedium.addCodec(resip::SdpContents::Session::Codec("PS", 96, 90000));
        }

        if (rtpType == 1)
        {
            //videoMedium.addAttribute("TCP_CLIENT", Data(rtpPort));
            videoMedium.addAttribute("setup", "active");
        }
        else if (rtpType == 2)
        {
            //videoMedium.addAttribute("TCP_SERVER", Data(rtpPort));
            videoMedium.addAttribute("setup", "passive");
        }
        //if (ProtocolType == RTP_TCP_S)
        //{
        //    //videoMedium.addAttribute("TCP_SERVER",Data(iter->port));
        //    videoMedium.addAttribute("setup", "passive");
        //}
        //else if (ProtocolType == RTP_TCP_C)
        //{
        //    //videoMedium.addAttribute("TCP_CLIENT",Data(iter->port));
        //    videoMedium.addAttribute("setup", "active");
        //}
        //if (name == Data("Download"))
        //{
        //    Data DownSpeed(pDownloadParam->nDownloadSpeed);
        //    videoMedium.addAttribute("downloadspeed", DownSpeed);
        //}
        videoMedium.addAttribute("recvonly");
        offer.session().addMedium(videoMedium);
    }
}
void UaClientCall::closeMediaStream()
{
    sipserver::SipServer* pSvr = GetServer();
    ostringstream ss;
    ss << "http://" << (pSvr ? pSvr->zlmHost : "127.0.0.1") << ":" << (pSvr ? pSvr->zlmHttpPort : 8080) << "/index/api/closeRtpServer?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc"
        << "&stream_id=" << streamId;
    ss.flush();
    /*string strReponse = GetRequest(ss.str());
    rapidjson::Document document;
    document.Parse((char*)strReponse.c_str());
    if (!document.HasParseError())
    {
        int codeno = json_check_int32(document, "code");
        if (codeno == 0)
        {
            mUserAgent.setCallStatus(streamId, CALL_MEDIA_STREAM_CLOSED);
        }
    }*/
}
bool UaClientCall::makeBLeg()
{
    return mUserAgent.RequestLiveStream(devId, devIp, devPort, channelId, 0, mUserAgent.GetAvailableRtpPort(), 0, this);
}
void UaClientCall::ReceiveInviteOffRequest(resip::InviteSessionHandle handle, const resip::SipMessage& msg, const resip::SdpContents& offer)
{
    //解析offer sdp
    mRequestSdp = offer;
    app = "rtp";
    sessionName = offer.session().name();
    ssrc = offer.session().YSSRC().Value().c_str();

    remoteIp = offer.session().origin().getAddress();
    

    const SdpContents::Session::MediumContainer& rmedialist = offer.session().media();
    for (auto& iter : rmedialist)
    {
        if (iter.protocol() == Data("RTP/AVP"))
        {
            rtpType = 0;
        }
        else if (iter.protocol() == Data("TCP/RTP/AVP"))
        {
            rtpType = 1;
            if (iter.exists("setup"))
            {
                //active passive
                //if(iter.getValues("setup"))
            }
        }
        else
        {
            rtpType = 0;
        }
        if (iter.port() > 0)
        {
            remotePort = iter.port();
            break;
        }
    }

    //解析h_Subject
    std::vector<std::string> subjectArray;
    if (msg.exists(h_Subject))
    {
        std::string line = msg.header(h_Subject).value().c_str();
        std::regex eSubject("(.*):(.*),(.*):(.*)");
        std::smatch smSubject;

        std::regex_search(line, smSubject, eSubject);
        for (uint32_t i = 1; i < smSubject.size(); i++)
        {
            subjectArray.push_back(smSubject[i].str());
        }
    }

    //创建answer sdp
    mResponseSdp = offer;
    myRtpPort = mUserAgent.GetAvailableRtpPort();
    SdpContents::Session::MediumContainer& medialist = mResponseSdp.session().media();
    for (auto& iter : medialist)
    {
        if (iter.exists("recvonly"))
        {
            iter.clearAttribute("recvonly");
            iter.addAttribute("sendonly");
        }
        iter.setPort(myRtpPort);
        break;
    }
    mResponseSdp.session().connection().setAddress(DnsUtil::getLocalIpAddress());
    mResponseSdp.session().origin().setAddress(DnsUtil::getLocalIpAddress());

    if (subjectArray.size() == 4)
    {
        if (sessionName == Data("Play"))
        {
            channelId = subjectArray[0];

            //channelId = "37028806251320111559";
            //channelId = "37028806251320111506";
            
            remoteId = subjectArray[2];
            streamId = channelId + "_" + std::to_string(0);
            auto mdaStream = MediaMng::GetInstance().findStream(streamId);
            if (mdaStream)
            {
                //流存在//直接回复
                //mResponseSdp
                auto transport = std::make_shared<RTPUdpTransport>();
                unsigned short localport[2] = { myRtpPort, ++myRtpPort };
                unsigned short peerport[2] = { remotePort, ++remotePort };
                if (0 != transport->Init(localport, remoteIp.c_str(), peerport))
                {
                    psSource = new PSFileSource("", ssrc.convertInt());
                    psSource->SetTransport("sip", transport);
                    mdaStream->increasing();
                    psSource->setMediaStream(mdaStream);
                }
                std::cout << "found stream :" << channelId << std::endl;
                handle->provideAnswer(mResponseSdp);
                ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(handle.get());
                if (uas && !uas->isAccepted())
                {
                    uas->accept();
                }
            }
            else
            {
                //流不存在申请
                bool chlOnline = makeBLeg();
                if (!chlOnline)
                {
                    WarningCategory warning;
                    warning.hostname() = DnsUtil::getLocalIpAddress();
                    warning.code() = 488;
                    warning.text() = "make bleg error!";
                    handle->reject(488, &warning);
                }
                else
                {
                    auto transport = std::make_shared<RTPUdpTransport>();
                    unsigned short localport[2] = { myRtpPort, ++myRtpPort };
                    unsigned short peerport[2] = { remotePort, ++remotePort };
                    if (0 != transport->Init(localport, remoteIp.c_str(), peerport))
                    {
                        MediaStream::Ptr streamInfo = MediaMng::GetInstance().findStream(streamId);
                        if (streamInfo)
                        {
                            psSource = new PSFileSource("", ssrc.convertInt());
                            psSource->SetTransport("sip", transport);
                            streamInfo->increasing();
                            printf("xxxxxxxxxxxxxxxxx stream:%s ref:%d\n", streamId.c_str(), streamInfo->refNum());
                            psSource->setMediaStream(streamInfo);
                        }
                        //streamInfo->setMediaSource(psSource);
                        //return true;
                    }
                    //直接回复
                        
                    //mResponseSdp
                    std::cout << "makeBLeg OK :" << std::endl;
                    handle->provideAnswer(mResponseSdp);
                    ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(handle.get());
                    if (uas && !uas->isAccepted())
                    {
                        uas->accept();
                    }
                }
            }
        }
        else if (sessionName == Data("Playback"))
        {
            if (offer.session().getTimes().size() == 2)
            {
                startTime = offer.session().getTimes().front().getStart();
                stopTime = offer.session().getTimes().front().getStop();
            }
            IDeviceMngrSvr& DevMgr = GetIDeviceMngr();
            SipServerDeviceInfo devuinfo;
            list<GBDeviceChannel> chlist = DevMgr.GetGBDeviceMapper().GetGBDeviceChannelMapper().queryChannelByChannelId(channelId.c_str());
            for (auto& it : chlist)
            {
                if (it.getStatus())
                {
                    devuinfo = DevMgr.GetGBDeviceMapper().getDeviceByDeviceId(it.getDeviceId());
                    devId = devuinfo.getDeviceId();
                    devIp = devuinfo.getIp();
                    devPort = devuinfo.getPort();

                    CDateTime staTime(startTime);
                    CDateTime endTime(stopTime);
                    //streamId = std::str_format("%s_%s_%s_%s", devId.c_str(), channelId.c_str(), staTime.tmFormat("%Y-%m-%dT%H:%M:%S").c_str(), endTime.tmFormat("%Y-%m-%dT%H:%M:%S").c_str());
                    break;
                }
            }
            //拉流
            bool chlOnline = makeBLeg();
            if (!chlOnline)
            {
                WarningCategory warning;
                warning.hostname() = DnsUtil::getLocalIpAddress();
                warning.code() = 488;
                warning.text() = "make bleg error!";
                handle->reject(488, &warning);
            }
        }
        else if (sessionName == Data("Download"))
        {
            //拉流
            bool chlOnline = makeBLeg();
            if (!chlOnline)
            {
                WarningCategory warning;
                warning.hostname() = DnsUtil::getLocalIpAddress();
                warning.code() = 488;
                warning.text() = "make bleg error!";
                handle->reject(488, &warning);
            }
        }
        else
        {
            WarningCategory warning;
            warning.hostname() = DnsUtil::getLocalIpAddress();
            warning.code() = 488;
            warning.text() = "sdp session name error!";
            handle->reject(488, &warning);
        }

    }
    else
    {
        //参数错误
        WarningCategory warning;
        warning.hostname() = DnsUtil::getLocalIpAddress();
        warning.code() = 488;
        warning.text() = "h_Subject parameter error";
        handle->reject(488, &warning);
    }
}
resip::InviteSessionHandle& UaClientCall::getInviteSessionHandle()
{
    return mInviteSessionHandle;
}

void __stdcall UaClientCall::VskX86NvrRtPreDataCb(uint32_t handle, const uint8_t*data, size_t dataSize, void *pUser)
{
    UaClientCall* pThis = (UaClientCall*)pUser;
    if (pThis)
    {
        /*if (pThis->pStreamInfo && pThis->pStreamInfo->psSource)
        {
            pThis->pStreamInfo->psSource->Input(0, data, dataSize);
        }
        else
        {
            
        }*/
    }
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
   mUserAgent.setCallStatus(streamId, CALL_UAC_NEW_GET1XX);
   m_CallEvt.notify_one();
   //cout << "***************  ******************* 1 " << msg << endl;
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
       //h->provisional(100);
       h->provisional(180);
       if (msg.isFromWire())
       {
           //鉴权处理
           
       }
   }
   mUserAgent.setCallStatus(streamId, CALL_UAS_NEW);
   m_CallEvt.notify_one();
   //cout << "***************  ******************* 2\n" << msg << endl;
}

void
UaClientCall::onFailure(ClientInviteSessionHandle h, const SipMessage& msg)
{
   WarningLog(<< "onFailure: msg=" << msg.brief());
   mUserAgent.setCallStatus(streamId, CALL_UAC_FAILURE);
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
                m_CallEvt.notify_one();
            }
         default:
            break;
      }
   }
   //cout << "***************  ******************* 3\n" << msg << endl;
}

void
UaClientCall::onEarlyMedia(ClientInviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
   InfoLog(<< "onEarlyMedia: msg=" << msg.brief() << ", sdp=" << sdp);
   //cout << "***************  ******************* 4\n" << msg << endl;
}

void
UaClientCall::onProvisional(ClientInviteSessionHandle h, const SipMessage& msg)
{
   InfoLog(<< "onProvisional: msg=" << msg.brief());
   mUserAgent.setCallStatus(streamId, CALL_UAC_GET1XX_PROV);
   if(isStaleFork(h->getDialogId()))
   {
      // If we receive a response from a stale fork (ie. after someone sends a 200), then we want to ignore it
      InfoLog(<< "onProvisional: from stale fork, msg=" << msg.brief());
      return;
   }
   InfoLog(<< "onProvisional: msg=" << msg.brief());
   //cout << "***************  ******************* 5\n" << msg << endl;
   
}

void
UaClientCall::onConnected(ClientInviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 6 " << msg << endl;
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

        mUserAgent.setCallStatus(streamId, CALL_UAC_CONNECTED);
        m_CallEvt.notify_one();
        /*sipserver::SipServer* pSvr = GetServer();
        ostringstream ss;
        ss << "http://" << (pSvr ? pSvr->zlmHost : "127.0.0.1") << ":" << (pSvr ? pSvr->zlmHttpPort : 8080) << "/index/api/openRtpServer?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&port="
            << myRtpPort;
        if (rtpType == 0)
        {
            ss << "&enable_tcp=0";
        }
        else
        {
            ss << "&enable_tcp=1";
        }
        ss << "&stream_id=" << streamId;
        ss.flush();
        string strReponse = GetRequest(ss.str());
        rapidjson::Document document;
        document.Parse((char*)strReponse.c_str());
        if (!document.HasParseError())
        {
            int codeno = json_check_int32(document, "code");
            if (codeno == 0)
            {
                mUserAgent.setCallStatus(streamId, CALL_MEDIA_READY);
                return;
            }
        }*/
        mUserAgent.setCallStatus(streamId, CALL_MEDIA_ERROR);
    }
    else
    {
        // We already have a connected leg - end this one with a BYE
        h->end();
    }
   /*cout << "*************** onConnected ***************************\n"
       << msg
       << "***********************************************\n" << endl;*/
}
void
UaClientCall::onConnected(InviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 7\n" << msg << endl;
   InfoLog(<< "onConnected: msg=" << msg.brief());
   /*cout << "*************** onConnected 1 ***************************\n"
       << msg
       << "***********************************************\n" << endl;*/
   //mUserAgent.setCallStatus(streamId, CALL_UAS_CONNECTED);
}

void UaClientCall::onConnectedConfirmed(InviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 8\n" << msg << endl;
    InfoLog(<< "onConnectedConfirmed: msg=" << msg.brief());
    if (msg.isRequest())
    {
        if (msg.method() == ACK)
        {
            /*mMyUasInviteVideoInfo.devId, mMyUasInviteVideoInfo.channelID, ssrc.c_str(), 
                connectport.convertInt(), mUserAgent.GetAvailableRtpPort()*/
            MediaStream::Ptr streamInfo =  MediaMng::GetInstance().findStream(streamId);
            //auto dev = mUserAgent.GetStreamInfo(streamId);
            if (streamInfo)
            {
                if (psSource)
                {
                    psSource->run();
                    mUserAgent.setCallStatus(streamId, CALL_MY_MEDIA_OK);
                }
                //if(streamInfo->getStreamId())
                for (int i = 0; i < 2 * 5; i++)
                {
                    resip::UaMgr::streamStatus smStatus = resip::UaMgr::_UERAGERNT_NOT_STREAM;// = (resip::UaMgr::streamStatus)dev->streamStatus;
                    if (smStatus == resip::UaMgr::_UERAGERNT_STREAM_OK)
                    {
                        //dev->psSource->run();
                        mUserAgent.setCallStatus(streamId, CALL_MY_MEDIA_OK);
                        break;
                    }
                    else if (smStatus == resip::UaMgr::_UERAGERNT_NOT_STREAM)
                    {
                        mUserAgent.setCallStatus(streamId, CALL_UAC_TERMINATED);
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }
            
        }
    }
    
    
    /*cout << "*************** onConnectedConfirmed ***************************\n"
        << msg
        << "***********************************************\n" << endl;*/
    //mUserAgent.setCallStatus(streamId, CALL_UAS_CONNECTED_CONFIRMED);
}
void
UaClientCall::onStaleCallTimeout(ClientInviteSessionHandle h)
{
    //cout << "***************  ******************* 9\n" << endl;
   WarningLog(<< "onStaleCallTimeout");
}

void
UaClientCall::onTerminated(InviteSessionHandle h, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
{
    //cout << "***************  ******************* 10\n" << endl;
    if (isUACConnected())
    {
        mUserAgent.setCallStatus(streamId, CALL_UAC_TERMINATED);
    }
    else
    {
        //mUserAgent.setCallStatus(streamId, CALL_UAS_TERMINATED);
    }
   Data reasonData;
   string strUrl;
   switch(reason)
   {
   case InviteSessionHandler::RemoteBye:
      reasonData = "received a BYE from peer";
      {
          if (isUACConnected())
          {
              m_CallEvt.notify_one();
          }
          else
          {
              //mUserAgent.CloseStreamStreamId(streamId);
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
      /*cout << "*************** onTerminated ***************************\n"
          << *msg
          << "***********************************************\n" << endl;*/
   }
   else
   {
      InfoLog(<< "onTerminated: reason=" << reasonData);
   }
}

void
UaClientCall::onRedirected(ClientInviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 11\n" << msg << endl;
   // DUM will recurse on redirect requests, so nothing to do here
   InfoLog(<< "onRedirected: msg=" << msg.brief());
}

void
UaClientCall::onAnswer(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
    //cout << "***************  ******************* 12 " << msg << endl;
    mUserAgent.setCallStatus(streamId, CALL_UAC_ANSWER_200OK);
   if(isStaleFork(h->getDialogId()))
   {
      // If we receive a response from a stale fork (ie. after someone sends a 200), then we want to ignore it
      InfoLog(<< "onAnswer: from stale fork, msg=" << msg.brief() << ", sdp=" << sdp);
      return;
   }
   InfoLog(<< "onAnswer: msg=" << msg.brief() << ", sdp=" << sdp);

   NameAddr myaddr = h->myAddr();
   NameAddr peeraddr = h->peerAddr();
   NameAddr remoteTarget = h->remoteTarget();
   NameAddr pendingRemoteTarget = h->pendingRemoteTarget();
   std::thread t(&UaClientCall::createRtpServer, this, 30000, "192.168.1.221", 15060);
   t.detach();
   // Process Answer here
   //Tuple sourceTuple = msg.getSource();
   //in_addr_t msgSourceAddress = sourceTuple.toGenericIPAddress().v4Address.sin_addr.s_addr;
   //call->onAnswer(this, sdp, msgSourceAddress);
   //开启一个rtpserver接收rtp数据
   m_CallEvt.notify_one();
}

void
UaClientCall::onOffer(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
    //cout << "***************  ******************* 13\n" << msg << endl;
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
       //mUserAgent.setCallStatus(streamId, CALL_UAS_RECEIVE_OFFER);
       ReceiveInviteOffRequest(h, msg, sdp);
   }
  
   /*cout << "*************** onOffer ***************************\n"
       << msg
       << "***********************************************\n" << endl;*/
}

void
UaClientCall::onOfferRequired(InviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 14\n" << msg << endl;
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
    //cout << "***************  ******************* 15\n" << msg << endl;
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
    //cout << "***************  ******************* 16\n" << msg << endl;
   InfoLog(<< "onOfferRequestRejected: msg=" << msg.brief());
   // This is called when we are waiting to resend a INVITE with no sdp after a glare condition, and we 
   // instead receive an inbound INVITE or UPDATE
}

void
UaClientCall::onRemoteSdpChanged(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
    //cout << "***************  ******************* 17\n" << msg << endl;
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
    //cout << "***************  ******************* 18\n" << msg << endl;
   InfoLog(<< "onInfo: msg=" << msg.brief());

   // Handle message here
   h->acceptNIT();
}

void
UaClientCall::onInfoSuccess(InviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 19\n" << msg << endl;
   InfoLog(<< "onInfoSuccess: msg=" << msg.brief());
}

void
UaClientCall::onInfoFailure(InviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 20\n" << msg << endl;
   WarningLog(<< "onInfoFailure: msg=" << msg.brief());
}

void
UaClientCall::onRefer(InviteSessionHandle h, ServerSubscriptionHandle ss, const SipMessage& msg)
{
    //cout << "***************  ******************* 21\n" << msg << endl;
   InfoLog(<< "onRefer: msg=" << msg.brief());

   // Handle Refer request here
}

void
UaClientCall::onReferAccepted(InviteSessionHandle h, ClientSubscriptionHandle csh, const SipMessage& msg)
{
    //cout << "***************  ******************* 22\n" << msg << endl;
   InfoLog(<< "onReferAccepted: msg=" << msg.brief());
}
void UaClientCall::onAckReceived(InviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 23\n" << msg << endl;
    InfoLog(<< "onOffer: " << msg);

    /*cout << "*************** onReferAccepted ***************************\n"
        << msg
        << "***********************************************\n" << endl;*/
}
void
UaClientCall::onReferRejected(InviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 24\n" << msg << endl;
   WarningLog(<< "onReferRejected: msg=" << msg.brief());
}

void
UaClientCall::onReferNoSub(InviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 25\n" << msg << endl;
   InfoLog(<< "onReferNoSub: msg=" << msg.brief());

   // Handle Refer request with (no-subscription indication) here
}

void
UaClientCall::onMessage(InviteSessionHandle h, const SipMessage& msg)
{
    //cout << "***************  ******************* 26\n" << msg << endl;
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
    //cout << "***************  ******************* 27\n" << msg << endl;
    mTimerExpiredCounter = 0;
    mSessionState = msg.header(h_StatusLine).statusCode();
   InfoLog(<< "onMessageSuccess: msg=" << msg.brief());
}

void
UaClientCall::onMessageFailure(InviteSessionHandle h, const SipMessage& msg)
{
    mSessionState = msg.header(h_StatusLine).statusCode();
    //cout << "***************  ******************* 28\n" << msg << endl;
    if (mSessionState == 408)
    {
        mTimerExpiredCounter++;
    }
    else
    {
        mTimerExpiredCounter = 0;
    }
   WarningLog(<< "onMessageFailure: msg=" << msg.brief());
}

void
UaClientCall::onForkDestroyed(ClientInviteSessionHandle h)
{
    //cout << "***************  ******************* 29\n" << endl;
   InfoLog(<< "onForkDestroyed:");
}

void 
UaClientCall::onReadyToSend(InviteSessionHandle h, SipMessage& msg)
{
    Data msgData;
    {
        oDataStream ds(msgData);
        ds << msg;
        ds.flush();
    }
    if (isUACConnected())
    {
        if (msg.isRequest() && msg.method() == ACK)
        {
            msg.header(h_RequestLine).uri().host() = devIp.c_str();
            msg.header(h_RequestLine).uri().port() = devPort;
            msg.header(h_Contacts).front().uri() = msg.header(h_RequestLine).uri();
        }
    }
    else
    {

    }
    
    //cout << "***************  ******************* 30\n" << msgData << endl;
}

void 
UaClientCall::onFlowTerminated(InviteSessionHandle h)
{
    //cout << "***************  ******************* 31\n" << endl;
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

   //ServerInviteSession* sis = (ServerInviteSession*)(h->getInviteSession().get());
   //sis->provisional(180);
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
void UaClientCall::createRtpServer(short localport, const char* peerIp, short peerPort)
{
    rtp_ctx = new rtp_receiver(localport, peerIp, peerPort);
    if (rtp_ctx)
    {
        rtp_ctx->initRtpContext(0, 96, "PS");
        rtp_ctx->run();
    }
    //asio::io_service io_service;
    //asio::ip::udp::endpoint rtp_endpoint(asio::ip::address_v4::from_string("192.168.1.230"), port);//create  a local endpoint
    //asio::ip::udp::endpoint rtcp_endpoint(asio::ip::address_v4::from_string("192.168.1.230"), port+1);

    //asio::ip::udp::endpoint romote_endpoint; //this enpoint is used to store the endponit from remote-computer

    //asio::ip::udp::socket rtp_socket(io_service, rtp_endpoint);//create socket and bind the endpoint

    //asio::ip::udp::socket rtcp_socket(io_service, rtcp_endpoint);

    //char buffer[40000];

    //int nAdd = 0;

    //while (1)
    //{
    //    memset(buffer, 0, 40000);//to initialize variables
    //    nAdd++;
    //    rtcp_socket.receive_from(asio::buffer(buffer, 40000), romote_endpoint);

    //    rtp_socket.receive_from(asio::buffer(buffer, 40000), romote_endpoint);//receive data from  remote-computer
    //    printf("recv %d datapacket:%s\n", nAdd, buffer);
    //}

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
RequestStreamTask::RequestStreamTask(std::string dId, std::string dIp, int dPort, std::string channelId, UaMgr& userAgent, int iRtpPort, int rtptype, UaClientCall* pAlegcall,
    std::string sesName, unsigned long stime, unsigned long etime)
    :devId(dId), devIp(dIp), devPort(dPort), channelId(channelId), mUserAgent(userAgent), rtpPort(iRtpPort), rtpType(rtptype), pmAlegCall(pAlegcall)
    , sessionName(sesName), startTime(stime), stopTime(etime)
{
    if (sessionName == "Play")
    {
        streamId = channelId;// std::str_format("%s_%s", devId.c_str(), channelId.c_str());
    }
    else
    {
        CDateTime staTime(stime);
        CDateTime endTime(etime);
        //streamId = std::str_format("%s_%s_%s_%s", devId.c_str(), channelId.c_str(), staTime.tmFormat("%Y-%m-%dT%H:%M:%S").c_str(), endTime.tmFormat("%Y-%m-%dT%H:%M:%S").c_str());
    }
}
bool RequestStreamTask::TaskRun()
{
    if (pmAlegCall)
    {
        if (pmAlegCall->sessionName == Data("Play"))
        {
            if (mUserAgent.getStreamStatus(streamId) == resip::UaMgr::streamStatus::_UERAGERNT_NOT_STREAM)
            {
                if (mUserAgent.RequestLiveStream(devId, devIp, devPort, channelId, 0, rtpPort, rtpType, pmAlegCall))
                {
                    UaClientCall::CALL_STATE state = UaClientCall::CALL_UAC_RES_START;
                    while (state != UaClientCall::CALL_NOT_FOUND && state != UaClientCall::CALL_MEDIA_STREAM_CLOSED)
                    {
                        state = (UaClientCall::CALL_STATE)mUserAgent.getCallStatus(streamId);
                        switch (state)
                        {
                        case UaClientCall::CALL_UAC_FAILURE:
                        {
                            if (pmAlegCall)
                            {
                                if (pmAlegCall->getInviteSessionHandle().isValid())
                                {
                                    WarningCategory warning;
                                    warning.hostname() = DnsUtil::getLocalIpAddress();;
                                    warning.code() = 488;
                                    warning.text() = "play b leg invite failure";
                                    pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                                }
                            }
                            return true;
                        }
                        case UaClientCall::CALL_UAC_TERMINATED:
                        {
                            if (pmAlegCall)
                            {
                                if (pmAlegCall->getInviteSessionHandle().isValid())
                                {
                                    WarningCategory warning;
                                    warning.hostname() = DnsUtil::getLocalIpAddress();;
                                    warning.code() = 488;
                                    warning.text() = "play b leg invite time out";
                                    pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                                }
                            }
                            return true;
                        }
                        case UaClientCall::CALL_MY_MEDIA_OK:
                        case UaClientCall::CALL_MEDIA_READY:
                        case UaClientCall::CALL_UAC_CONNECTED:
                        {
                            if (pmAlegCall && pmAlegCall->getInviteSessionHandle().isValid())
                            {
                                auto dev = mUserAgent.GetStreamInfo(streamId);
                                if (dev)
                                {
                                    /*pmAlegCall->pStreamInfo = dynamic_cast<JsonDevice*>(dev);
                                    auto transport = std::make_shared<RTPUdpTransport>();
                                    unsigned short localport[2] = { pmAlegCall->myRtpPort, pmAlegCall->myRtpPort + 1 };
                                    unsigned short peerport[2] = { pmAlegCall->remotePort, pmAlegCall->remotePort + 1 };
                                    if (0 != transport->Init(localport, pmAlegCall->remoteIp.c_str(), peerport))
                                    {
                                        dev->psSource = new PSFileSource("v5_34.264", pmAlegCall->ssrc.convertInt());
                                        dev->psSource->SetTransport("sip", transport);
                                    }*/
                                }
                                pmAlegCall->getInviteSessionHandle()->provideAnswer(pmAlegCall->mResponseSdp);
                                ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(pmAlegCall->getInviteSessionHandle().get());
                                if (uas && !uas->isAccepted())
                                {
                                    uas->accept();
                                }
                            }
                            return true;
                        }
                        case UaClientCall::CALL_MEDIA_ERROR:
                        {
                            if (pmAlegCall)
                            {
                                if (pmAlegCall->getInviteSessionHandle().isValid())
                                {
                                    WarningCategory warning;
                                    warning.hostname() = DnsUtil::getLocalIpAddress();;
                                    warning.code() = 488;
                                    warning.text() = "play b leg media error";
                                    pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                                }
                            }
                            return true;
                        }
                        case UaClientCall::CALL_MEDIA_TIMEOUT:
                        {
                            if (pmAlegCall)
                            {
                                if (pmAlegCall->getInviteSessionHandle().isValid())
                                {
                                    WarningCategory warning;
                                    warning.hostname() = DnsUtil::getLocalIpAddress();;
                                    warning.code() = 488;
                                    warning.text() = "play b leg media timeout";
                                    pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                                }
                            }
                            return true;
                        }
                        default:
                            break;
                        }
                    }
                }
            }
            else if (mUserAgent.getStreamStatus(streamId) == resip::UaMgr::streamStatus::_UERAGERNT_STREAM_OK)
            {
                if (pmAlegCall && pmAlegCall->getInviteSessionHandle().isValid())
                {
                    pmAlegCall->getInviteSessionHandle()->provideAnswer(pmAlegCall->mResponseSdp);
                    ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(pmAlegCall->getInviteSessionHandle().get());
                    if (uas && !uas->isAccepted())
                    {
                        uas->accept();
                    }
                }
            }
            else
            {
                UaClientCall::CALL_STATE state = UaClientCall::CALL_UAC_RES_START;
                while (state != UaClientCall::CALL_NOT_FOUND && state != UaClientCall::CALL_MEDIA_STREAM_CLOSED)
                {
                    state = (UaClientCall::CALL_STATE)mUserAgent.getCallStatus(streamId);
                    switch (state)
                    {
                    case UaClientCall::CALL_UAC_FAILURE:
                    {
                        if (pmAlegCall)
                        {
                            if (pmAlegCall->getInviteSessionHandle().isValid())
                            {
                                WarningCategory warning;
                                warning.hostname() = DnsUtil::getLocalIpAddress();;
                                warning.code() = 488;
                                warning.text() = "play b leg invite failure";
                                pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                            }
                        }
                        return true;
                    }
                    case UaClientCall::CALL_UAC_TERMINATED:
                    {
                        if (pmAlegCall)
                        {
                            if (pmAlegCall->getInviteSessionHandle().isValid())
                            {
                                WarningCategory warning;
                                warning.hostname() = DnsUtil::getLocalIpAddress();;
                                warning.code() = 488;
                                warning.text() = "play b leg invite time out";
                                pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                            }
                        }
                        return true;
                    }
                    case UaClientCall::CALL_MY_MEDIA_OK:
                    case UaClientCall::CALL_MEDIA_READY:
                    case UaClientCall::CALL_UAC_CONNECTED:
                    {
                        if (pmAlegCall && pmAlegCall->getInviteSessionHandle().isValid())
                        {
                            pmAlegCall->getInviteSessionHandle()->provideAnswer(pmAlegCall->mResponseSdp);
                            ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(pmAlegCall->getInviteSessionHandle().get());
                            if (uas && !uas->isAccepted())
                            {
                                uas->accept();
                            }
                        }
                        return true;
                    }
                    case UaClientCall::CALL_MEDIA_ERROR:
                    {
                        if (pmAlegCall)
                        {
                            if (pmAlegCall->getInviteSessionHandle().isValid())
                            {
                                WarningCategory warning;
                                warning.hostname() = DnsUtil::getLocalIpAddress();;
                                warning.code() = 488;
                                warning.text() = "play b leg media error";
                                pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                            }
                        }
                        return true;
                    }
                    case UaClientCall::CALL_MEDIA_TIMEOUT:
                    {
                        if (pmAlegCall)
                        {
                            if (pmAlegCall->getInviteSessionHandle().isValid())
                            {
                                WarningCategory warning;
                                warning.hostname() = DnsUtil::getLocalIpAddress();;
                                warning.code() = 488;
                                warning.text() = "play b leg media timeout";
                                pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                            }
                        }
                        return true;
                    }
                    default:
                        break;
                    }
                }
            }
        }
        else if (pmAlegCall->sessionName == Data("Playback"))
        {
            if (mUserAgent.RequestVodStream(devId, devIp, devPort, channelId, streamId, rtpPort, rtpType, startTime, stopTime))
            {
                UaClientCall::CALL_STATE state = UaClientCall::CALL_UAC_RES_START;
                while (state != UaClientCall::CALL_NOT_FOUND && state != UaClientCall::CALL_MEDIA_STREAM_CLOSED)
                {
                    state = (UaClientCall::CALL_STATE)mUserAgent.getCallStatus(streamId);
                    switch (state)
                    {
                    case UaClientCall::CALL_UAC_FAILURE:
                    {
                        if (pmAlegCall)
                        {
                            if (pmAlegCall->getInviteSessionHandle().isValid())
                            {
                                WarningCategory warning;
                                warning.hostname() = DnsUtil::getLocalIpAddress();;
                                warning.code() = 488;
                                warning.text() = "play b leg invite failure";
                                pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                            }
                        }
                        return true;
                    }
                    case UaClientCall::CALL_UAC_TERMINATED:
                    {
                        if (pmAlegCall)
                        {
                            if (pmAlegCall->getInviteSessionHandle().isValid())
                            {
                                WarningCategory warning;
                                warning.hostname() = DnsUtil::getLocalIpAddress();;
                                warning.code() = 488;
                                warning.text() = "play b leg invite time out";
                                pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                            }
                        }
                        return true;
                    }
                    case UaClientCall::CALL_MY_MEDIA_OK:
                    case UaClientCall::CALL_MEDIA_READY:
                    case UaClientCall::CALL_UAC_CONNECTED:
                    {
                        if (pmAlegCall && pmAlegCall->getInviteSessionHandle().isValid())
                        {
                            pmAlegCall->getInviteSessionHandle()->provideAnswer(pmAlegCall->mResponseSdp);
                            ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(pmAlegCall->getInviteSessionHandle().get());
                            if (uas && !uas->isAccepted())
                            {
                                uas->accept();
                            }
                        }
                        return true;
                    }
                    case UaClientCall::CALL_MEDIA_ERROR:
                    {
                        if (pmAlegCall)
                        {
                            if (pmAlegCall->getInviteSessionHandle().isValid())
                            {
                                WarningCategory warning;
                                warning.hostname() = DnsUtil::getLocalIpAddress();;
                                warning.code() = 488;
                                warning.text() = "play b leg media error";
                                pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                            }
                        }
                        return true;
                    }
                    case UaClientCall::CALL_MEDIA_TIMEOUT:
                    {
                        if (pmAlegCall)
                        {
                            if (pmAlegCall->getInviteSessionHandle().isValid())
                            {
                                WarningCategory warning;
                                warning.hostname() = DnsUtil::getLocalIpAddress();;
                                warning.code() = 488;
                                warning.text() = "play b leg media timeout";
                                pmAlegCall->getInviteSessionHandle()->reject(488, &warning);
                            }
                        }
                        return true;
                    }
                    default:
                        break;
                    }
                }
            }
        }
        else if (pmAlegCall->sessionName == Data("Download"))
        {
        }
    }
    else
    {
        if (sessionName == "Play")
        {
            if (mUserAgent.getStreamStatus(streamId) == resip::UaMgr::streamStatus::_UERAGERNT_NOT_STREAM)
            {
                if (mUserAgent.RequestLiveStream(devId, devIp, devPort, channelId, 0, rtpPort, rtpType))
                {
                }
            }
            else if (mUserAgent.getStreamStatus(streamId) == resip::UaMgr::streamStatus::_UERAGERNT_STREAM_OK)
            {
                if (pmAlegCall && pmAlegCall->getInviteSessionHandle().isValid())
                {
                    pmAlegCall->getInviteSessionHandle()->provideAnswer(pmAlegCall->mResponseSdp);
                    ServerInviteSession* uas = dynamic_cast<ServerInviteSession*>(pmAlegCall->getInviteSessionHandle().get());
                    if (uas && !uas->isAccepted())
                    {
                        uas->accept();
                    }
                }
            }
            else
            {
                
            }
        }
        else if (sessionName == "Playback")
        {
            if (mUserAgent.RequestVodStream(devId, devIp, devPort, channelId, streamId, rtpPort, rtpType, startTime, stopTime))
            {
                
            }
        }
        else if (sessionName == "Download")
        {
        }
    }
  
    return true;
};

bool PushRtpStream::TaskRun()
{
    bool isStream = false;
    std::string app;
    std::string streamId;
    rapidjson_sip::Document document;
    //sipserver::SipServer* pSvr = GetServer();
    //for (int i = 0; i < 2 * 5; i++)
    //{
    //    ostringstream ss;
    //    ss << "http://" << (pSvr?pSvr->zlmHost:"127.0.0.1") <<":" << (pSvr?pSvr->zlmHttpPort:8080) << "/index/api/isMediaOnline?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&"
    //        << "schema=rtsp&vhost=__defaultVhost__&app=rtp&stream="
    //        << stream_Id;
    //    ss.flush();
    //    std::string strResponse = GetRequest(ss.str());
    //    document.Parse((char*)strResponse.c_str());
    //    if (!document.HasParseError())
    //    {
    //        if (document.HasMember("online"))
    //        {
    //            if (document["online"].IsBool())
    //            {
    //                isStream = document["online"].GetBool();
    //                if (isStream)
    //                    break;
    //            }
    //            else
    //            {
    //                cout << "online not bool type" << endl;
    //            }
    //        }
    //    }
    //    //"{{ZLMediaKit_URL}}/index/api/isMediaOnline?secret={{ZLMediaKit_secret}}&schema=rtsp&vhost={{defaultVhost}}&app=rtp&stream=34020000001180000800_34020000001320000014"
    //    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //}
    if (isStream)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ostringstream ss;
        /*ss << "http://" << (pSvr?pSvr->zlmHost:"127.0.0.1") << ":" << (pSvr?pSvr->zlmHttpPort:8080) << "/index/api/startSendRtp?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&vhost=__defaultVhost__&app="
            << "rtp" << "&stream=" << devId << "_" << channelId
            << "&ssrc=" << strSsrc << "&dst_url=" << strDstIp << "&dst_port=" << connectPort
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
        }*/
    }
    else
    {
        cout << "getMediaList error streamid:" << stream_Id << endl;
    }
    return true;
};