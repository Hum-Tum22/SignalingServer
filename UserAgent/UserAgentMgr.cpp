#include "UserAgentMgr.h"

#include <rutil/Log.hxx>
#include <rutil/Logger.hxx>
#include <rutil/DnsUtil.hxx>
#include <rutil/MD5Stream.hxx>
#include <rutil/FdPoll.hxx>
#include <resip/stack/SdpContents.hxx>
#include <resip/stack/PlainContents.hxx>
#include <resip/stack/ConnectionTerminated.hxx>
#include <resip/stack/Helper.hxx>
#include <resip/dum/AppDialogSetFactory.hxx>
#include <resip/dum/ClientAuthManager.hxx>
#include <resip/dum/KeepAliveManager.hxx>
#include <resip/dum/ClientInviteSession.hxx>
#include <resip/dum/ServerInviteSession.hxx>
#include <resip/dum/ClientSubscription.hxx>
#include <resip/dum/ServerSubscription.hxx>
#include <resip/dum/ClientRegistration.hxx>
#include <resip/dum/ServerRegistration.hxx>
#include <resip/dum/ServerOutOfDialogReq.hxx>
#include <rutil/dns/AresDns.hxx>
#include "resip/dum/DumThread.hxx"

#if defined (USE_SSL)
#if defined(WIN32) 
#include "resip/stack/ssl/WinSecurity.hxx"
#else
#include "resip/stack/ssl/Security.hxx"
#endif
#endif

#include "UaClientCall.h"
#include "../XmlMsgAnalysis.h"
#include "../MsgContentXml.h"
#include "UaMessageMgr.h"
#include "../device/DeviceManager.h"
#include "../deviceMng/deviceMng.h"
#include "../SipServerConfig.h"
#include "../SipServer.h"
#include "../tools/m_Time.h"

#include "../deviceMng/JsonDevice.h"
#include "../media/MediaMng.h"
#include "../media/mediaIn/JsonStream.h"

#include <memory>

#include "myjsondef.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"

using namespace resip;
using namespace std;
#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

static unsigned int MaxRegistrationRetryTime = 1800;              // RFC5626 section 4.5 default
static unsigned int BaseRegistrationRetryTimeAllFlowsFailed = 120; // RFC5626 section 4.5 default
//static unsigned int BaseRegistrationRetryTime = 90;               // RFC5626 section 4.5 default
static unsigned int NotifySendTime = 30;  // If someone subscribes to our test event package, then send notifies every 30 seconds
static unsigned int FailedSubscriptionRetryTime = 60;

namespace resip
{
class ClientAppDialogSetFactory : public AppDialogSetFactory
{
public:
    ClientAppDialogSetFactory(UaMgr& ua) : mUserAgent(ua) {}
    resip::AppDialogSet* createAppDialogSet(DialogUsageManager& dum, const SipMessage& msg)
    {
        Data command;
        {
            DataStream ds(command);
            ds << msg;
        }
        int method = msg.method();
        if (method == REGISTER)
        {
            printf("");
        }
        else if (method == MESSAGE)
        {
            printf("");
        }
        if (msg.isRequest())
        {
            printf("");
        }
        else if (msg.isResponse())
        {
            printf("");
        }
        switch (msg.method())
        {
        case INVITE:
            return  new UaClientCall(mUserAgent);
            break;
        default:
            return AppDialogSetFactory::createAppDialogSet(dum, msg);
            break;
        }
}
private:
    UaMgr& mUserAgent;
};

// Used to set the IP Address in outbound SDP to match the IP address choosen by the stack to send the message on
class SdpMessageDecorator : public MessageDecorator
{
public:
    virtual ~SdpMessageDecorator() {}
    virtual void decorateMessage(SipMessage& msg,
        const Tuple& source,
        const Tuple& destination,
        const Data& sigcompId)
    {
        if (msg.method() == INVITE)
        {
            SdpContents* sdp = dynamic_cast<SdpContents*>(msg.getContents());
            if (sdp)
            {
                // Fill in IP and Port from source
                //sdp->session().connection().setAddress(Tuple::inet_ntop(source), source.ipVersion() == V6 ? SdpContents::IP6 : SdpContents::IP4);
                //sdp->session().origin().setAddress(Tuple::inet_ntop(source), source.ipVersion() == V6 ? SdpContents::IP6 : SdpContents::IP4);
                sipserver::SipServer* pSvr = GetServer();
                if (msg.isRequest())
                {
                    if (pSvr)
                    {
                        pSvr->zlmHost = "192.168.1.230";
                        sdp->session().connection().setAddress(pSvr->zlmHost.c_str());
                        sdp->session().origin().setAddress(pSvr->zlmHost.c_str());
                    }
                }
                else if (msg.isResponse())
                {
                    if (pSvr)
                    {
                        sdp->session().connection().setAddress(pSvr->zlmHost.c_str());
                        sdp->session().origin().setAddress(pSvr->zlmHost.c_str());
                    }
                }
                Data sourceip = Tuple::inet_ntop(source);
                Data destip = Tuple::inet_ntop(destination);
            }
        }
        else if (msg.method() == REGISTER)
        {
            Data destip = Tuple::inet_ntop(destination);
        }
        else if (msg.method() == MESSAGE)
        {
            Data destip = Tuple::inet_ntop(destination);
            if (destip == "192.168.1.230")
            {
                cout << destip << endl;
            }
        }
        else
        {
            Data destip = Tuple::inet_ntop(destination);
        }
        
        InfoLog(<< "SdpMessageDecorator: src=" << source << ", dest=" << destination << ", msg=" << endl << msg.brief() << ": " << msg);
    }
    virtual void rollbackMessage(SipMessage& msg) {}  // Nothing to do
    virtual MessageDecorator* clone() const { return new SdpMessageDecorator; }
};

class NotifyTimer : public resip::DumCommand
{
public:
    NotifyTimer(UaMgr& userAgent, unsigned int timerId) : mUserAgent(userAgent), mTimerId(timerId) {}
    NotifyTimer(const NotifyTimer& rhs) : mUserAgent(rhs.mUserAgent), mTimerId(rhs.mTimerId) {}
    ~NotifyTimer() {}

    void executeCommand() { mUserAgent.onNotifyTimeout(mTimerId); }

    resip::Message* clone() const { return new NotifyTimer(*this); }
    EncodeStream& encode(EncodeStream& strm) const { strm << "NotifyTimer: id=" << mTimerId; return strm; }
    EncodeStream& encodeBrief(EncodeStream& strm) const { return encode(strm); }

private:
    UaMgr& mUserAgent;
    unsigned int mTimerId;
};
};


