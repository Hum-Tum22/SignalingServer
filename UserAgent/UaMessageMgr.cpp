#include "UaMessageMgr.h"
#include "resip/stack/PlainContents.hxx"
#include "../MsgContentXml.h"
#include "../XmlMsgAnalysis.h"
#include "../device/DeviceManager.h"
#include "../tools/genuuid.h"
#include "../tools/CodeConversion.h"
#include "../tools/ThreadPool.h"

#include "../http.h"
#include "myjsondef.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"

#include "../deviceMng/deviceMng.h"
#include "../deviceMng/JsonDevice.h"

using namespace std;

CUserMessageMrg::CUserMessageMrg(DialogUsageManager& dum):mDum(dum), mMsgSn(1)
{
	//G_SipMrg()->InitSipDumMrg()->Dum()->getMasterProfile()->addSupportedMethod(MESSAGE);
	//G_SipMrg()->InitSipDumMrg()->Dum()->getMasterProfile()->addSupportedMimeType(MESSAGE, Mime("text", "plain")); // Invite session in-dialog routing testing
	//G_SipMrg()->InitSipDumMrg()->Dum()->getMasterProfile()->addSupportedMimeType(MESSAGE, Mime("Application", "MANSCDP+xml"));//28181
	//G_SipMrg()->InitSipDumMrg()->Dum()->getMasterProfile()->addSupportedMimeType(MESSAGE, Mime("Application", "xml"));
	mDum.setServerPagerMessageHandler(this);
	mDum.setClientPagerMessageHandler(this);
}





