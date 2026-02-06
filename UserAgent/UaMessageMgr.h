#ifndef UA_MESSAGE_MGR_H_
#define UA_MESSAGE_MGR_H_
#include "resip/dum/ServerPagerMessage.hxx"
#include "resip/dum/ClientPagerMessage.hxx"
#include "resip/dum/PagerMessageHandler.hxx"
#include "rutil/Time.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include <mutex>
#include <atomic>


#include "../uainfo.h"
#include "../device/CatalogData.h"
#include "../deviceMng/VirtualOrganization.h"
#include "GB28181Msg.h"



//#define RESIPROCATE_SUBSYSTEM Subsystem::TEST
using namespace resip;
using namespace std;

typedef void(__stdcall* RegistPageMsgStateCallBack)(const Data& UasName, const MsgCmdType& MsgCmdType, int reason, void* pUserData);
typedef void(__stdcall* RegistArrivedMsgCallBackFun)(ServerPagerMessageHandle h, const resip::SipMessage& message, void* pUserData);

class PtzCtrlObj;
class CUserMessageMrg : public ServerPagerMessageHandler,
    public ClientPagerMessageHandler
{
public:
    CUserMessageMrg(DialogUsageManager& dum);
    ~CUserMessageMrg() {}

    // session  message
    virtual void onMessageArrived(resip::ServerPagerMessageHandle handle, const resip::SipMessage& message);
    // message
    virtual void onSuccess(ClientPagerMessageHandle, const SipMessage& status);
    virtual void onFailure(ClientPagerMessageHandle, const SipMessage& status, std::unique_ptr<Contents> contents);
public:
    void SendHeart(shared_ptr<UaSessionInfo> uaState, const DevConfig& config);
    MsgCmdType PopMsgTypeByCallID(const Data& callID);
    bool findMsgCallID(const Data& callID);
    void GetFromuserByMessage(const  SipMessage& message, Uri& fromuser);
    void GetAndReFormateRecordRoute(const SipMessage& outgoing, NameAddrs& Routlist);
    resip::Data SendResponsePageMsg(const Uri& target, const string& sContent, const MsgCmdType& MsgCmdType, const NameAddrs& Routlist);
    void SendRequestPageMsg(const Uri& target, const string& sContent, const MsgCmdType& MsgCmdType);
    CatalogDateQuery* GetCatalogQuery(string deviceId);
    void PopCatalogQuery(string deviceId);

    void PtzControlResponseTask(std::string deviceId, PTZCMDType ptzCmd, int ControlPriority);
    void CatalogQueryResponseTask(const Uri target, const std::string user, const uint32_t sn, const NameAddrs Routlist);
    void DeviceInfoQueryResponseTask(const Uri target, const std::string user, const uint32_t sn, const NameAddrs Routlist);
    void QueryCatalogTask(const Uri target, const std::string user, const uint32_t sn, const NameAddrs Routlist);
    void RecordInfoQueryResponseTask(const Uri target, const std::string user, const uint32_t sn, RecordInfoQueryMsg queryMsg, const NameAddrs Routlist);
    public:
    void RegistPageMsgCallBack(RegistPageMsgStateCallBack fun, void* pUser);
    void RegistArrivedMsgCallBack(RegistArrivedMsgCallBackFun fun, void* pUser);
    uint32_t getMsgId();
protected:
    std::mutex CallIdmapMtx;
    map<Data, MsgCmdType> MsgCallIDs;
    std::atomic<uint32_t> mMsgSn;
    std::mutex CatalogmapMtx;
    std::map<string, CatalogDateQuery*> mCatalogQueryMap;
private:
    DialogUsageManager& mDum;

    RegistPageMsgStateCallBack PageMsgStateFun;
    void* PageMsgStateUser;
    RegistArrivedMsgCallBackFun ArrivedMsgFun;
    void* ArrivedMsgUser;
};
#endif