UaMgr::UaMgr(SipStack& stack, int iRtpPortRangeMin, int iRtpPortRangeMax):
mProfile(new MasterProfile),
mStack(stack),
mDum(new DialogUsageManager(mStack)),
mDumShutdownRequested(false),
mShuttingdown(false),
mDumShutdown(false),
mRegistrationRetryDelayTime(0),
mCurrentNotifyTimerId(0),
//mRegHandle(NULL),
mMessageMgr(NULL),
mDumThread(NULL),
mRtpPortMngr(iRtpPortRangeMin, iRtpPortRangeMax)
{
    //Log::initialize(Log::Cout, Log::Stack, "gb28181");

    if (mHostFileLookupOnlyDnsMode)
    {
        AresDns::enableHostFileLookupOnlyMode(true);
    }

    // Disable Statistics Manager
    mStack.statisticsManagerEnabled() = false;

    // Supported Methods
    mProfile->clearSupportedMethods();
    mProfile->addSupportedMethod(INVITE);
    mProfile->addSupportedMethod(ACK);
    mProfile->addSupportedMethod(CANCEL);
    mProfile->addSupportedMethod(OPTIONS);
    mProfile->addSupportedMethod(BYE);
    //mProfile->addSupportedMethod(REFER);
    mProfile->addSupportedMethod(NOTIFY);
    mProfile->addSupportedMethod(SUBSCRIBE);
    //mProfile->addSupportedMethod(UPDATE);
    mProfile->addSupportedMethod(INFO);
    mProfile->addSupportedMethod(MESSAGE);
    mProfile->addSupportedMethod(PRACK);
    //mProfile->addSupportedOptionTag(Token(Symbols::C100rel));  // Automatically added when using setUacReliableProvisionalMode
    mProfile->setUacReliableProvisionalMode(MasterProfile::Supported);
    mProfile->setUasReliableProvisionalMode(MasterProfile::SupportedEssential);

    // Support Languages
    mProfile->clearSupportedLanguages();
    mProfile->addSupportedLanguage(Token("en"));

    // Support Mime Types
    mProfile->clearSupportedMimeTypes();
    mProfile->addSupportedMimeType(INVITE, Mime("application", "sdp"));
    mProfile->addSupportedMimeType(INVITE, Mime("multipart", "mixed"));
    mProfile->addSupportedMimeType(INVITE, Mime("multipart", "signed"));
    mProfile->addSupportedMimeType(INVITE, Mime("multipart", "alternative"));
    mProfile->addSupportedMimeType(OPTIONS, Mime("application", "sdp"));
    mProfile->addSupportedMimeType(OPTIONS, Mime("multipart", "mixed"));
    mProfile->addSupportedMimeType(OPTIONS, Mime("multipart", "signed"));
    mProfile->addSupportedMimeType(OPTIONS, Mime("multipart", "alternative"));
    mProfile->addSupportedMimeType(PRACK, Mime("application", "sdp"));
    mProfile->addSupportedMimeType(PRACK, Mime("multipart", "mixed"));
    mProfile->addSupportedMimeType(PRACK, Mime("multipart", "signed"));
    mProfile->addSupportedMimeType(PRACK, Mime("multipart", "alternative"));
    mProfile->addSupportedMimeType(UPDATE, Mime("application", "sdp"));
    mProfile->addSupportedMimeType(UPDATE, Mime("multipart", "mixed"));
    mProfile->addSupportedMimeType(UPDATE, Mime("multipart", "signed"));
    mProfile->addSupportedMimeType(UPDATE, Mime("multipart", "alternative"));

    mProfile->addSupportedMimeType(SUBSCRIBE, Mime("application", "MANSCDP+xml"));
    mProfile->addSupportedMimeType(NOTIFY, Mime("application", "MANSCDP+xml"));
    mProfile->addSupportedMimeType(MESSAGE, Mime("application", "MANSCDP+xml"));

    //mProfile->addSupportedMimeType(MESSAGE, Mime("text", "plain")); // Invite session in-dialog routing testing
    //mProfile->addSupportedMimeType(NOTIFY, Mime("text", "plain"));  // subscription testing
    //mProfile->addSupportedMimeType(NOTIFY, Mime("message", "sipfrag"));  

    // Supported Options Tags
    mProfile->clearSupportedOptionTags();
    //mMasterProfile->addSupportedOptionTag(Token(Symbols::Replaces));      
    mProfile->addSupportedOptionTag(Token(Symbols::Timer));     // Enable Session Timers
    if (mOutboundEnabled)
    {
        mProfile->addSupportedOptionTag(Token(Symbols::Outbound));  // RFC 5626 - outbound
        mProfile->addSupportedOptionTag(Token(Symbols::Path));      // RFC 3327 - path
    }
    //mMasterProfile->addSupportedOptionTag(Token(Symbols::NoReferSub));
    //mMasterProfile->addSupportedOptionTag(Token(Symbols::TargetDialog));

    // Supported Schemes
    mProfile->clearSupportedSchemes();
    mProfile->addSupportedScheme("sip");
#if defined(USE_SSL)
    mProfile->addSupportedScheme("sips");
#endif

    // Validation Settings
    mProfile->validateContentEnabled() = false;
    mProfile->validateContentLanguageEnabled() = false;
    mProfile->validateAcceptEnabled() = false;

    // Have stack add Allow/Supported/Accept headers to INVITE dialog establishment messages
    mProfile->clearAdvertisedCapabilities(); // Remove Profile Defaults, then add our preferences
    mProfile->addAdvertisedCapability(Headers::Allow);
    //mProfile->addAdvertisedCapability(Headers::AcceptEncoding);  // This can be misleading - it might specify what is expected in response
    mProfile->addAdvertisedCapability(Headers::AcceptLanguage);
    mProfile->addAdvertisedCapability(Headers::Supported);
    mProfile->setMethodsParamEnabled(true);

   // Install Sdp Message Decorator
    mProfile->setOutboundDecorator(std::make_shared<SdpMessageDecorator>());

    // Other Profile Settings
    mProfile->setUserAgent("basicClient/1.0");
    mProfile->setDefaultRegistrationTime(mRegisterDuration);
    mProfile->setDefaultRegistrationRetryTime(120);

    MyServerConfig& svrCfgi = GetSipServerConfig();
    Data conTactUriData(svrCfgi.getConfigData("ContactUri", "", true));
    if (!conTactUriData.empty())
    {
        mContact = Uri(conTactUriData);
    }
    if (!mContact.host().empty())
    {
        mProfile->setOverrideHostAndPort(mContact);
    }
    if (!mOutboundProxy.host().empty())
    {
        mProfile->setOutboundProxy(Uri(mOutboundProxy));
        //mProfile->setForceOutboundProxyOnAllRequestsEnabled(true);
        mProfile->setExpressOutboundAsRouteSetEnabled(true);
    }

    // UserProfile Settings
    //Uri mFrom("sip:34020000002000000001@192.168.1.230:5060");
    Uri defaultFrom;
    defaultFrom.user() = svrCfgi.getConfigData("GBID", "34020000002000000001", true);
    defaultFrom.host() = DnsUtil::getLocalIpAddress();
    defaultFrom.port() = svrCfgi.getConfigInt("UDPPort", 5060);
    mProfile->setDefaultFrom(NameAddr(defaultFrom));

    // Generate InstanceId appropriate for testing only.  Should be UUID that persists 
    // across machine re-starts and is unique to this applicaiton instance.  The one used 
    // here is only as unique as the hostname of this machine.  If someone runs two 
    // instances of this application on the same host for the same Aor, then things will 
    // break.  See RFC5626 section 4.1
    Data hostname = DnsUtil::getLocalHostName();
    Data instanceHash = hostname.md5().uppercase();
    assert(instanceHash.size() == 32);
    Data instanceId(48, Data::Preallocate);
    instanceId += "<urn:uuid:";
    instanceId += instanceHash.substr(0, 8);
    instanceId += "-";
    instanceId += instanceHash.substr(8, 4);
    instanceId += "-";
    instanceId += instanceHash.substr(12, 4);
    instanceId += "-";
    instanceId += instanceHash.substr(16, 4);
    instanceId += "-";
    instanceId += instanceHash.substr(20, 12);
    instanceId += ">";
    mProfile->setInstanceId(instanceId);
    if (mOutboundEnabled)
    {
        mProfile->setRegId(1);
        mProfile->clientOutboundEnabled() = true;
    }

    resip::Timer::TcpConnectTimeout = 10000;

    // Install Managers
    mDum->setClientAuthManager(std::unique_ptr<ClientAuthManager>(new ClientAuthManager));
    mDum->setKeepAliveManager(std::unique_ptr<KeepAliveManager>(new KeepAliveManager));
    mProfile->setKeepAliveTimeForDatagram(30);
    mProfile->setKeepAliveTimeForStream(120);

    // Install Handlers
    mDum->setInviteSessionHandler(this);
    mDum->setDialogSetHandler(this);
    mDum->addOutOfDialogHandler(OPTIONS, this);
    ////mDum->addOutOfDialogHandler(REFER, this);
    mDum->setRedirectHandler(this);
    
    mDum->setClientRegistrationHandler(this);
    mDum->addClientSubscriptionHandler("basicClientTest", this);   // fabricated test event package
    mDum->addServerSubscriptionHandler("basicClientTest", this);
    list<Data> eventlist;
    eventlist.push_back(Data("catalagitem"));
    eventlist.push_back(Data("Catalog"));
    eventlist.push_back(Data("e_Alarm"));
    eventlist.push_back(Data("Alarm"));
    eventlist.push_back(Data("MobilePosition"));
    eventlist.push_back(Data("presence"));
    for (auto iter: eventlist)
    {
        mDum->addClientSubscriptionHandler(iter, this);
        mDum->addServerSubscriptionHandler(iter, this);
        ServerSubscriptionInfos svsubinfos;
        m_SvSubmap[iter] = svsubinfos;
    }

    // Set AppDialogSetFactory
   std::unique_ptr<AppDialogSetFactory> dsf(new ClientAppDialogSetFactory(*this));
    mDum->setAppDialogSetFactory(std::move(dsf));

    mDum->setMasterProfile(mProfile);

    mDum->registerForConnectionTermination(this);

    /*mRegHandle = new CUacRegistHandler(*mDum);
    if (mRegHandle)
        mRegHandle->RegistCallBackRegistState(RegistStateCallBack, this);*/
    mMessageMgr = new CUserMessageMrg(*mDum);
    if (mMessageMgr)
    {
        mMessageMgr->RegistPageMsgCallBack(RegistPageMsgCallBack, this);
        mMessageMgr->RegistArrivedMsgCallBack(RegistArrivedMsgCallBack, this);
    }

    mDumThread = new DumThread(*mDum);

    MyServerConfig& SvrCfg = GetSipServerConfig();
    mDevCfg.HeartBeatCount = SvrCfg.getConfigInt("keepaliveTimeOutNum", 3);
    mDevCfg.HeartBeatInterval = SvrCfg.getConfigInt("keepaliveInterval", 3);
    mDevCfg.Expires = SvrCfg.getConfigInt("Expires", 3);
    StateThread = std::thread(checkStateThread, this);
}
UaMgr::~UaMgr()
{
    if (mMessageMgr)
    {
        delete mMessageMgr; mMessageMgr = NULL;
    }
    delete mDum;
}