// session  message
void CUserMessageMrg::onMessageArrived(resip::ServerPagerMessageHandle h, const resip::SipMessage& message)
{
    if (message.isClientTransaction())
    {
        cout << "onMessageArrived isClientTransaction: " << message << endl;
    }
    if (message.isExternal())
    {
        cout << "onMessageArrived isExternal: " << message << endl;
    }
    if (message.isFromWire())
    {
        cout << "onMessageArrived isFromWire: " << message << endl;
    }
    if (message.isInvalid())
    {
        cout << "onMessageArrived isInvalid: " << message << endl;
    }
    if (message.isRequest())
    {
        cout << "onMessageArrived isInvalid: " << message << endl;
    }
    if (message.isResponse())
    {
        cout << "onMessageArrived isResponse: " << message << endl;
    }
    if (message.hasForceTarget())
    {
        cout << "onMessageArrived ForceTarget: " << message.getForceTarget() << endl;
    }
    Contents* body = message.getContents();
    if (!body || body->getBodyData().empty())
    {
        auto ok = h->accept(400);
        h->send(ok);
        return;
    }
    std::shared_ptr<SipMessage> ok;
    GB28181XmlMsg XmlMsg;
    if (AnalyzeReceivedSipMsg(body->getBodyData().c_str(), XmlMsg))
    {
        ok = h->accept();
    }
    else
    {
        ok = h->accept(400);
    }
    h->send(ok);

    const Tuple& src = message.getSource();
    const Tuple& dst = message.getReceivedTransportTuple();
    Uri fromuser;
    GetFromuserByMessage(message, fromuser);
    NameAddrs routelist;
    GetAndReFormateRecordRoute(message, routelist);

    //XML_CMD_NAME_CONTROL,//表示一个控制的动作
    //XML_CMD_NAME_QUERY,//表示一个查询的动作
    //XML_CMD_NAME_NOTIFY,//表示一个通知的动作
    ////应答命令
    //XML_CMD_NAME_RESPONSE,//表示一个请求动作的应答
    if (XmlMsg.cmdname == XML_CMD_NAME_CONTROL)
    {
        if (XmlMsg.cmdtype == XML_CMDTYPE_DEVICE_CONTROL)//DeviceControl 设备控制
        {
            //XML_CONTROLCMD_PTZ,
            ////TeleBoot 远程启动控制命令
            //XML_CONTROLCMD_TELEBOOT,
            ////RecordCmd 录像控制命令
            //XML_CONTROLCMD_RECORD,
            ////GuardCmd 报警布防/撤防命令
            //XML_CONTROLCMD_GUARD,
            ////AlarmCmd 报警复位命令
            //XML_CONTROLCMD_ALARM,
            ////IFameCmd 强制关键帧命令,设备收到此命令应立刻发送一个IDR帧
            //XML_CONTROLCMD_IFRAME,
            ////DragZoomIn 拉框放大控制命令
            //XML_CONTROLCMD_DRAGZOOMIN,
            ////DragZoomOut 拉框缩小控制命令
            //XML_CONTROLCMD_DRAGZOOMOUT,
            ////HomePosition 看守位控制命令
            //XML_CONTROLCMD_HOMEPOSITION
            if (XmlMsg.controlCmd == XML_CONTROLCMD_PTZ)
            {
                PtzControlInfo* ptz = (PtzControlInfo*)XmlMsg.pPoint;
                ThreadPool::Instance().submit(std::bind(&CUserMessageMrg::PtzControlResponseTask, this, XmlMsg.DeviceID.c_str(), PTZCMDType(ptz->value.c_str()), ptz->ControlPriority));
            }
            else if (XmlMsg.controlCmd == XML_CONTROLCMD_TELEBOOT)
            {
            }
            else if (XmlMsg.controlCmd == XML_CONTROLCMD_RECORD)
            {
            }
            else if (XmlMsg.controlCmd == XML_CONTROLCMD_GUARD)
            {
            }
            else if (XmlMsg.controlCmd == XML_CONTROLCMD_ALARM)
            {
            }
            else if (XmlMsg.controlCmd == XML_CONTROLCMD_IFRAME)
            {
            }
            else if (XmlMsg.controlCmd == XML_CONTROLCMD_DRAGZOOMIN)
            {
            }
            else if (XmlMsg.controlCmd == XML_CONTROLCMD_DRAGZOOMOUT)
            {
            }
            else if (XmlMsg.controlCmd == XML_CONTROLCMD_HOMEPOSITION)
            {
            }
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_DEVICE_CONFIG)//DeviceConfig 设备配置
        {
        }
        else
        {
        }
    }
    else if (XmlMsg.cmdname == XML_CMD_NAME_QUERY)
    {

        if (XmlMsg.cmdtype == XML_CMDTYPE_DEVICE_STATUS)//DeviceStatus 设备状态查询
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_CATALOG)//Catalog 设备目录查询
        {
            ThreadPool::Instance().submit(std::bind(&CUserMessageMrg::CatalogQueryResponseTask, this, fromuser, XmlMsg.DeviceID, XmlMsg.sn, routelist)); 
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_DEVICE_INFO)//DeviceInfo 设备信息查询
        {
            ThreadPool::Instance().submit(std::bind(&CUserMessageMrg::DeviceInfoQueryResponseTask, this, fromuser, XmlMsg.DeviceID, XmlMsg.sn, routelist));
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_RECORDINFO)//RecordInfo 文件目录检索
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_ALARM)//Alarm 报警查询
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_CONFIG_DOWNLOAD)//ConfigDownload 设备配置查询
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_PRESET_QUERY)//PresetQuery 预置位查询
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_MOBILE_POSITION)//MobilePosition 移动设备位置数据查询
        {
        }
        else
        {
        }
    }
    else if (XmlMsg.cmdname == XML_CMD_NAME_NOTIFY)
    {
        ////通知命令
        ////Keepalive 设备状态信息报送
        //XML_CMDTYPE_NOTIFY_KEEPALIVE,
        ////Alarm 报警通知
        //XML_CMDTYPE_NOTIFY_ALARM,
        ////MediaStatus 媒体通知
        //XML_CMDTYPE_NOTIFY_MEDIA_STATUS,
        ////Broadcast 广播通知
        //XML_CMDTYPE_NOTIFY_BROADCAST,
        ////MobilePosition 移动设备位置数据通知
        //XML_CMDTYPE_NOTIFY_MOBILE_POSITION,
        if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_KEEPALIVE)//Keepalive 设备状态信息报送
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_ALARM)//Alarm 报警通知
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MEDIA_STATUS)//MediaStatus 媒体通知
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_BROADCAST)//Broadcast 广播通知
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)//MobilePosition 移动设备位置数据通知
        {
        }
        else
        {
        }
    }
    else if (XmlMsg.cmdname == XML_CMD_NAME_RESPONSE)
    {
        ////应答命令
        ////Catalog 目录信息查询收到应答
        //XML_CMDTYPE_RESPONSE_CATALOG,
        ////Catalog 目录收到应答
        //XML_CMDTYPE_RESPONSE_CATALOG_RECEIVED,
        ////DeviceInfo 设备信息查询应答
        //XML_CMDTYPE_RESPONSE_DEV_INFO,
        ////DeviceStatus 设备状态信息查询应答
        //XML_CMDTYPE_RESPONSE_DEV_STATUS,
        ////RecordInfo 文件目录检索应答
        //XML_CMDTYPE_RESPONSE_RECORD_INFO,
        ////DeviceConfig 设备配置应答
        //XML_CMDTYPE_RESPONSE_DEV_CONFIG,
        ////ConfigDownload 设备配置查询应答
        //XML_CMDTYPE_RESPONSE_CONFIG_DOWNLOAD,
        ////PresetQuery 设备预置位查询应答
        //XML_CMDTYPE_RESPONSE_PRESET_QUERY,
        ////Broadcast 语音广播应答
        //XML_CMDTYPE_RESPONSE_BROADCAST,
        if (XmlMsg.cmdtype == XML_CMDTYPE_RESPONSE_DEV_CONTROL)//DeviceControl 设备控制应答
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_RESPONSE_ALARM)//Alarm 报警通知应答
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_RESPONSE_CATALOG_NOTIFY_RECEIVED)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_RESPONSE_CATALOG_ITEM)//Catalog 设备目录信息查询应答
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_RESPONSE_SUB_CATALOG)//Catalog 目录信息查询收到应答
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_NOTIFY_MOBILE_POSITION)
        {
        }
    }
    else
    {

    }
}
// message
void CUserMessageMrg::onSuccess(ClientPagerMessageHandle h, const SipMessage& status)
{
    int reason = status.header(h_StatusLine).statusCode();
    MsgCmdType cmdtype = PopMsgTypeByCallID(status.header(h_CallID).value());
    if (cmdtype == MsgCmdType_Catalog)
    {

    }
    if (PageMsgStateFun)
        PageMsgStateFun(status.header(h_To).uri().user(), cmdtype, reason, PageMsgStateUser);
	h->end();
}
void CUserMessageMrg::onFailure(ClientPagerMessageHandle h, const SipMessage& status, std::unique_ptr<Contents> contents)
{
    int reason = status.header(h_StatusLine).statusCode();
    MsgCmdType cmdtype = PopMsgTypeByCallID(status.header(h_CallID).value());
    if (PageMsgStateFun)
        PageMsgStateFun(status.header(h_To).uri().user(), cmdtype, reason, PageMsgStateUser);
	h->end();
}
void CUserMessageMrg::RegistPageMsgCallBack(RegistPageMsgStateCallBack fun, void* pUser)
{
	PageMsgStateFun = fun;
	PageMsgStateUser = pUser;
}
void CUserMessageMrg::RegistArrivedMsgCallBack(RegistArrivedMsgCallBackFun fun, void* pUser)
{
	ArrivedMsgFun = fun;
	ArrivedMsgUser = pUser;
}
void CUserMessageMrg::SendHeart(shared_ptr<UaSessionInfo> uaState, const DevConfig& config)
{
    time_t tCurTime = time(0);
    if (uaState->i_State == 200 && uaState->heartTimeOutCount <= config.HeartBeatCount)
    {
        if (uaState->lastSendHeartTime == 0 || tCurTime - uaState->lastSendHeartTime > config.HeartBeatInterval)
        {
            string keepalivestr;
            CreateKeepAliveMsg(uaState->fromUri.user().c_str(), mMsgSn++, keepalivestr);
            ClientPagerMessageHandle cpmh = mDum.makePagerMessage(NameAddr(uaState->toUri));
            unique_ptr<Contents> content(new PlainContents(keepalivestr.c_str(), Mime("Application", "MANSCDP+xml")));
            {
                CUSTORLOCKGUARD locker(CallIdmapMtx);
                MsgCallIDs[cpmh.get()->getMessageRequest().header(h_CallID).value()] = MsgCmdType_Keepalive;
            }
            cpmh.get()->page(std::move(content));
            uaState->lastSendHeartTime = time(0);
        }
    }
#ifdef MESSAGE_FROM_TO_DOMAIN
    Uri from = cpmh.get()->getMessageRequest().header(h_From).uri();
    if (from.user().size() > 8)
        cpmh.get()->getMessageRequest().header(h_From).uri().host() = from.user().substr(0, 8);
    Uri to = cpmh.get()->getMessageRequest().header(h_To).uri();
    if (to.user().size() > 8)
        cpmh.get()->getMessageRequest().header(h_To).uri().host() = to.user().substr(0, 8);
#endif
}
MsgCmdType CUserMessageMrg::PopMsgTypeByCallID(const Data& callID)
{
    CUSTORLOCKGUARD locker(CallIdmapMtx);
    map<Data, MsgCmdType>::iterator iter = MsgCallIDs.find(callID);
    if (iter != MsgCallIDs.end())
    {
        MsgCmdType MsgType = iter->second;
        MsgCallIDs.erase(iter);
        return MsgType;
    }
    return MsgCmdType_unknown;
}
bool CUserMessageMrg::findMsgCallID(const Data& callID)
{
    CUSTORLOCKGUARD locker(CallIdmapMtx);
    if (MsgCallIDs.find(callID) == MsgCallIDs.end())
    {
        return false;
    }
    return true;
}
void CUserMessageMrg::GetFromuserByMessage(const  SipMessage& message, Uri& fromuser)
{
    Data dfromuser;
    if (!message.header(h_Vias).empty())
    {
        oDataStream dataStream(dfromuser);
        dataStream << message.header(h_Vias).front().protocolName();
        dataStream << ":";
        dataStream << message.header(h_From).uri().user();
        dataStream << "@";
        if (message.header(h_Vias).front().exists(p_received))
        {

            dataStream << message.header(h_Vias).front().param(p_received);
            dataStream << ":";
            dataStream << message.header(h_Vias).front().param(p_rport).port();
        }
        else
        {
            dataStream << message.header(h_Vias).front().sentHost();
            dataStream << ":";
            dataStream << message.header(h_Vias).front().sentPort();
        }
    }
    else
    {
        oDataStream dataStream(dfromuser);
        dataStream << "sip:";
        dataStream << message.header(h_From).uri().getAor();
    }
    fromuser = Uri(dfromuser);
}
resip::Data CUserMessageMrg::SendResponsePageMsg(const Uri& target, const string& sContent, const MsgCmdType& MsgCmdType, const NameAddrs& Routlist)
{
    ClientPagerMessageHandle cpmh = mDum.makePagerMessage(NameAddr(target));
    //cpmh.get()->getMessageRequest().header(h_From).uri().user() = target.user();
    unique_ptr<Contents> content(new PlainContents(sContent.c_str(), Mime("Application", "MANSCDP+xml")));
    if (Routlist.empty())
    {
    }
    else
    {
        cpmh.get()->getMessageRequest().header(h_Routes) = Routlist;
        //cpmh.get()->getMessageRequest().header(h_Vias).front().sentHost() = "";
        //cpmh.get()->getMessageRequest().header(h_Vias).front().sentPort() = 0
    }
    {
        CUSTORLOCKGUARD locker(CallIdmapMtx);
        MsgCallIDs[cpmh.get()->getMessageRequest().header(h_CallID).value()] = MsgCmdType;
    }
    cpmh.get()->page(std::move(content));
    return cpmh.get()->getMessageRequest().header(h_CallID).value();
}
void CUserMessageMrg::SendRequestPageMsg(const Uri& target, const string& sContent, const MsgCmdType& MsgCmdType)
{
    ClientPagerMessageHandle cpmh = mDum.makePagerMessage(NameAddr(target));
    unique_ptr<Contents> content(new PlainContents(sContent.c_str(), Mime("Application", "MANSCDP+xml")));
    {
        CUSTORLOCKGUARD locker(CallIdmapMtx);
        MsgCallIDs[cpmh.get()->getMessageRequest().header(h_CallID).value()] = MsgCmdType;
    }
    cpmh.get()->page(std::move(content));
    if (MsgCmdType == MsgCmdType_Catalog)
    {
        IDeviceMngrSvr& devmng = GetIDeviceMngr();
        std::shared_ptr<Device> pDev = devmng.queryDevice(target.user().c_str());
        if (pDev)
        {
            SipServerDeviceInfo* devinfo = dynamic_cast<SipServerDeviceInfo*>(pDev.get());
            if (devinfo)
            {
                devinfo->setCatalogStatus(SipServerDeviceInfo::CatalogDataStatus::ready);
            }
        }
    }
}
CatalogDateQuery* CUserMessageMrg::GetCatalogQuery(string deviceId)
{
    CUSTORLOCKGUARD locker(CatalogmapMtx);
    auto iter = mCatalogQueryMap.find(deviceId);
    if (iter != mCatalogQueryMap.end())
    {
        return iter->second;
    }
    return NULL;
}
void CUserMessageMrg::PopCatalogQuery(string deviceId)
{
    CUSTORLOCKGUARD locker(CatalogmapMtx);
    auto iter = mCatalogQueryMap.find(deviceId);
    if (iter != mCatalogQueryMap.end())
    {
        delete iter->second;
        mCatalogQueryMap.erase(deviceId);
    }
    return;
}

