#include "UaMessageMgr.h"
#include "resip/stack/PlainContents.hxx"
#include "../MsgContentXml.h"
#include "../XmlMsgAnalysis.h"
#include "../device/DeviceManager.h"
#include "../tools/genuuid.h"
#include "../tools/CodeConvert.h"
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
    shared_ptr<SipMessage> ok;
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

    if (XmlMsg.cmdname == XML_CMD_NAME_QUERY)
    {
        if (XmlMsg.cmdtype == XML_CMDTYPE_REQUEST_CATALOG)
        {
            //青岛

            //QDCCTVCatalogResponse(fromuser, XmlMsg.DeviceID.c_str(), XmlMsg.sn, routelist);
            
            IDeviceMngrSvr& devmng = GetIDeviceMngr();
            if (Data(XmlMsg.DeviceID) == mDum.getMasterUserProfile()->getDefaultFrom().uri().user())
            {
                //青岛
                {
                    ThreadPool::Instance().submit(std::bind(&CUserMessageMrg::QueryCatalogTask, this, fromuser, "", XmlMsg.sn, routelist));
                }
                list<std::shared_ptr<Device>> devlist;
                devmng.GetAllDeviceList(devlist);
                int sumnum = devlist.size();
                for (auto& it : devlist)
                {
                    SipServerDeviceInfo* devinfo = dynamic_cast<SipServerDeviceInfo*>(it.get());
                    if (devinfo)
                    {
                        sumnum += devinfo->getChannelCount();
                    }
                }
#ifdef SENDONLY_CHANNEL_CATALOG
                for (auto& it : devlist)
                {
                    SipServerDeviceInfo* devinfo = dynamic_cast<SipServerDeviceInfo*>(it);
                    if (devinfo)
                    {
                        list<GBDeviceChannel*> chllist = devinfo->getChannelList();
                        for (auto& it : chllist)
                        {
                            vector<CatalogItem> items;
                            CatalogItem item;
                            item.DeviceID = it->getChannelId();
                            item.ParentID = it->getDeviceId();
                            item.Name = "ipc";
                            item.Status = "ON";

                            item.DeviceID = it->getChannelId();
                            item.ParentID = it->getDeviceId();
                            item.Name = it->getChannelId();
                            item.Manufacturer = it->getManufacture();
                            item.Model = it->getModel();
                            item.Owner = it->getOwner();
                            item.CivilCode = it->getChannelId().substr(0, 10);
                            item.Block = it->getBlock();
                            item.Address = it->getAddress();
                            item.Parental = 0;
                            item.SafetyWay = 0;
                            item.RegisterWay = 1;
                            item.CertNum = 0;
                            item.Certifiable = 0;
                            item.ErrCode = 0;
                            item.EndTime = "";
                            item.Secrecy = 0;
                            item.IPAddress = it->getIpAddress();
                            item.Port = it->getPort();
                            item.Password = "";
                            item.Status = "ON";
                            item.Longitude = 0;
                            item.Latitude = 0;
                            items.push_back(item);
                            string outStr;
                            CreateCatalogResponse(XmlMsg.DeviceID.c_str(), XmlMsg.sn, sumnum, items, NULL, outStr);
                            unique_ptr<Contents> content(new PlainContents(outStr.c_str(), Mime("Application", "MANSCDP+xml")));
                            //h->send
                            SendResponsePageMsg(fromuser, outStr, MsgCmdType_Catalog, routelist);
                        }
                    }
                }
#else
                for (auto& it : devlist)
                {
                    SipServerDeviceInfo* devinfo = dynamic_cast<SipServerDeviceInfo*>(it.get());
                    if (devinfo)
                    {
                        vector<CatalogItem> items;
                        CatalogItem item;
                        item.DeviceID = devinfo->getDeviceId();
                        item.ParentID = mDum.getMasterUserProfile()->getDefaultFrom().uri().user().c_str();
                        item.Name = devinfo->getDeviceId();
                        item.Manufacturer = devinfo->getManufacturer();
                        item.Model = devinfo->getModel();
                        item.Owner = "";
                        item.CivilCode = devinfo->getDeviceId().substr(0, 10);
                        item.Block = "";
                        item.Address = "";
                        item.Parental = devinfo->getChannelCount() ? 1 : 0;
                        item.SafetyWay = 0;
                        item.RegisterWay = 1;
                        item.CertNum = 0;
                        item.Certifiable = 0;
                        item.ErrCode = 0;
                        item.EndTime = "";
                        item.Secrecy = 0;
                        item.IPAddress = devinfo->getIp();
                        item.Port = devinfo->getPort();
                        item.Password = "";
                        item.Status = "ON";
                        item.Longitude = 0;
                        item.Latitude = 0;
                        items.push_back(item);
                        string outStr;
                        CreateCatalogResponse(XmlMsg.DeviceID.c_str(), XmlMsg.sn, sumnum, items, NULL, outStr);
                        unique_ptr<Contents> content(new PlainContents(outStr.c_str(), Mime("Application", "MANSCDP+xml")));
                        //h->send
                        SendResponsePageMsg(fromuser, outStr, MsgCmdType_Catalog, routelist);
                        list<std::shared_ptr<IDeviceChannel>> chllist = devinfo->getChannelList();
                        for (auto& iter : chllist)
                        {
                            std::shared_ptr<GBDeviceChannel> chlinfo = std::static_pointer_cast<GBDeviceChannel>(iter);
                            if (chlinfo)
                            {
                                vector<CatalogItem> items;
                                CatalogItem item;
                                item.DeviceID = chlinfo->getChannelId();
                                item.ParentID = chlinfo->getDeviceId();
                                item.Name = "ipc";
                                item.Status = "ON";

                                item.DeviceID = chlinfo->getChannelId();
                                item.ParentID = chlinfo->getDeviceId();
                                item.Name = chlinfo->getChannelId();
                                item.Manufacturer = chlinfo->getManufacture();
                                item.Model = chlinfo->getModel();
                                item.Owner = chlinfo->getOwner();
                                item.CivilCode = chlinfo->getChannelId().substr(0, 10);
                                item.Block = chlinfo->getBlock();
                                item.Address = chlinfo->getAddress();
                                item.Parental = 0;
                                item.SafetyWay = 0;
                                item.RegisterWay = 1;
                                item.CertNum = 0;
                                item.Certifiable = 0;
                                item.ErrCode = 0;
                                item.EndTime = "";
                                item.Secrecy = 0;
                                item.IPAddress = chlinfo->getIpAddress();
                                item.Port = chlinfo->getPort();
                                item.Password = "";
                                item.Status = "ON";
                                item.Longitude = 0;
                                item.Latitude = 0;
                                items.push_back(item);
                                string outStr;
                                CreateCatalogResponse(XmlMsg.DeviceID.c_str(), XmlMsg.sn, sumnum, items, NULL, outStr);
                                unique_ptr<Contents> content(new PlainContents(outStr.c_str(), Mime("Application", "MANSCDP+xml")));
                                //h->send
                                SendResponsePageMsg(fromuser, outStr, MsgCmdType_Catalog, routelist);
                            }
                        }
                    }
                }
#endif
                
            }
            else
            {
                std::shared_ptr<Device> pDev = devmng.queryDevice(XmlMsg.DeviceID);
                if (pDev)
                {
                    SipServerDeviceInfo* devinfo = dynamic_cast<SipServerDeviceInfo*>(pDev.get());
                    if (devinfo)
                    {
                        list<std::shared_ptr<IDeviceChannel>> chllist = devinfo->getChannelList();
                        int sumnum = chllist.size();
                        for (auto& it : chllist)
                        {
                            std::shared_ptr<GBDeviceChannel> chlInfo = std::static_pointer_cast<GBDeviceChannel>(it);
                            if (chlInfo)
                            {
                                vector<CatalogItem> items;
                                CatalogItem item;
                                item.DeviceID = chlInfo->getChannelId();
                                item.ParentID = chlInfo->getDeviceId();
                                item.Name = "ipc";
                                item.Status = "ON";

                                item.DeviceID = chlInfo->getChannelId();
                                item.ParentID = chlInfo->getDeviceId();
                                item.Name = chlInfo->getChannelId();
                                item.Manufacturer = chlInfo->getManufacture();
                                item.Model = chlInfo->getModel();
                                item.Owner = chlInfo->getOwner();
                                item.CivilCode = chlInfo->getChannelId().substr(0, 10);
                                item.Block = chlInfo->getBlock();
                                item.Address = chlInfo->getAddress();
                                item.Parental = 0;
                                item.SafetyWay = 0;
                                item.RegisterWay = 1;
                                item.CertNum = 0;
                                item.Certifiable = 0;
                                item.ErrCode = 0;
                                item.EndTime = "";
                                item.Secrecy = 0;
                                item.IPAddress = chlInfo->getIpAddress();
                                item.Port = chlInfo->getPort();
                                item.Password = "";
                                item.Status = "ON";
                                item.Longitude = 0;
                                item.Latitude = 0;
                                items.push_back(item);
                                string outStr;
                                CreateCatalogResponse(XmlMsg.DeviceID.c_str(), XmlMsg.sn, sumnum, items, NULL, outStr);
                                unique_ptr<Contents> content(new PlainContents(outStr.c_str(), Mime("Application", "MANSCDP+xml")));
                                //h->send
                                SendResponsePageMsg(fromuser, outStr, MsgCmdType_Catalog, routelist);
                            }
                        }
                    }
                }
            }
        }
        else if (XmlMsg.cmdtype == XML_CMDTYPE_REQUEST_DEVICEINFO)
        {
            if (XmlMsg.DeviceID == "34020000002000000001")
            {
                DeviceInfoMsg devInfoMsg;
                devInfoMsg.DeviceName = "sipserver";
                devInfoMsg.Result = "OK";
                devInfoMsg.Manufacturer = "vsk";
                devInfoMsg.Model = "901";
                devInfoMsg.Firmware = "1.1.9";
                devInfoMsg.Channel = 1;
                string outStr;
                CreateDeviceInfoResponse(XmlMsg.DeviceID.c_str(), XmlMsg.sn, devInfoMsg, outStr);
                SendResponsePageMsg(fromuser, outStr, MsgCmdType_DeviceInfo, routelist);
            }
        }
    }
    else if(XmlMsg.cmdname == XML_CMD_NAME_NOTIFY)
    {
        if (XmlMsg.cmdtype == XML_CMDTYPE_REQUEST_KEEPALIVE)
        {

        }
    }
    else if (XmlMsg.cmdname == XML_CMD_NAME_RESPONSE)
    {
        if (XmlMsg.cmdtype == XML_CMDTYPE_RESPONSE_CATALOG)
        {
            IDeviceMngrSvr& devmng = GetIDeviceMngr();
            std::shared_ptr<Device> pDev = devmng.queryDevice(XmlMsg.DeviceID);
            if (pDev)
            {
                std::shared_ptr<SipServerDeviceInfo> devinfo = std::static_pointer_cast<SipServerDeviceInfo>(pDev);
                if (devinfo)
                {
                    ResponseCatalogList* pResponseCatalogList = (ResponseCatalogList*)XmlMsg.pPoint;
                    if (devinfo->getCatalogStatus() == SipServerDeviceInfo::ready)
                    {
                        devinfo->setCatalogStatus(SipServerDeviceInfo::runIng);
                        devinfo->setTotal(pResponseCatalogList->allnum);
                    }
                    for (auto& it : pResponseCatalogList->m_devVect)
                    {
                        std::shared_ptr<GBDeviceChannel> pChannel = std::static_pointer_cast<GBDeviceChannel>(devinfo->GetGBChannel(it.DeviceID));
                        //GBDeviceChannel* pChannel = dynamic_cast<GBDeviceChannel*>(devinfo->GetGBChannel(it.DeviceID).get());
                        if (pChannel)
                        {
                            //update
                            GBDeviceChannel* pGbChannel = new GBDeviceChannel();
                            pGbChannel->setChannelId(it.DeviceID);
                            pGbChannel->setName(it.Name);
                            pGbChannel->setManufacture(it.Manufacturer);
                            pGbChannel->setModel(it.Model);
                            pGbChannel->setOwner(it.Owner);
                            pGbChannel->setCivilCode(it.CivilCode);
                            pGbChannel->setBlock(it.Block);
                            pGbChannel->setAddress(it.Address);
                            pGbChannel->setParental(it.Parental);
                            pGbChannel->setParentId(it.ParentID);
                            pGbChannel->setSafetyWay(it.SafetyWay);
                            pGbChannel->setRegisterWay(it.RegisterWay);
                            pGbChannel->setCertNum(it.CertNum);
                            pGbChannel->setCertifiable(it.Certifiable);
                            pGbChannel->setErrCode(it.ErrCode);
                            pGbChannel->setEndTime(it.EndTime);
                            pGbChannel->setSecrecy(std::to_string(it.Secrecy));
                            pGbChannel->setIpAddress(it.IPAddress);
                            pGbChannel->setPort(it.Port);
                            pGbChannel->setPassword(it.Password);
                            pGbChannel->setStatus(it.Status);
                            pGbChannel->setLongitude(it.Longitude);
                            pGbChannel->setLatitude(it.Latitude);
                            pGbChannel->setDeviceId(devinfo->getDeviceId());
                            devinfo->ResponseUpdateCatalog(pGbChannel);
                            //devmng.GetGBDeviceMapper().GetGBDeviceChannelMapper().update(pGbChannel);
                        }
                        else
                        {
                            //add
                            GBDeviceChannel *pGbChannel = new GBDeviceChannel();
                            pGbChannel->setUuid(imuuid::uuidgen());
                            pGbChannel->setChannelId(it.DeviceID);
                            pGbChannel->setName(it.Name);
                            pGbChannel->setManufacture(it.Manufacturer);
                            pGbChannel->setModel(it.Model);
                            pGbChannel->setOwner(it.Owner);
                            pGbChannel->setCivilCode(it.CivilCode);
                            pGbChannel->setBlock(it.Block);
                            pGbChannel->setAddress(it.Address);
                            pGbChannel->setParental(it.Parental);
                            pGbChannel->setParentId(it.ParentID);
                            pGbChannel->setSafetyWay(it.SafetyWay);
                            pGbChannel->setRegisterWay(it.RegisterWay);
                            pGbChannel->setCertNum(it.CertNum);
                            pGbChannel->setCertifiable(it.Certifiable);
                            pGbChannel->setErrCode(it.ErrCode);
                            pGbChannel->setEndTime(it.EndTime);
                            pGbChannel->setSecrecy(std::to_string(it.Secrecy));
                            pGbChannel->setIpAddress(it.IPAddress);
                            pGbChannel->setPort(it.Port);
                            pGbChannel->setPassword(it.Password);
                            pGbChannel->setStatus(it.Status);
                            pGbChannel->setLongitude(it.Longitude);
                            pGbChannel->setLatitude(it.Latitude);
                            pGbChannel->setDeviceId(devinfo->getDeviceId());
                            devinfo->addResponseCatalog(pGbChannel);
                            devmng.GetGBDeviceMapper().GetGBDeviceChannelMapper().add(pGbChannel);
                        }
                    }
                    if (devinfo->getTotal() == devinfo->getCatalogNum())
                    {
                        devinfo->setCatalogStatus(SipServerDeviceInfo::end);
                    }
                    if (devinfo->getCatalogStatus() == SipServerDeviceInfo::end)
                    {
                        devinfo->setChannelCount(devinfo->getTotal());
                        IDeviceMngrSvr& devmng = GetIDeviceMngr();
                        devmng.sync(devinfo);
                    }
                }
            }
        }
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
std::string CUserMessageMrg::GetDeviceList()
{
    return GetRequest("http://192.168.1.21:8080/device/gbInfo");
    //return PostRequest("http://192.168.1.21:8080/device/gbInfo", "");
}
int CUserMessageMrg::parseDeviceList(std::string devline, std::vector<VirtualOrganization> voVc, std::vector<CatalogItem>& devList)
{
    if (devline.empty())
        return 0;
    int count = 0;
    rapidjson::Document document;
    document.Parse((char*)devline.c_str());
    if (!document.HasParseError())
    {
        if (document.HasMember("data") && document["data"].IsObject())
        {
            rapidjson::Value& body = document["data"];
            count = json_check_int32(body, "Count");
            if (body.HasMember("Data") && body["Data"].IsArray())
            {
                rapidjson::Value& msbody = body["Data"];
                for (size_t i = 0; i < msbody.Size(); i++)
                {
                    VirtualOrganization vo;
                    vo.Name = json_check_string(msbody[i], "Title");
                    vo.DeviceID = json_check_string(msbody[i], "GBId");
                    int port = json_check_int32(msbody[i], "ManagePort");
                    std::string user = json_check_string(msbody[i], "ManageUser");
                    std::string pass = json_check_string(msbody[i], "ManagePass");
                    std::string devIp = json_check_string(msbody[i], "ManageIp");
                    voVc.push_back(vo);
                    if (msbody[i].HasMember("Upward") && msbody[i]["Upward"].IsObject())
                    {
                        rapidjson::Value& upbody = msbody[i]["Upward"];
                        VirtualOrganization subVo;
                        subVo.Name = json_check_string(upbody, "Title");
                        subVo.DeviceID = json_check_string(upbody, "GBId");
                        subVo.ParentID = vo.DeviceID;
                        voVc.push_back(subVo);
                        if (upbody.HasMember("Data") && upbody["Data"].IsArray())
                        {
                            rapidjson::Value& ipcbody = upbody["Data"];
                            for (size_t j = 0; j < ipcbody.Size(); j++)
                            {
                                CatalogItem item;
                                item.Name = json_check_string(ipcbody[j], "ipc");
                                item.DeviceID = json_check_string(ipcbody[j], "GBId");
                                int ipcStatus = json_check_int32(ipcbody[j], "status");
                                item.Status = (ipcStatus ? "OFF" : "ON");
                                item.ParentID = subVo.DeviceID;
                                devList.push_back(item);
                            }
                        }
                    }
                    if (msbody[i].HasMember("Downward") && msbody[i]["Downward"].IsObject())
                    {
                        rapidjson::Value& downbody = msbody[i]["Downward"];
                        VirtualOrganization subVo;
                        subVo.Name = json_check_string(downbody, "Title");
                        subVo.DeviceID = json_check_string(downbody, "GBId");
                        subVo.ParentID = vo.DeviceID;
                        voVc.push_back(subVo);
                        if (downbody.HasMember("Data") && downbody["Data"].IsArray())
                        {
                            rapidjson::Value& ipcbody = downbody["Data"];
                            for (size_t j = 0; j < ipcbody.Size(); j++)
                            {
                                CatalogItem item;
                                item.Name = json_check_string(ipcbody[j], "ipc");
                                item.DeviceID = json_check_string(ipcbody[j], "GBId");
                                int ipcStatus = json_check_int32(ipcbody[j], "status");
                                item.Status = (ipcStatus ? "OFF" : "ON");
                                item.ParentID = subVo.DeviceID;
                                devList.push_back(item);
                            }
                        }
                    }
                    if (msbody[i].HasMember("KKIpc") && msbody[i]["KKIpc"].IsObject())
                    {
                        rapidjson::Value& kkbody = msbody[i]["KKIpc"];
                        VirtualOrganization subVo;
                        subVo.Name = json_check_string(kkbody, "Title");
                        subVo.DeviceID = json_check_string(kkbody, "GBId");
                        subVo.ParentID = vo.DeviceID;
                        voVc.push_back(subVo);
                        if (kkbody.HasMember("Data") && kkbody["Data"].IsArray())
                        {
                            rapidjson::Value& ipcbody = kkbody["Data"];
                            for (size_t j = 0; j < ipcbody.Size(); j++)
                            {
                                CatalogItem item;
                                item.Name = json_check_string(ipcbody[j], "ipc");
                                item.DeviceID = json_check_string(ipcbody[j], "GBId");
                                int ipcStatus = json_check_int32(ipcbody[j], "status");
                                item.Status = (ipcStatus ? "OFF" : "ON");
                                item.ParentID = subVo.DeviceID;
                                devList.push_back(item);
                            }
                        }
                    }

                }
            }
        }
        return count;
    }
    return 0;
}
void CUserMessageMrg::QDCCTVCatalogResponse(const Uri& target, const char* user, const uint32_t& sn, const NameAddrs& Routlist)
{
    std::string dirstr = GetDeviceList();
    std::list<std::shared_ptr<Device>> devlist;
    std::vector<VirtualOrganization> voList;
    std::vector<CatalogItem> voItem;
    int SumNum = parseDeviceList(dirstr, voList, voItem);
    //ok->setBody();
    if (SumNum > 0)
    {
        auto vbegin = voList.begin();
        for (; vbegin != voList.end();)
        {
            std::vector<VirtualOrganization> vov(vbegin, ++vbegin);
            std::string outStr;
            CreateVirtualOrganizationCatalogResponse(user, sn, SumNum, vov, outStr);
            SendResponsePageMsg(target, outStr, MsgCmdType_Catalog, Routlist);
        }
        auto vcbegin = voItem.begin();
        for (; vcbegin != voItem.end();)
        {
            std::vector<CatalogItem> vov(vcbegin, ++vcbegin);
            std::string outStr;
            CreateCatalogResponse(user, sn, SumNum, vov, NULL, outStr);
            SendResponsePageMsg(target, outStr, MsgCmdType_Catalog, Routlist);
        }
    }
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
void CUserMessageMrg::QueryCatalogTask(const Uri& target, const char* user, const uint32_t& sn, const NameAddrs& Routlist)
{
    std::vector<VirtualOrganization> vcList;
    std::vector<BaseChildDevice*> vChildList;
#ifdef QINGDONG_CCTC
    DeviceMng::Instance().getVirtualOrganization("", vcList);
    DeviceMng::Instance().getChildDevice("", vChildList);
#endif
    int sumnum = vcList.size() + vChildList.size();
    if (sumnum > 0)
    {
        std::vector<std::string> sendMsgVc;
        uint32_t itemNum = 1, senderNum = 0;
        std::vector<VirtualOrganization> vov;
        for (uint32_t i = 0; i < vcList.size(); i++)
        {
            if (senderNum <= 10)
            {
                std::string outStr;
                vov.push_back(vcList[i]);
                if (vov.size() >= itemNum)
                {
                    CreateVirtualOrganizationCatalogResponse(user, sn, sumnum, vov, outStr);
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
                    CreateVirtualOrganizationCatalogResponse(user, sn, sumnum, vov, outStr);
                    sendMsgVc.push_back(outStr);
                    vov.clear();
                }
            }
        }
        std::vector<CatalogItem> ChildTtems;
        for (uint32_t i = 0; i < vChildList.size(); i++)
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
                    item.Name = ownCodeCvt::Utf8ToGbk(pChildDev->getName());

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
                CreateCatalogResponse(user, sn, sumnum, ChildTtems, NULL, outStr);
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