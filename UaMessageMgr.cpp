#include "UaMessageMgr.h"
#include "resip/stack/PlainContents.hxx"
#include "MsgContentXml.h"
#include "XmlMsgAnalysis.h"
#include "device/DeviceManager.h"
#include "tools/genuuid.h"


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
            IDeviceMngrSvr& devmng = GetIDeviceMngr();
            if (Data(XmlMsg.DeviceID) == mDum.getMasterUserProfile()->getDefaultFrom().uri().user())
            {
                list<std::shared_ptr<Device>> devlist;
                int sumnum = 0;
                devmng.GetAllDeviceList(devlist);
                sumnum = devlist.size();
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
void CUserMessageMrg::SendResponsePageMsg(const Uri& target, const string& sContent, const MsgCmdType& MsgCmdType, const NameAddrs& Routlist)
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