void CUserMessageMrg::GetAndReFormateRecordRoute(const SipMessage& outgoing, NameAddrs& Routlist)
{
    if (outgoing.exists(h_RecordRoutes) && !outgoing.const_header(h_RecordRoutes).empty())
    {
        Routlist = outgoing.header(h_RecordRoutes);
        NameAddr& rr = Routlist.front();
        if (rr.uri().host().empty())
        {
            rr.uri().host() = Tuple::inet_ntop(outgoing.getSource());
            rr.uri().port() = outgoing.getSource().getPort();
        }
        else
        {
            if (outgoing.exists(h_Vias) && outgoing.header(h_Vias).front().exists(p_received))
            {
                rr.uri().host() = outgoing.header(h_Vias).front().param(p_received);
                rr.uri().port() = outgoing.header(h_Vias).front().param(p_rport).port();
            }
        }
    }
}
void CUserMessageMrg::PtzControlResponseTask(const char* deviceId, PTZCMDType PtzCmd, int ControlPriority)
{
    BaseChildDevice *child = DeviceMng::Instance().findChildDevice(deviceId);
    if (child)
    {
        auto parentDev = child->getParentDev();
        if (parentDev || parentDev->devType == BaseDevice::JSON_NVR)
        {
            auto JsonNvr = std::dynamic_pointer_cast<JsonNvrDevic>(parentDev);
            if (JsonNvr)
            {
                JsonChildDevic* jsonChild = dynamic_cast<JsonChildDevic*>(child);
                if (jsonChild)
                {
                    int channel = jsonChild->getChannel();
                    if (channel > 0 && jsonChild->getStatus())
                    {
                        int lrArg, udarg;
                        int cmd = PtzCmd.GetPTZUDLREx(lrArg, udarg);
                        if (cmd > 0)
                        {
                            int err = 0;
                            int arg = lrArg > 0 ? lrArg : udarg;
                            JsonNvr->Dev_PTZCtrl(channel, JsonNvrDevic::switchFromGB28181((PTZCMDType::GB28181PtzCmd)cmd), arg, err);
                            if (err != 0)
                            {
                                printf(" ptz ctrl err:%d, arg:%d\n", err, arg);
                            }
                        }
                        else
                        {
                            printf("ptz cmd:%d channel:%d status:%d\n", cmd, channel, jsonChild->getStatus());
                        }
                    }
                    else
                    {
                        printf("json channel:%d status:%d\n", channel, jsonChild->getStatus());
                    }
                }
                else
                {
                    printf("jsonChild is null\n");
                }
            }
        }
    }
    
}
void CUserMessageMrg::CatalogQueryResponseTask(const Uri target, const std::string user, const uint32_t sn, const NameAddrs Routlist)
{
    std::vector<VirtualOrganization> vcList;
    std::vector<BaseChildDevice*> vChildList;

    DeviceMng::Instance().getVirtualOrganization(user, vcList);
    DeviceMng::Instance().getChildDevice(user, vChildList);

    int voSize = vcList.size();
    int childSize = vChildList.size();
    int sumnum = voSize + childSize;
    if (sumnum > 0)
    {
        std::vector<std::string> sendMsgVc;
        uint32_t itemNum = 1, senderNum = 0;
        std::vector<VirtualOrganization> vov;
        for (uint32_t i = 0; i < voSize; i++)
        {
            if (senderNum <= 10)
            {
                std::string outStr;
                vov.push_back(vcList[i]);
                if (vov.size() >= itemNum)
                {
                    CreateVirtualOrganizationCatalogResponse(user.c_str(), sn, sumnum, vov, outStr);
                    sendMsgVc.push_back(outStr);
                    vov.clear();
                }
            }
            else
            {
                vov.push_back(vcList[i]);
                if (vov.size() > 2 * itemNum)
                {
                    std::string outStr;
                    CreateVirtualOrganizationCatalogResponse(user.c_str(), sn, sumnum, vov, outStr);
                    sendMsgVc.push_back(outStr);
                    vov.clear();
                }
                else if(voSize - i < 2 * itemNum)
                {
                    std::string outStr;
                    CreateVirtualOrganizationCatalogResponse(user.c_str(), sn, sumnum, vov, outStr);
                    sendMsgVc.push_back(outStr);
                    vov.clear();
                }
            }
        }
        std::vector<CatalogItem> ChildTtems;
        for (uint32_t i = 0; i < childSize; i++)
        {
            BaseDevice::Ptr baseDev = vChildList[i]->getParentDev();
            if (baseDev && vChildList[i] && baseDev->devType == BaseDevice::JSON_NVR)
            {
                JsonChildDevic * pChildDev = dynamic_cast<JsonChildDevic*>(vChildList[i]);
                if (pChildDev)
                {
                    CatalogItem item;
                    item.DeviceID = pChildDev->getDeviceId();
                    item.ParentID = pChildDev->getParentId();
                    item.Name = Utf8ToGbk(pChildDev->getName());

                    item.Manufacturer = "VSK";//当为设备时,设备厂商(必选)
                    item.Model = "";//当为设备时,设备型号(必选)
                    item.Owner = "";//当为设备时,设备归属(必选)
                    item.CivilCode = item.DeviceID.substr(0, 6);//行政区域(必选)
                    item.Block = "";//警区(可选)
                    item.Address = "";//当为设备时,安装地址(必选)
                    item.Parental = 0;//当为设备时,是否有子设备(必选)1有,0没有
                    item.SafetyWay = 0;//信令安全模式(可选)缺省为0; 0:不采用;2:S/MIME 签名方式;3:S/MIME加密签名同时采用方式; 4:数字摘要方式
                    item.RegisterWay = 1;//注册方式(必选)缺省为1;1:符合IETFRFC3261标准的认证注册模式; 2:基于口令的双向认证注册模式; 3:基于数字证书的双向认证注册模式
                    item.CertNum = 0;//证书序列号(有证书的设备必选)
                    item.Certifiable = 0;//证书有效标识(有证书的设备必选)缺省为0;证书有效标识:0:无效 1:有效
                    item.ErrCode = 0;//无效原因码(有证书且证书无效的设备必选)
                    item.EndTime = "";//证书终止有效期(有证书的设备必选)
                    item.Secrecy = 0;//保密属性(必选)缺省为0;0:不涉密,1:涉密
                    item.IPAddress = pChildDev->getChildIp();//设备/区域/系统IP地址(可选)
                    item.Port = 0;//设备/区域/系统端口(可选)
                    item.Password = "";
                    item.Status = pChildDev->getStatus() ? "ON" : "OFF";
                    item.Longitude = 0;//经度(可选)
                    item.Latitude = 0;//纬度(可选)
                    ChildTtems.push_back(item);
                }
            }
            if (ChildTtems.size() > 2 * itemNum)
            {
                std::string outStr;
                CreateCatalogResponse(user.c_str(), sn, sumnum, ChildTtems, NULL, outStr);
                sendMsgVc.push_back(outStr);
                ChildTtems.clear();
            }
            else if(childSize - i < 2 * itemNum)
            {
                std::string outStr;
                CreateCatalogResponse(user.c_str(), sn, sumnum, ChildTtems, NULL, outStr);
                sendMsgVc.push_back(outStr);
                ChildTtems.clear();
            }
        }
        for (auto& it : sendMsgVc)
        {
            resip::Data callId = SendResponsePageMsg(target, it, MsgCmdType_Catalog, Routlist);
            while (findMsgCallID(callId))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
}
void CUserMessageMrg::DeviceInfoQueryResponseTask(const Uri target, const std::string user, const uint32_t sn, const NameAddrs Routlist)
{
    if (user == DeviceMng::Instance().getSelfId())
    {
        DeviceInfoMsg devInfoMsg;
        devInfoMsg.DeviceName = "sipserver";
        devInfoMsg.Result = "OK";
        devInfoMsg.Manufacturer = "vsk";
        devInfoMsg.Model = "901";
        devInfoMsg.Firmware = "1.1.9";
        devInfoMsg.Channel = 1;
        string outStr;
        CreateDeviceInfoResponse(user.c_str(), sn, devInfoMsg, outStr);
        SendResponsePageMsg(target, outStr, MsgCmdType_DeviceInfo, Routlist);
    }
}
uint32_t CUserMessageMrg::getMsgId()
{
    return mMsgSn++;
}