void UaMgr::DoRegist(const Uri& target, const Uri& fromUri, const Data& passwd)
{
    if (mDum == NULL)
        return;
    shared_ptr<UaSessionInfo> ua = GetUaInfoByUser(target.user());
    if (!ua)
    {
        shared_ptr<UaSessionInfo> uaSession(new UaSessionInfo(target, mProfile->getDefaultFrom().uri(), passwd));
        Regist(uaSession);
        CUSTORLOCKGUARD locker(mapMtx);
        UserAgentInfoMap[target.user()] = uaSession;
    }
}
void UaMgr::Regist(shared_ptr<UaSessionInfo> ua)
{
    if (mDum == NULL)
        return;
    shared_ptr<SipMessage> regMessage = mDum->makeRegistration(NameAddr(ua->toUri), mDevCfg.Expires);
    {
#ifdef TEST_PASSING_A1_HASH_FOR_PASSWORD
        MD5Stream a1;
        a1 << mAor.user()
            << Symbols::COLON
            << mAor.host()
            << Symbols::COLON
            << ua->passwd;
        mDum->getMasterProfile()->setDigestCredential(ua->toUri.host(), ua->fromUri.user(), a1.getHex(), true);
#else
        mDum->getMasterProfile()->setDigestCredential(/*ua->toUri.host()*/
            mProfile->getDefaultFrom().uri().host(), mProfile->getDefaultFrom().uri().user(), ua->passwd);
#endif
        Uri tUri = ua->toUri;
        tUri.host() = tUri.user().substr(0, 10);
        regMessage->header(h_RequestLine).uri().user() = tUri.user();// Uri(tUri.getAorNoReally());
        Uri tmpUri = ua->fromUri;
        tmpUri.host() = tmpUri.user().substr(0, 10);
        regMessage->header(h_To).uri() = Uri(tmpUri.getAorNoReally());
        regMessage->header(h_From).uri() = Uri(tmpUri.getAorNoReally());
        regMessage->header(h_Contacts).front().uri().user() = ua->fromUri.user();
        regMessage->header(h_Expires).value() = mDevCfg.Expires;
        //regMessage->setForceTarget(ua->toUri);
        //regMessage->header(h_Routes).push_back(NameAddr(ua->toUri));
        //regMessage->header(h_Routes).push_front(NameAddr(ua->toUri));
        ua->regcallid = regMessage->header(h_CallID).value();
        ua->heartTimeOutCount = 0;
        cout << "**************************************\n"
            << *regMessage
            << "\n***********************************\n" << endl;
    }
    mDum->send(regMessage);
}
void UaMgr::reRegist(shared_ptr<UaSessionInfo> ua)
{
    if (ua->i_State == 0)
    {
        Regist(ua);
    }
    /*if (ua->mh.isValid() || ua->mh->isRequestPending())
    {
    }
    else
    {
        Regist(ua);
    }*/
}
void UaMgr::DoCancelRegist(const Data& targetuser)
{
    shared_ptr<UaSessionInfo> uaInfo = GetUaInfoByUser(targetuser);
    if (uaInfo->mh.isValid())
    {
        uaInfo->mh->end();
        uaInfo->i_State = 0;
    }
    return;
}
shared_ptr<UaSessionInfo> UaMgr::GetUaInfoByCallID(const Data& callID)
{
    CUSTORLOCKGUARD locker(mapMtx);
    map<Data, shared_ptr<UaSessionInfo>>::iterator iter = UserAgentInfoMap.begin();
    for (; iter != UserAgentInfoMap.end(); iter++)
    {
        if (iter->second->regcallid == callID)
        {
            return iter->second;
        }
    }
    shared_ptr<UaSessionInfo> uaSession((UaSessionInfo*)NULL);
    return uaSession;
}
shared_ptr<UaSessionInfo> UaMgr::GetUaInfoByUser(const Data& user)
{
    CUSTORLOCKGUARD locker(mapMtx);
    map<Data, shared_ptr<UaSessionInfo>>::iterator iter = UserAgentInfoMap.find(user);
    if(iter != UserAgentInfoMap.end())
        return iter->second;
    shared_ptr<UaSessionInfo> uaSession((UaSessionInfo*)NULL);
    return uaSession;
}
shared_ptr<UaSessionInfo> UaMgr::GetNextUaInfoByUser(const Data& user)
{
    CUSTORLOCKGUARD locker(mapMtx);
    if (user.empty() && UserAgentInfoMap.size() > 0)
    {
        return UserAgentInfoMap.begin()->second;
    }
    else
    {
        map<Data, shared_ptr<UaSessionInfo>>::iterator iter = UserAgentInfoMap.find(user);
        if (iter != UserAgentInfoMap.end() && ++iter != UserAgentInfoMap.end())
            return iter->second;
    }
    shared_ptr<UaSessionInfo> uaSession((UaSessionInfo*)NULL);
    return uaSession;
}
void __stdcall UaMgr::RegistStateCallBack(const Data& callID, ClientRegistrationHandle h, int reason, void* pUserData)
{
    if (pUserData)
    {
        UaMgr* pThis = (UaMgr*)pUserData;
        shared_ptr<UaSessionInfo> ua = pThis->GetUaInfoByCallID(callID);
        if (ua)
        {
            ua->i_State = reason;
            ua->mh = h;
            if (reason == 200)
                ua->heartTimeOutCount = 0;
        }
        else
        {
            h->removeMyBindingsCommand(true);
        }
    }
}
void UaMgr::checkStateThread(UaMgr* chandle)
{
    int count = 0;
    while (!chandle->mDumShutdown)
    {
        if (count++ > 30)
        {
            chandle->CheckRegistState();
            count = 0;
        }
#ifdef _WIN32
        Sleep(1000);
#else
        usleep(1000 * 1000);
#endif
    }
}
void UaMgr::CheckRegistState()
{
    Data user;
    shared_ptr<UaSessionInfo> uaState;
    while ((uaState = GetNextUaInfoByUser(user)) != NULL)
    {
        user = uaState->toUri.user();
        if (uaState->heartTimeOutCount >= mDevCfg.HeartBeatCount)
        {
            if (uaState->mh.isValid())
            {
                uaState->i_State = 0;
                uaState->mh->end();
            }
            else
            {
                //reRegist(uaState);
            }
        }
        else
        {
           /* if(mMessageMgr)
                mMessageMgr->SendHeart(uaState, mDevCfg);*/
        }
    }
}
bool UaMgr::RequestLiveStream(std::string devId, std::string devIp, int devPort, std::string channelId, int streamId, int sdpPort, int rtpType, UaClientCall* pAleg)
{
    Uri mCallTarget;
    mCallTarget.user() = channelId.c_str();
    mCallTarget.host() = devIp.c_str();
    mCallTarget.port() = devPort;

    //UaClientCall* newCall = new UaClientCall(*this);
    //newCall->devId = devId;
    //newCall->devIp = devIp;
    //newCall->devPort = devPort;
    //newCall->channelId = channelId;
    //newCall->myRtpPort = sdpPort;
    //newCall->rtpType = rtpType;
    //newCall->mySdpIp = "192.168.1.230";// DnsUtil::getLocalIpAddress().c_str();
    //newCall->ssrc = CreateSSRC("Play", channelId).c_str();
    //newCall->sessionName = "Play";
    //newCall->streamId = streamId;
    BaseDevice::Ptr parentDev = NULL;
    BaseChildDevice *childDev = NULL;
    if (devId.empty())
    {
        if (!channelId.empty())
        {
            childDev = DeviceMng::Instance().findChildDevice(channelId);
            if (childDev)
            {
                parentDev = childDev->getParentDev();
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {

    }
    
    if (parentDev)
    {
        if (parentDev->devType == BaseDevice::JSON_NVR)
        {
            JsonNvrDevic::Ptr Nvr = std::dynamic_pointer_cast<JsonNvrDevic>(parentDev);
            if (Nvr)
            {
                int err = 0, chl = -1;
                uint32_t msgSize = 16384;
                char Buffer[16384] = { 0 };
                Nvr->Dev_ListIPC(Buffer, msgSize, err);
                if (err == 0)
                {
                    rapidjson::Document document;
                    document.Parse(Buffer);
                    if (!document.HasParseError())
                    {
                        if (document.HasMember("ipc_list") && document["ipc_list"].IsArray())
                        {
                            rapidjson::Value& body = document["ipc_list"];
                            for (uint32_t i = 0; i < body.Size(); i++)
                            {
                                std::string devNum = json_check_string(body[i], "device_number");
                                JsonChildDevic* pChild = dynamic_cast<JsonChildDevic*>(childDev);
                                if (devNum == pChild->getName())
                                {
                                    int enable_flag = json_check_int32(body[i], "enable_flag");
                                    if (enable_flag == 2)
                                    {
                                        int child = json_check_int32(body[i], "chid");
                                        int online = json_check_int32(body[i], "online_status");
                                        if (online == 1)
                                        {
                                            std::string Id = json_check_string(body[i], "device_id");
                                            chl = child;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
                if (chl >= 0)
                {
                    JsonStream::Ptr streamIn = std::make_shared<JsonStream>(channelId.c_str());
                    ULHandle playhandle = Nvr->Dev_Preview(chl, streamId, JsonStream::VskX86NvrRtPreDataCb, (void*)streamIn.get(), err);
                    if (err == 0)
                    {
                        //parent->streamStatus = UaClientCall::CALL_MY_MEDIA_OK;
                        //create mediain
                        streamIn->setStreamHandle(playhandle);
                        MediaMng::GetInstance().addStream(streamIn);
                        return true;
                    }
                    else
                    {
                        //delete streamIn; streamIn = NULL;
                        return false;
                    }
                }
            }
        }
    }
    
    
    

   /* auto dev = new JsonDevice(devIp.c_str(), devPort, "admin", "12345", 56);
    dev->setParent(parent);

    int err = 0;
    if (dev)
    {
        BaseDevice::ULHandle playhandle = dev->Dev_Preview(UaClientCall::VskX86NvrRtPreDataCb, (void*)pAleg, err);
        dev->streamStatus = UaClientCall::CALL_MY_MEDIA_OK;
    }*/
    {
        CUSTORLOCKGUARD locker(mapStreamMtx);
        //m_StreamInfoMap[streamId] = NULL;// dev;
    }

    //newCall->initiateCall(mCallTarget, mProfile);
    return false;
}
bool UaMgr::RequestVodStream(std::string devId, std::string devIp, int devPort, std::string channelId, std::string streamId, int sdpPort, int rtpType, unsigned long stime, unsigned long etime)
{
    Uri mCallTarget;
    mCallTarget.user() = channelId.c_str();
    mCallTarget.host() = devIp.c_str();
    mCallTarget.port() = devPort;

    UaClientCall* newCall = new UaClientCall(*this);
    newCall->devId = devId;
    newCall->devIp = devIp;
    newCall->devPort = devPort;
    newCall->channelId = channelId;
    newCall->myRtpPort = sdpPort;
    newCall->rtpType = rtpType;
    newCall->mySdpIp = DnsUtil::getLocalIpAddress().c_str();
    newCall->ssrc = CreateSSRC("Playback", channelId).c_str();
    newCall->sessionName = "Playback";
    newCall->startTime = stime;
    newCall->stopTime = etime;
    CDateTime startTime(stime);
    CDateTime stopTime(etime);
    newCall->streamId = streamId;
    {
        CUSTORLOCKGUARD locker(mapStreamMtx);
        m_StreamInfoMap[streamId] = NULL;// new JsonDevice(devIp.c_str(), devPort, "admin", "12345", 0);
    }
    newCall->initiateCall(mCallTarget, mProfile);
    return true;
}
BaseDevice* UaMgr::GetStreamInfo(std::string channelId)
{
    CUSTORLOCKGUARD locker(mapStreamMtx);
    auto iter = m_StreamInfoMap.find(channelId);
    if (iter != m_StreamInfoMap.end())
    {
        return iter->second;
    }
    return NULL;
}
UaMgr::streamStatus UaMgr::getStreamStatus(std::string channelId)
{
    CUSTORLOCKGUARD locker(mapStreamMtx);
    auto iter = m_StreamInfoMap.find(channelId);
    if (iter != m_StreamInfoMap.end())
    {
        switch (1/*iter->second->streamStatus*/)
        {
        case UaClientCall::CALL_MEDIA_READY:
        case UaClientCall::CALL_MY_MEDIA_OK:
            return _UERAGERNT_STREAM_OK;
        case UaClientCall::CALL_UAC_CONNECTED:
            return _UERAGERNT_CALL_OK;
        case UaClientCall::CALL_UAC_RES_START:
        case UaClientCall::CALL_UAC_NEW_GET1XX:
        case UaClientCall::CALL_UAC_GET1XX_PROV:
        case UaClientCall::CALL_UAC_ANSWER_200OK:
            return _UERAGERNT_STREAM_PENDING;
        case UaClientCall::CALL_UAC_FAILURE:
        case UaClientCall::CALL_UAC_TERMINATED:
        case UaClientCall::CALL_MEDIA_ERROR:
        case UaClientCall::CALL_MEDIA_TIMEOUT:
        default:
            return _UERAGERNT_NOT_STREAM;
        }
    }
    return _UERAGERNT_NOT_STREAM;
}
void UaMgr::setCallStatus(std::string streamId, int status)
{
    CUSTORLOCKGUARD locker(mapStreamMtx);
    auto iter = m_StreamInfoMap.find(streamId);
    if (iter != m_StreamInfoMap.end())
    {
        //iter->second->streamStatus = status;
    }
}
int UaMgr::getCallStatus(std::string streamId)
{
    CUSTORLOCKGUARD locker(mapStreamMtx);
    auto iter = m_StreamInfoMap.find(streamId);
    if (iter != m_StreamInfoMap.end())
    {
        //return iter->second->streamStatus;
    }
    return UaClientCall::CALL_NOT_FOUND;
}
//UaClientCall* UaMgr::reTurnCallByStreamId(std::string streamId)
//{
//    CUSTORLOCKGUARD locker(mapStreamMtx);
//    auto iter = m_StreamInfoMap.find(streamId);
//    if (iter != m_StreamInfoMap.end() && iter->second != NULL)
//    {
//        return iter->second;
//    }
//    return NULL;
//}
bool UaMgr::CloseStreamStreamId(std::string channelId)
{
    /*ostringstream ss;
    ss << "http://192.168.1.38:80/index/api/closeRtpServer?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&stream="
        << devId << "_" << channelId;
    string strReponse = GetRequest(ss.str());*/
    CUSTORLOCKGUARD locker(mapStreamMtx);
    auto it = m_StreamInfoMap.find(channelId);
    if (it != m_StreamInfoMap.end())
    {
        if (1/*--it->second->def == 0*/)
        {
            delete it->second;
        }
        m_StreamInfoMap.erase(channelId);
    }
    return true;
}
//UaMgr::streamStatus UaMgr::getPlaybackStatus(std::string streamId)
//{
//    CUSTORLOCKGUARD locker(mapPbMtx);
//    auto iter = m_PbInfoMap.find(streamId);
//    if (iter != m_PbInfoMap.end() && iter->second != NULL)
//    {
//        switch (iter->second->callState)
//        {
//        case UaClientCall::CALL_MEDIA_READY:
//        case UaClientCall::CALL_MY_MEDIA_OK:
//            return _UERAGERNT_STREAM_OK;
//        case UaClientCall::CALL_UAC_CONNECTED:
//            return _UERAGERNT_CALL_OK;
//        case UaClientCall::CALL_UAC_RES_START:
//        case UaClientCall::CALL_UAC_NEW_GET1XX:
//        case UaClientCall::CALL_UAC_GET1XX_PROV:
//        case UaClientCall::CALL_UAC_ANSWER_200OK:
//            return _UERAGERNT_STREAM_PENDING;
//        default:
//            return _UERAGERNT_NOT_STREAM;
//        }
//    }
//    return _UERAGERNT_NOT_STREAM;
//}
//UaClientCall* UaMgr::reTurnCallByPlaybackStreamId(std::string streamId)
//{
//    CUSTORLOCKGUARD locker(mapPbMtx);
//    auto iter = m_PbInfoMap.find(streamId);
//    if (iter != m_PbInfoMap.end() && iter->second != NULL)
//    {
//        return iter->second;
//    }
//    return NULL;
//}
//bool UaMgr::CloseStreamByPlaybackStreamId(std::string streamId)
//{
//    CUSTORLOCKGUARD locker(mapPbMtx);
//    auto it = m_PbInfoMap.find(streamId);
//    if (it != m_PbInfoMap.end())
//    {
//        m_PbInfoMap.erase(streamId);
//    }
//    return true;
//}
unsigned int UaMgr::GetAvailableRtpPort()
{
    return mRtpPortMngr.allocateRTPPort();
}
void UaMgr::FreeRptPort(unsigned int uiRtpPort)
{
    mRtpPortMngr.freeRTPPort(uiRtpPort);
}
void __stdcall UaMgr::RegistPageMsgCallBack(const Data &UasName, const MsgCmdType& MsgCmdType, int reason, void* pUserData)
{
    if (pUserData)
    {
        if (MsgCmdType == MsgCmdType_Keepalive)
        {
            UaMgr* pThis = (UaMgr*)pUserData;
            shared_ptr<UaSessionInfo> ua = pThis->GetUaInfoByUser(UasName);
            if (ua)
            {
                if (reason == 200)
                {
                    ua->heartTimeOutCount = 0;
                }
                else
                {
                    ua->heartTimeOutCount++;
                }
            }
        }
    }
}
void __stdcall UaMgr::RegistArrivedMsgCallBack(ServerPagerMessageHandle h, const resip::SipMessage& message, void* pUserData)
{

}


void
UaMgr::startup()
{
    if (mRegisterDuration)
    {
        InfoLog(<< "register for " << mAor);
        //mDum->send(mDum->makeRegistration(NameAddr(mAor)));
    }
    else
    {
        // If not registering then form subscription and/or call here.  If registering then we will start these
        // after the registration is successful.

        // Check if we should try to form a test subscription
        if (!mSubscribeTarget.host().empty())
        {
         auto sub = mDum->makeSubscription(NameAddr(mSubscribeTarget), mProfile, "basicClientTest");
         mDum->send(std::move(sub));
      }

        // Check if we should try to form a test call
        if (!mCallTarget.host().empty())
        {
            UaClientCall* newCall = new UaClientCall(*this);
            newCall->initiateCall(mCallTarget, mProfile);
        }
    }
}

void
UaMgr::shutdown()
{
    assert(mDum);
    mDumShutdownRequested = true; // Set flag so that shutdown operations can be run in dum process thread
    mShuttingdown = true;  // This flag stays on during the shutdown process where as mDumShutdownRequested will get toggled back to false
}
bool UaMgr::run()
{
    if (mDumThread)
    {
        mDumThread->run();
        return true;
    }
    return false;
}
bool
UaMgr::process(int timeoutMs)
{
    if (!mDumShutdown)
    {
        if (mDumShutdownRequested)
        {
            // unregister
         if(mRegHandle.isValid())
         {
            mRegHandle->end();
         }

            // end any subscriptions
            if (mServerSubscriptionHandle.isValid())
            {
                mServerSubscriptionHandle->end();
            }
            if (mClientSubscriptionHandle.isValid())
            {
                mClientSubscriptionHandle->end();
            }

            // End all calls - copy list in case delete/unregister of call is immediate
            std::set<UaClientCall*> tempCallList = mCallList;
            std::set<UaClientCall*>::iterator it = tempCallList.begin();
            for (; it != tempCallList.end(); it++)
            {
                (*it)->terminateCall();
            }

            mDum->shutdown(this);
            mDumShutdownRequested = false;
        }
        mDum->process(timeoutMs);
        return true;
    }
    return false;
}

void
UaMgr::post(Message* msg)
{
    ConnectionTerminated* terminated = dynamic_cast<ConnectionTerminated*>(msg);
    if (terminated)
    {
        InfoLog(<< "UaMgr received connection terminated message for: " << terminated->getFlow());
        delete msg;
        return;
    }
    assert(false);
}

void
UaMgr::onNotifyTimeout(unsigned int timerId)
{
    if (timerId == mCurrentNotifyTimerId)
    {
        //sendNotify();
    }
}

void
UaMgr::sendNotify(const Data& event, const Data& content)
{
    CUSTORLOCKGUARD locker(mapSubMtx);
    std::map<Data, ServerSubscriptionInfos>::iterator iter = m_SvSubmap.find(event.c_str());
    if (iter != m_SvSubmap.end())
    {
        ServerSubscriptionInfos& SubInfos = iter->second;
        if (!SubInfos.Subscrips.empty())
        {
            for (auto it : SubInfos.Subscrips)
            {
                PlainContents plain(content.c_str(), Mime("Application", "MANSCDP+xml"));
                it.m_sh->send(it.m_sh->update(&plain));
            }
        }
    }
    if (mServerSubscriptionHandle.isValid())
    {
        PlainContents plain("test notify");
        mServerSubscriptionHandle->send(mServerSubscriptionHandle->update(&plain));

        // start timer for next one
      std::unique_ptr<ApplicationMessage> timer(new NotifyTimer(*this, ++mCurrentNotifyTimerId));
      mStack.post(std::move(timer), NotifySendTime, mDum);
   }
}

void
UaMgr::onCallTimeout(UaClientCall* call)
{
    if (isValidCall(call))
    {
        call->timerExpired();
    }
    else  // call no longer exists
    {
        // If there are no more calls, then start a new one
        if (mCallList.empty() && !mCallTarget.host().empty())
        {
            // re-start a new call
            UaClientCall* newCall = new UaClientCall(*this);
            newCall->initiateCall(mCallTarget, mProfile);
        }
    }
}

void
UaMgr::registerCall(UaClientCall* call)
{
    mCallList.insert(call);
}

void
UaMgr::unregisterCall(UaClientCall* call)
{
    std::set<UaClientCall*>::iterator it = mCallList.find(call);
    if (it != mCallList.end())
    {
        mCallList.erase(it);
    }
}

bool
UaMgr::isValidCall(UaClientCall* call)
{
    std::set<UaClientCall*>::iterator it = mCallList.find(call);
    if (it != mCallList.end())
    {
        return true;
    }
    return false;
}

void
UaMgr::onDumCanBeDeleted()
{
    mDumShutdown = true;
}

////////////////////////////////////////////////////////////////////////////////
// Registration Handler ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
UaMgr::onSuccess(ClientRegistrationHandle h, const SipMessage& msg)
{
    InfoLog(<< "onSuccess(ClientRegistrationHandle): msg=" << msg.brief());
    if (mShuttingdown)
    {
        h->end();
        return;
    }

    if (mRegHandle.getId() == 0)  // Note: reg handle id will only be 0 on first successful registration
    {
        // Check if we should try to form a test subscription
        if (!mSubscribeTarget.host().empty())
        {
         auto sub = mDum->makeSubscription(NameAddr(mSubscribeTarget), mProfile, "basicClientTest");
         mDum->send(std::move(sub));
      }

        // Check if we should try to form a test call
        if (!mCallTarget.host().empty())
        {
            UaClientCall* newCall = new UaClientCall(*this);
            newCall->initiateCall(mCallTarget, mProfile);
        }
    }
    //h->addBinding(msg.header(h_Contacts).front());
    Data localip = resip::Tuple::inet_ntop(msg.getSource());
    int reason = msg.header(h_StatusLine).statusCode();
    RegistStateCallBack(msg.header(h_CallID).value(), h, reason, this);
    //mRegHandle = h;
    mRegistrationRetryDelayTime = 0;  // reset
    Data command;
    {
        DataStream ds(command);
        ds << msg;
    }
    cout << command << endl;
    if (msg.exists(h_Vias))
    {
        const Via& via = msg.header(h_Vias).front();
        if (via.exists(p_received))
        {
            int port = via.sentPort();
            if (via.exists(p_rport) && via.param(p_rport).hasValue())
            {
                port = via.param(p_rport).port();
            }
            Uri contact;
            contact.user() = msg.header(h_From).uri().user();
            contact.host() = via.param(p_received);
            contact.port() = port;
            mProfile->setOverrideHostAndPort(contact);
        }
        else
        {
            if (mProfile->hasOverrideHostAndPort())
            {
                mProfile->unsetOverrideHostAndPort();
            }
        }
    }
}

void
UaMgr::onFailure(ClientRegistrationHandle h, const SipMessage& msg)
{
    InfoLog(<< "onFailure(ClientRegistrationHandle): msg=" << msg.brief());
    //mRegHandle = h;
    if (mShuttingdown)
    {
        h->end();
    }
    int reason = msg.header(h_StatusLine).statusCode();
    RegistStateCallBack(msg.header(h_CallID).value(), h, reason, this);
}

void
UaMgr::onRemoved(ClientRegistrationHandle h, const SipMessage& msg)
{
    InfoLog(<< "onRemoved(ClientRegistrationHandle): msg=" << msg.brief());
    //mRegHandle = h;
    int reason = msg.header(h_StatusLine).statusCode();
    RegistStateCallBack(msg.header(h_CallID).value(), h, reason, this);
}

int
UaMgr::onRequestRetry(ClientRegistrationHandle h, int retryMinimum, const SipMessage& msg)
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

    int reason = msg.header(h_StatusLine).statusCode();
    RegistStateCallBack(msg.header(h_CallID).value(), h, reason, this);

    return retryTime;
}
bool UaMgr::onRefreshRequired(ClientRegistrationHandle h, const SipMessage& lastRequest)
{
    Data command;
    {
        DataStream ds(command);
        ds << lastRequest;
    }
    cout << command << endl;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// InviteSessionHandler ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
UaMgr::onNewSession(ClientInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onNewSession(h, oat, msg);
}

void
UaMgr::onNewSession(ServerInviteSessionHandle h, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onNewSession(h, oat, msg);
}

void
UaMgr::onFailure(ClientInviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onFailure(h, msg);
}

void
UaMgr::onEarlyMedia(ClientInviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onEarlyMedia(h, msg, sdp);
}

void
UaMgr::onProvisional(ClientInviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onProvisional(h, msg);
}

void
UaMgr::onConnected(ClientInviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onConnected(h, msg);
}

void
UaMgr::onConnected(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onConnected(h, msg);
}
void UaMgr::onConnectedConfirmed(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onConnectedConfirmed(h, msg);
}
void
UaMgr::onStaleCallTimeout(ClientInviteSessionHandle h)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onStaleCallTimeout(h);
}

void
UaMgr::onTerminated(InviteSessionHandle h, InviteSessionHandler::TerminatedReason reason, const SipMessage* msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onTerminated(h, reason, msg);
}

void
UaMgr::onRedirected(ClientInviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onRedirected(h, msg);
}

void
UaMgr::onAnswer(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onAnswer(h, msg, sdp);
}

void
UaMgr::onOffer(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onOffer(h, msg, sdp);
}

void
UaMgr::onOfferRequired(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onOfferRequired(h, msg);
}

void
UaMgr::onOfferRejected(InviteSessionHandle h, const SipMessage* msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onOfferRejected(h, msg);
}

void
UaMgr::onOfferRequestRejected(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onOfferRequestRejected(h, msg);
}

void
UaMgr::onRemoteSdpChanged(InviteSessionHandle h, const SipMessage& msg, const SdpContents& sdp)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onRemoteSdpChanged(h, msg, sdp);
}

void
UaMgr::onInfo(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onInfo(h, msg);
}

void
UaMgr::onInfoSuccess(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onInfoSuccess(h, msg);
}

void
UaMgr::onInfoFailure(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onInfoFailure(h, msg);
}

void
UaMgr::onRefer(InviteSessionHandle h, ServerSubscriptionHandle ssh, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onRefer(h, ssh, msg);
}

void
UaMgr::onReferAccepted(InviteSessionHandle h, ClientSubscriptionHandle csh, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onReferAccepted(h, csh, msg);
}
void UaMgr::onAckReceived(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onAckReceived(h, msg);
}
void
UaMgr::onReferRejected(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onReferRejected(h, msg);
}

void
UaMgr::onReferNoSub(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onReferNoSub(h, msg);
}

void
UaMgr::onMessage(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onMessage(h, msg);
}

void
UaMgr::onMessageSuccess(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onMessageSuccess(h, msg);
}

void
UaMgr::onMessageFailure(InviteSessionHandle h, const SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onMessageFailure(h, msg);
}

void
UaMgr::onForkDestroyed(ClientInviteSessionHandle h)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onForkDestroyed(h);
}

void
UaMgr::onReadyToSend(InviteSessionHandle h, SipMessage& msg)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onReadyToSend(h, msg);
    std::cout << "*********************************************\n"
        << msg
        << "******************************************************\n" << std::endl;
}

void
UaMgr::onFlowTerminated(InviteSessionHandle h)
{
    dynamic_cast<UaClientCall*>(h->getAppDialogSet().get())->onFlowTerminated(h);
}


////////////////////////////////////////////////////////////////////////////////
// DialogSetHandler ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
UaMgr::onTrying(AppDialogSetHandle h, const SipMessage& msg)
{
    UaClientCall* call = dynamic_cast<UaClientCall*>(h.get());
    if (call)
    {
        call->onTrying(h, msg);
    }
    else
    {
        InfoLog(<< "onTrying(AppDialogSetHandle): " << msg.brief());
    }
}

void
UaMgr::onNonDialogCreatingProvisional(AppDialogSetHandle h, const SipMessage& msg)
{
    UaClientCall* call = dynamic_cast<UaClientCall*>(h.get());
    if (call)
    {
        call->onNonDialogCreatingProvisional(h, msg);
    }
    else
    {
        InfoLog(<< "onNonDialogCreatingProvisional(AppDialogSetHandle): " << msg.brief());
    }
}

////////////////////////////////////////////////////////////////////////////////
// ClientSubscriptionHandler ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
UaMgr::onUpdatePending(ClientSubscriptionHandle h, const SipMessage& msg, bool outOfOrder)
{
    UaClientCall* call = dynamic_cast<UaClientCall*>(h->getAppDialogSet().get());
    if (call)
    {
        call->onUpdatePending(h, msg, outOfOrder);
        return;
    }
    InfoLog(<< "onUpdatePending(ClientSubscriptionHandle): " << msg.brief());
    h->acceptUpdate();
}

void
UaMgr::onUpdateActive(ClientSubscriptionHandle h, const SipMessage& msg, bool outOfOrder)
{
    UaClientCall* call = dynamic_cast<UaClientCall*>(h->getAppDialogSet().get());
    if (call)
    {
        call->onUpdateActive(h, msg, outOfOrder);
        return;
    }
    InfoLog(<< "onUpdateActive(ClientSubscriptionHandle): " << msg.brief());
    h->acceptUpdate();
}

void
UaMgr::onUpdateExtension(ClientSubscriptionHandle h, const SipMessage& msg, bool outOfOrder)
{
    UaClientCall* call = dynamic_cast<UaClientCall*>(h->getAppDialogSet().get());
    if (call)
    {
        call->onUpdateExtension(h, msg, outOfOrder);
        return;
    }
    InfoLog(<< "onUpdateExtension(ClientSubscriptionHandle): " << msg.brief());
    h->acceptUpdate();
}

void
UaMgr::onNotifyNotReceived(ClientSubscriptionHandle h)
{
    UaClientCall* call = dynamic_cast<UaClientCall*>(h->getAppDialogSet().get());
    if (call)
    {
        call->onNotifyNotReceived(h);
        return;
    }
    WarningLog(<< "onNotifyNotReceived(ClientSubscriptionHandle)");
    h->end();
}

void
UaMgr::onTerminated(ClientSubscriptionHandle h, const SipMessage* msg)
{
    UaClientCall* call = dynamic_cast<UaClientCall*>(h->getAppDialogSet().get());
    if (call)
    {
        call->onTerminated(h, msg);
        return;
    }
    if (msg)
    {
        InfoLog(<< "onTerminated(ClientSubscriptionHandle): msg=" << msg->brief());
    }
    else
    {
        InfoLog(<< "onTerminated(ClientSubscriptionHandle)");
    }
}

void
UaMgr::onNewSubscription(ClientSubscriptionHandle h, const SipMessage& msg)
{
    UaClientCall* call = dynamic_cast<UaClientCall*>(h->getAppDialogSet().get());
    if (call)
    {
        call->onNewSubscription(h, msg);
        return;
    }
    mClientSubscriptionHandle = h;
    InfoLog(<< "onNewSubscription(ClientSubscriptionHandle): msg=" << msg.brief());
}

int
UaMgr::onRequestRetry(ClientSubscriptionHandle h, int retrySeconds, const SipMessage& msg)
{
    UaClientCall* call = dynamic_cast<UaClientCall*>(h->getAppDialogSet().get());
    if (call)
    {
        return call->onRequestRetry(h, retrySeconds, msg);
    }
    InfoLog(<< "onRequestRetry(ClientSubscriptionHandle): msg=" << msg.brief());
    return FailedSubscriptionRetryTime;
}

////////////////////////////////////////////////////////////////////////////////
// ServerSubscriptionHandler ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
UaMgr::onNewSubscription(ServerSubscriptionHandle h, const SipMessage& msg)
{
    InfoLog(<< "onNewSubscription(ServerSubscriptionHandle): " << msg.brief());
    Contents* content = msg.getContents();
    if (content == NULL)
    {
        h->end();
        return;
    }
    GB28181XmlMsg XmlMsg;
    if (AnalyzeSubscriptionMsg(content->getBodyData().c_str(), XmlMsg))
    {
        string outStr;
        if (XmlMsg.cmdname == XML_CMD_NAME_SUBSCRIPTION)
        {
            if (XmlMsg.cmdtype == XML_CMDTYPE_REQUEST_CATALOG_SUBSCRIPTION)
            {
                CatalogSubscriptionMsg* pCatalogSubMsg = (CatalogSubscriptionMsg*)XmlMsg.pPoint;
                //CreateCatalogSubscriptionResponseMsg(pCatalogSubMsg->DeviceID.c_str(), XmlMsg.sn, outStr);
            }
            else if (XmlMsg.cmdtype == XML_CMDTYPE_REQUEST_ALARM_SUBSCRIPTION)
            {

            }
        }
        h->setSubscriptionState(Active);
        shared_ptr<SipMessage> msg = h->accept();
        unique_ptr<Contents> content(new PlainContents(outStr.c_str(), Mime("Application", "MANSCDP+xml")));
        msg->setContents(std::move(content));
        h->send(msg);
    }

    Data event = msg.header(h_Event).value();
    CUSTORLOCKGUARD locker(mapSubMtx);
    std::map<Data, ServerSubscriptionInfos>::iterator iter = m_SvSubmap.find(event);
    if (iter != m_SvSubmap.end())
    {
        ServerSubscriptionInfos& SubInfos = iter->second;
        if (SubInfos.Subscrips.empty())
        {
            ServerSubscriptionInfo subinfo;
            subinfo.m_content = content->getBodyData();
            subinfo.m_sh = h;
            SubInfos.Subscrips.push_back(subinfo);
        }
        else
        {
            bool findflag = false;
            for (auto it : SubInfos.Subscrips)
            {
                if (it.m_sh.getId() == h.getId())
                {
                    findflag = true;
                    it.m_content = content->getBodyData();
                }
            }
            if (!findflag)
            {
                ServerSubscriptionInfo subinfo;
                subinfo.m_content = content->getBodyData();
                subinfo.m_sh = h;
                SubInfos.Subscrips.push_back(subinfo);
            }
        }
    }
}

void
UaMgr::onNewSubscriptionFromRefer(ServerSubscriptionHandle ss, const SipMessage& msg)
{
    InfoLog(<< "onNewSubscriptionFromRefer(ServerSubscriptionHandle): " << msg.brief());
    // Received an out-of-dialog refer request with implicit subscription
    try
    {
        if (msg.exists(h_ReferTo))
        {
            // Check if TargetDialog header is present
            if (msg.exists(h_TargetDialog))
            {
                pair<InviteSessionHandle, int> presult;
                presult = mDum->findInviteSession(msg.header(h_TargetDialog));
                if (!(presult.first == InviteSessionHandle::NotValid()))
                {
                    UaClientCall* callToRefer = (UaClientCall*)presult.first->getAppDialogSet().get();

                    callToRefer->onRefer(presult.first, ss, msg);
                    return;
                }
            }

            // We don't support ood refers that don't target a dialog - reject request 
            WarningLog(<< "onNewSubscriptionFromRefer(ServerSubscriptionHandle): Received ood refer (noSub) w/out a Target-Dialog: " << msg.brief());
            ss->send(ss->reject(400));
        }
        else
        {
            WarningLog(<< "onNewSubscriptionFromRefer(ServerSubscriptionHandle): Received refer w/out a Refer-To: " << msg.brief());
            ss->send(ss->reject(400));
        }
    }
    catch (BaseException& e)
    {
        WarningLog(<< "onNewSubscriptionFromRefer(ServerSubscriptionHandle): exception " << e);
    }
    catch (...)
    {
        WarningLog(<< "onNewSubscriptionFromRefer(ServerSubscriptionHandle): unknown exception");
    }
}

void
UaMgr::onRefresh(ServerSubscriptionHandle, const SipMessage& msg)
{
    InfoLog(<< "onRefresh(ServerSubscriptionHandle): " << msg.brief());
}

void
UaMgr::onTerminated(ServerSubscriptionHandle)
{
    InfoLog(<< "onTerminated(ServerSubscriptionHandle)");
}

void
UaMgr::onReadyToSend(ServerSubscriptionHandle, SipMessage&)
{
}

void
UaMgr::onNotifyRejected(ServerSubscriptionHandle, const SipMessage& msg)
{
    WarningLog(<< "onNotifyRejected(ServerSubscriptionHandle): " << msg.brief());
}

void
UaMgr::onError(ServerSubscriptionHandle, const SipMessage& msg)
{
    WarningLog(<< "onError(ServerSubscriptionHandle): " << msg.brief());
}

void
UaMgr::onExpiredByClient(ServerSubscriptionHandle, const SipMessage& sub, SipMessage& notify)
{
    InfoLog(<< "onExpiredByClient(ServerSubscriptionHandle): " << notify.brief());
}

void
UaMgr::onExpired(ServerSubscriptionHandle, SipMessage& msg)
{
    InfoLog(<< "onExpired(ServerSubscriptionHandle): " << msg.brief());
}

bool
UaMgr::hasDefaultExpires() const
{
    return true;
}

uint32_t UaMgr::getDefaultExpires() const
{
    return 60;
}

////////////////////////////////////////////////////////////////////////////////
// OutOfDialogHandler //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
UaMgr::onSuccess(ClientOutOfDialogReqHandle, const SipMessage& msg)
{
    InfoLog(<< "onSuccess(ClientOutOfDialogReqHandle): " << msg.brief());
}

void
UaMgr::onFailure(ClientOutOfDialogReqHandle h, const SipMessage& msg)
{
    WarningLog(<< "onFailure(ClientOutOfDialogReqHandle): " << msg.brief());
}

void
UaMgr::onReceivedRequest(ServerOutOfDialogReqHandle ood, const SipMessage& msg)
{
    InfoLog(<< "onReceivedRequest(ServerOutOfDialogReqHandle): " << msg.brief());

    switch (msg.method())
    {
    case OPTIONS:
    {
         auto optionsAnswer = ood->answerOptions();
         ood->send(std::move(optionsAnswer));
         break;
      }
   default:
      ood->send(ood->reject(501 /* Not Implemented*/));
      break;
   }
}

////////////////////////////////////////////////////////////////////////////////
// RedirectHandler /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void
UaMgr::onRedirectReceived(AppDialogSetHandle h, const SipMessage& msg)
{
    UaClientCall* call = dynamic_cast<UaClientCall*>(h.get());
    if (call)
    {
        call->onRedirectReceived(h, msg);
    }
    else
    {
        InfoLog(<< "onRedirectReceived(AppDialogSetHandle): " << msg.brief());
    }
}

bool
UaMgr::onTryingNextTarget(AppDialogSetHandle, const SipMessage& msg)
{
    InfoLog(<< "onTryingNextTarget(AppDialogSetHandle): " << msg.brief());

    // Always allow redirection for now
    return true;
}
static unsigned int g_YSRCSeq = 0;
unsigned int CreateSSRCseq()
{
    if (g_YSRCSeq >= 0x270f) //
    {
        g_YSRCSeq = 0;
    }
    g_YSRCSeq++;
    return g_YSRCSeq;
}
std::string UaMgr::CreateSSRC(std::string name, std::string streamId)
{
	std::string ssrc;
	if (name == std::string("Play"))
	{
		ssrc += std::to_string(0);
	}
	else
	{
		ssrc += std::to_string(1);
	}
	if (streamId.size() == 20)
	{
		ssrc += streamId.substr(3, 5);
	}
	else
	{
		ssrc += std::string("00000");
	}

    //ssrc += std::str_format("%04d", CreateSSRCseq());
	return ssrc;
}