#ifndef USERAGENTMGR_H_
#define USERAGENTMGR_H_
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <set>
#include <thread>
//#include "basicClientCmdLineParser.hxx"

#include "resip/stack/EventStackThread.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/RegistrationHandler.hxx"
#include "resip/dum/SubscriptionHandler.hxx"
#include "resip/dum/RedirectHandler.hxx"
#include "resip/dum/DialogSetHandler.hxx"
#include "resip/dum/DumShutdownHandler.hxx"
#include "resip/dum/OutOfDialogHandler.hxx"
#include "resip/dum/InviteSessionHandler.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/Postable.hxx"
#include <resip/dum/PublicationHandler.hxx>

#include "UaMessageMgr.h"
#include "localMediaServer/StreamInfo.h"
#include "RtpPortManager.h"

namespace resip
{
class UaClientCall;
class FdPollGrp;

class UaMgr : //public BasicClientCmdLineParser,
    public Postable,
    public DialogSetHandler,
    public ClientRegistrationHandler,
    public ClientSubscriptionHandler,
    public ServerSubscriptionHandler,
    public OutOfDialogHandler,
    public InviteSessionHandler,
    public DumShutdownHandler,
    public RedirectHandler
{
public:
    UaMgr(SipStack& stack);
    virtual ~UaMgr();

    virtual void startup();
    virtual void shutdown();
    bool process(int timeoutMs);  // returns false when shutdown is complete and process should no longer be called

    DialogUsageManager& getDialogUsageManager() { return *mDum; }
    std::shared_ptr<UserProfile> getIncomingUserProfile(const SipMessage& msg) { return mProfile; } // This test program only uses the one global Master Profile - just return it

protected:
    // Postable Handler ////////////////////////////////////////////////////////////
    virtual void post(Message*);  // Used to receive Connection Terminated messages

    // Shutdown Handler ////////////////////////////////////////////////////////////
    void onDumCanBeDeleted();

    // Registration Handler ////////////////////////////////////////////////////////
    using ClientRegistrationHandler::onFlowTerminated;
    virtual void onSuccess(resip::ClientRegistrationHandle h, const resip::SipMessage& response);
    virtual void onFailure(resip::ClientRegistrationHandle h, const resip::SipMessage& response);
    virtual void onRemoved(resip::ClientRegistrationHandle h, const resip::SipMessage& response);
    virtual int onRequestRetry(resip::ClientRegistrationHandle h, int retryMinimum, const resip::SipMessage& msg);
    virtual bool onRefreshRequired(ClientRegistrationHandle h, const SipMessage& lastRequest);

    // ClientSubscriptionHandler ///////////////////////////////////////////////////
    using ClientSubscriptionHandler::onReadyToSend;
    using ClientSubscriptionHandler::onFlowTerminated;
    virtual void onUpdatePending(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder);
    virtual void onUpdateActive(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder);
    virtual void onUpdateExtension(resip::ClientSubscriptionHandle, const resip::SipMessage& notify, bool outOfOrder);
    virtual void onNotifyNotReceived(ClientSubscriptionHandle h);
    virtual void onTerminated(resip::ClientSubscriptionHandle h, const resip::SipMessage* notify);
    virtual void onNewSubscription(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify);
    virtual int  onRequestRetry(resip::ClientSubscriptionHandle h, int retrySeconds, const resip::SipMessage& notify);

    // Invite Session Handler /////////////////////////////////////////////////////
    virtual void onNewSession(resip::ClientInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
    virtual void onNewSession(resip::ServerInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
    virtual void onFailure(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg);
    virtual void onEarlyMedia(resip::ClientInviteSessionHandle, const resip::SipMessage&, const resip::SdpContents&);
    virtual void onProvisional(resip::ClientInviteSessionHandle, const resip::SipMessage& msg);
    virtual void onConnected(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg);
    virtual void onConnected(resip::InviteSessionHandle, const resip::SipMessage& msg);
    virtual void onConnectedConfirmed(InviteSessionHandle h, const SipMessage& msg);
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
    virtual void onAckReceived(InviteSessionHandle h, const SipMessage& msg);
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

    // ServerSubscriptionHandler ///////////////////////////////////////////////////
    using ServerSubscriptionHandler::onFlowTerminated;
    virtual void onNewSubscription(resip::ServerSubscriptionHandle, const resip::SipMessage& sub);
    virtual void onNewSubscriptionFromRefer(resip::ServerSubscriptionHandle, const resip::SipMessage& sub);
    virtual void onRefresh(resip::ServerSubscriptionHandle, const resip::SipMessage& sub);
    virtual void onTerminated(resip::ServerSubscriptionHandle);
    virtual void onReadyToSend(resip::ServerSubscriptionHandle, resip::SipMessage&);
    virtual void onNotifyRejected(resip::ServerSubscriptionHandle, const resip::SipMessage& msg);
    virtual void onError(resip::ServerSubscriptionHandle, const resip::SipMessage& msg);
    virtual void onExpiredByClient(resip::ServerSubscriptionHandle, const resip::SipMessage& sub, resip::SipMessage& notify);
    virtual void onExpired(resip::ServerSubscriptionHandle, resip::SipMessage& notify);
    virtual bool hasDefaultExpires() const;
    virtual uint32_t getDefaultExpires() const;

    // OutOfDialogHandler //////////////////////////////////////////////////////////
    virtual void onSuccess(resip::ClientOutOfDialogReqHandle, const resip::SipMessage& response);
    virtual void onFailure(resip::ClientOutOfDialogReqHandle, const resip::SipMessage& response);
    virtual void onReceivedRequest(resip::ServerOutOfDialogReqHandle, const resip::SipMessage& request);

    // RedirectHandler /////////////////////////////////////////////////////////////
    virtual void onRedirectReceived(resip::AppDialogSetHandle, const resip::SipMessage& response);
    virtual bool onTryingNextTarget(resip::AppDialogSetHandle, const resip::SipMessage& request);

protected:
    //void addTransport(TransportType type, int port);
    friend class NotifyTimer;
    void onNotifyTimeout(unsigned int timerId);
    //void sendNotify();
    friend class CallTimer;
    void onCallTimeout(UaClientCall* call);

    std::shared_ptr<MasterProfile> mProfile;
    // Using pointers for the following classes so that we can control object destruction order
    Security* mSecurity;
    FdPollGrp* mPollGrp;
    EventThreadInterruptor* mInterruptor;
    SipStack& mStack;
    EventStackThread* mStackThread;
    DialogUsageManager* mDum;
    volatile bool mDumShutdownRequested;
    bool mShuttingdown;
    bool mDumShutdown;
    ClientRegistrationHandle mRegHandle;
    ClientSubscriptionHandle mClientSubscriptionHandle;
    ServerSubscriptionHandle mServerSubscriptionHandle;
    unsigned int mRegistrationRetryDelayTime;
    unsigned int mCurrentNotifyTimerId;

    friend class UaClientCall;
    std::set<UaClientCall*> mCallList;
    void registerCall(UaClientCall* call);
    void unregisterCall(UaClientCall* call);
    bool isValidCall(UaClientCall* call);
public:
    bool run();
    void DoRegist(const Uri& target, const Uri& fromUri, const Data& passwd);
    shared_ptr<UaSessionInfo> GetUaInfoByUser(const Data& user);
    shared_ptr<UaSessionInfo> GetNextUaInfoByUser(const Data& user);
    shared_ptr<UaSessionInfo> GetUaInfoByCallID(const Data& callID);
    void sendNotify(const Data& event, const Data& content);
    void Regist(shared_ptr<UaSessionInfo> ua);
    void reRegist(shared_ptr<UaSessionInfo> ua);
    void DoCancelRegist(const Data& targetuser);
    void CheckRegistState();
    CUserMessageMrg *GetMsgMgr() { return mMessageMgr; }
    bool RequestStream(std::string devIp, int devPort, std::string channelId, int sdpPort, UaClientCall* pUaClientCall);
    bool IsStreamExist(std::string channelId);
    bool CloseStreamStreamId(std::string channelId);
    unsigned int GetAvailableRtpPort();
    void FreeRptPort(unsigned int uiRtpPort);

    static void checkStateThread(UaMgr* chandle);
    static void __stdcall RegistStateCallBack(const Data& callID, ClientRegistrationHandle h, int reason, void* pUserData);
    static void __stdcall RegistPageMsgCallBack(const Data& UasName, const MsgCmdType& MsgCmdType, int reason, void* pUserData);
    static void __stdcall RegistArrivedMsgCallBack(ServerPagerMessageHandle h, const resip::SipMessage& message, void* pUserData);

public:
    resip::ThreadIf* mDumThread;
    CUserMessageMrg* mMessageMgr;
    resip::Uri mAor;
    std::mutex mapMtx;
    std::map<Data, shared_ptr<UaSessionInfo>> UserAgentInfoMap;
    std::mutex mapSubMtx;
    std::map<Data, ServerSubscriptionInfos> m_SvSubmap;
    std::mutex mapStreamMtx;
    std::map<std::string, InStreamInfo> m_StreamInfoMap;

    bool mHostFileLookupOnlyDnsMode;
    bool mOutboundEnabled;
    int mRegisterDuration;
    resip::Uri mContact;
    resip::Uri mOutboundProxy;
    DevConfig mDevCfg;
    std::thread StateThread;
    resip::Uri mSubscribeTarget;
    resip::Uri mCallTarget;


    RTPPortManager mRtpPortMngr;
};
}
#endif