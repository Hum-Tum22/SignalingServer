#include "ws.h"
#include "../media/mediaIn/JsonStream.h"
#include "../deviceMng/JsonDevice.h"
#include "../deviceMng/deviceMng.h"
#include "SelfLog.h"

#include <functional>
#include <chrono>

static const char* webs_Play_class = "Play";
static const char* web_RealTime_method = "Preview";
static const char* webs_PlayBack_method = "Playback";
static const char* webs_PBControl_method = "PlaybackControl";
static const char* webs_Download_class = "Download";
static const char* webs_Download_Raw_method = "Download_RAW";

WsServer::WsServer(short port) :num_threads(1)
{
    try
    {
        num_threads = std::thread::hardware_concurrency();
        if(num_threads / 6 > 0)
        {
            num_threads = num_threads / 6;
        }
        LogOut("BLL", L_DEBUG, "cpu num:%zu", num_threads);
        // Total silence
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.clear_error_channels(websocketpp::log::alevel::all);

        // Initialize ASIO
        m_endpoint.init_asio();
        m_endpoint.set_reuse_addr(true);

        // Register our message handler
        m_endpoint.set_message_handler(std::bind(&WsServer::on_message, this, &m_endpoint, std::placeholders::_1, std::placeholders::_2));
        m_endpoint.set_open_handler(bind(&WsServer::on_open, this, std::placeholders::_1));
        m_endpoint.set_close_handler(bind(&WsServer::on_close, this, std::placeholders::_1));

        m_endpoint.set_socket_init_handler(std::bind(&WsServer::on_socket_init, this, std::placeholders::_1, std::placeholders::_2));

        // Listen on specified port with extended listen backlog
        m_endpoint.set_listen_backlog(8192);
        m_endpoint.listen(port);

        // Start the server accept loop
        m_endpoint.start_accept();

        // Start the ASIO io_service run loop
        for(size_t i = 0; i < num_threads; i++)
        {
            ts.push_back(websocketpp::lib::make_shared<websocketpp::lib::thread>(&server::run, &m_endpoint));
        }
    }
    catch(websocketpp::exception const& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
}
WsServer::~WsServer()
{
    m_endpoint.stop();
    for(size_t i = 0; i < num_threads; i++)
    {
        if(ts[i] && ts[i]->joinable())
            ts[i]->join();
    }
}
void WsServer::on_open(websocketpp::connection_hdl hdl)
{
    //m_connections.insert(hdl);
    LogOut("BLL", L_DEBUG, "open connection hdl:");
}

void WsServer::on_close(websocketpp::connection_hdl hdl)
{
    WsStreamInfo* sinfo = NULL;
    {
        CUSTORLOCKGUARD clocker(conMtx);
        auto it = m_connections.find(hdl);
        if(it != m_connections.end())
        {
            sinfo = it->second;
            m_connections.erase(hdl);
        }
    }
    if(sinfo)
    {
        if(sinfo->ms)
        {
            sinfo->ms->removeReader(sinfo->readhandle);
        }
        sinfo->readhandle = 0;
        if(sinfo->t.joinable())
            sinfo->t.join();
        MediaMng::GetInstance().CloseStreamByStreamId(sinfo->ms);
        delete sinfo; sinfo = NULL;
    }
    //m_connections.erase(hdl);
    LogOut("BLL", L_DEBUG, "close connection hdl:");
}
void WsServer::on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    if(msg->get_opcode() == websocketpp::frame::opcode::text)
    {
        s->get_alog().write(websocketpp::log::alevel::app, "Text Message Received: " + msg->get_payload());
        std::string strMsg = msg->get_payload();
        WebsocketMessage(s, hdl, strMsg.c_str(), strMsg.size());
    }
    else
    {
        s->get_alog().write(websocketpp::log::alevel::app, "Binary Message Received: " + websocketpp::utility::to_hex(msg->get_payload()));
    }
    //s->send(hdl, msg->get_payload(), msg->get_opcode());
}

void WsServer::on_socket_init(websocketpp::connection_hdl, asio::ip::tcp::socket& s)
{
    asio::ip::tcp::no_delay option(true);
    if(s.is_open())
        s.set_option(option);
}
int CommonResponseParamToJson(const char* cmd, int msid, int errorCode, const char* describe, rapidjson_sip::StringBuffer& outStr)
{
    rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(outStr);
    writer.StartObject();
    writer.Key("cmd"); writer.String(cmd);
    if(msid != -1)
    {
        writer.Key("msid"); writer.Int(msid);
    }
    writer.Key("errorcode"); writer.Int(errorCode);
    if(describe)
        writer.Key("msg");writer.String(describe);
    writer.EndObject();
    return 0;
}
int WsServer::WebsocketMessage(server* s, websocketpp::connection_hdl hdl, const char* message, size_t len)
{
    if(message == NULL)
        return 0;
    rapidjson_sip::Document document;
    rapidjson_sip::ParseResult res = document.Parse(message, len);
    if(!document.IsObject() || document.HasParseError())
    {
        rapidjson_sip::StringBuffer response;
        CommonResponseParamToJson("CommonError", -1, 6, "json parse failed", response);
        //send(connection_hdl hdl, void const* payload, size_t len, frame::opcode::value op, lib::error_code & ec)
        s->send(hdl, response.GetString(), response.GetSize(), websocketpp::frame::opcode::text);
        return 0;
    }
    LogOut("BLL", L_DEBUG, "ws msg:%s", message);

    std::string classe = json_check_string(document, "classe");
    std::string strCmd = json_check_string(document, "cmd");

    //pWsTaskInfo->nCmd = 0;
    if(strcmp(webs_Play_class, classe.c_str()) == 0)
    {
        if(strcmp(web_RealTime_method, strCmd.c_str()) == 0)
        {
            rapidjson_sip::StringBuffer response;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
            auto sinfo = new WsStreamInfo();
            sinfo->IsLive = 0;
            sinfo->hdl = hdl;
            sinfo->s = s;
            int nRet = 0;
            {
                CUSTORLOCKGUARD clocker(conMtx);
                nRet = RealTimePlayAction(sinfo, document, writer);
                m_connections[hdl] = sinfo;
            }

            s->send(hdl, response.GetString(), response.GetSize(), websocketpp::frame::opcode::text);
            if(nRet == 0)
                sinfo->t = std::move(std::thread(&WsServer::RtPreviewThread, sinfo));
        }
        else if(strcmp(webs_PlayBack_method, strCmd.c_str()) == 0)
        {
            rapidjson_sip::StringBuffer response;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
            auto sinfo = new WsStreamInfo();
            sinfo->IsLive = 1;
            sinfo->hdl = hdl;
            sinfo->s = s;
            int nRet = 0;

            {
                CUSTORLOCKGUARD clocker(conMtx);
                nRet = PlayBackAction(sinfo, document, writer);
                m_connections[hdl] = sinfo;
            }
            //*(void**)c->data = cnPtr;

            s->send(hdl, response.GetString(), response.GetSize(), websocketpp::frame::opcode::text);
            if(nRet == 0)
            {
                sinfo->t = std::move(std::thread(&WsServer::PlayBackThread, sinfo));
            }
            else
            {
                LogOut("BLL", L_DEBUG, "ws msg:%s playback error:%d", message, nRet);
            }
        }
        else if(strcmp(webs_PBControl_method, strCmd.c_str()) == 0)
        {
            rapidjson_sip::StringBuffer response;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
            WsStreamInfo* msInfo = NULL;
            {
                CUSTORLOCKGUARD clocker(conMtx);
                auto it = m_connections.find(hdl);
                if(it != m_connections.end())
                {
                    msInfo = it->second;
                }
            }
            if(msInfo)
            {
                PlayBackControlAction(msInfo, document, writer);
            }
            else
            {
                writer.StartObject();
                writer.Key("classe"); writer.String(webs_Play_class);
                writer.Key("method"); writer.String(strCmd.c_str());
                writer.Key("errorcode"); writer.Int(-3);
                writer.Key("msg"); writer.String("play back control stream not found");
                writer.EndObject();
            }
            s->send(hdl, response.GetString(), response.GetSize(), websocketpp::frame::opcode::text);
        }
        else
        {
            rapidjson_sip::StringBuffer response;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
            writer.StartObject();
            writer.Key("classe"); writer.String(webs_Play_class);
            writer.Key("method"); writer.String(strCmd.c_str());
            writer.Key("errorcode"); writer.Int(-1);
            writer.Key("msg");writer.String("methed Not Supported");
            writer.EndObject();
            s->send(hdl, response.GetString(), response.GetSize(), websocketpp::frame::opcode::text);
        }
    }
    else if(strcmp(webs_Download_class, classe.c_str()) == 0)
    {
        if(strcmp(webs_Download_Raw_method, strCmd.c_str()) == 0)
        {
            rapidjson_sip::StringBuffer response;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
            //DownloadAction(connectinfo, document, writer, NULL);
            s->send(hdl, response.GetString(), response.GetSize(), websocketpp::frame::opcode::text);
        }
        else
        {
            rapidjson_sip::StringBuffer response;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
            writer.StartObject();
            writer.Key("classe"); writer.String(webs_Play_class);
            writer.Key("method"); writer.String(strCmd.c_str());
            writer.Key("errorcode"); writer.Int(-1);
            writer.Key("msg"); writer.String("methed Not Supported");
            writer.EndObject();
            s->send(hdl, response.GetString(), response.GetSize(), websocketpp::frame::opcode::text);
        }
    }
    else
    {
        const static char* pResult = "Does not support class";
        rapidjson_sip::StringBuffer response;
        rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
        writer.StartObject();
        writer.Key("classe"); writer.String(classe.c_str());
        writer.Key("errorcode"); writer.Int(-1);
        writer.Key("msg"); writer.String("class Not Supported");
        writer.EndObject();
        s->send(hdl, response.GetString(), response.GetSize(), websocketpp::frame::opcode::text);
    }
    return 0;
}
int WsServer::RealTimePlayAction(WsStreamInfo* sinfo, rapidjson_sip::Document& document, rapidjson_sip::Writer<rapidjson_sip::StringBuffer>& writer)
{
    int errcode = 0;
    if(document.HasMember("params"))
    {
        //{"streamType":0,"streamNo":0,"handle":5,}
        rapidjson_sip::Value& msbody = document["params"];
        unsigned int chl_handle = json_check_int32(msbody, "handle");
        int streamtype = json_check_uint32(msbody, "streamType");
        int streamno = json_check_int32(msbody, "streamno");
        std::string deviceId = json_check_string(msbody, "deviceId");

        std::string streamdId;
        BaseChildDevice* child = DeviceMng::Instance().findChildDeviceByCCTVDeviceId(deviceId);
        if(child == NULL)
        {
            errcode = 3;
            writer.StartObject();
            writer.Key("classe"); writer.String(webs_Play_class);
            writer.Key("method"); writer.String(web_RealTime_method);
            writer.Key("errorcode"); writer.Int(errcode);
            writer.Key("msg"); writer.String("device not found");
            writer.EndObject();
            return errcode;
        }
        streamdId = child->getDeviceId() + "_" + std::to_string(streamtype);
        MediaStream::Ptr s = MediaMng::GetInstance().findStream(streamdId);
        if(s)
        {
            s->increasing();
            sinfo->ms = s;
            sinfo->readhandle = s->createReader();

            errcode = 0;
            writer.StartObject();
            writer.Key("classe"); writer.String(webs_Play_class);
            writer.Key("method"); writer.String(web_RealTime_method);
            writer.Key("errorcode"); writer.Int(errcode);
            writer.Key("msg"); writer.String("");
            writer.EndObject();
            return errcode;
        }
        else
        {
            s = MediaMng::GetInstance().createLiveStream("37028806251320111520", streamtype);
            if(s)
            {
                s->increasing();
                sinfo->ms = s;
                sinfo->readhandle = s->createReader();

                errcode = 0;
                writer.StartObject();
                writer.Key("classe"); writer.String(webs_Play_class);
                writer.Key("method"); writer.String(web_RealTime_method);
                writer.Key("errorcode"); writer.Int(errcode);
                writer.Key("msg"); writer.String("");
                writer.EndObject();
                return errcode;
            }
            else
            {
                errcode = -2;
                writer.StartObject();
                writer.Key("classe"); writer.String(webs_Play_class);
                writer.Key("method"); writer.String(web_RealTime_method);
                writer.Key("errorcode"); writer.Int(errcode);
                writer.Key("playhandle"); writer.Int(0);
                //writer.Key("msg");writer.String(ErrStr(errcode));
                //writer.Key("fps");writer.Int(videoparam.codecMap[0].fps);
                writer.EndObject();
                return errcode;
            }
        }
    }
    else
    {
        errcode = 6;
        writer.StartObject();
        writer.Key("classe"); writer.String(webs_Play_class);
        writer.Key("method"); writer.String(web_RealTime_method);
        writer.Key("errorcode"); writer.Int(errcode);
        writer.Key("msg"); writer.String("Parser Json error!");
        //writer.Key("fps");writer.Int(videoparam.codecMap[0].fps);
        writer.EndObject();
    }
    return errcode;
}
int WsServer::PlayBackAction(WsStreamInfo* sinfo, rapidjson_sip::Document& document, rapidjson_sip::Writer<rapidjson_sip::StringBuffer>& writer)
{
    int errcode = 0;
    unsigned int readhandle = 0;
    bool playok = false;
    if(document.HasMember("params"))
    {
        //{"starttime":1,"endtime":0,"handle":0}}
        rapidjson_sip::Value& msbody = document["params"];
        unsigned int chl_handle = json_check_uint32(msbody, "handle");
        uint64_t starttime = json_check_uint64(msbody, "starttime");
        uint64_t endtime = json_check_uint64(msbody, "endtime");
        int filetype = json_check_int32(msbody, "fileType");
        uint64_t pointtime = json_check_uint64(msbody, "pointTime");
        std::string uuid = json_check_string(msbody, "uuid");
        std::string deviceId = json_check_string(msbody, "deviceId");
        if(pointtime == 0)
            pointtime = starttime;

        BaseChildDevice* child = DeviceMng::Instance().findChildDeviceByCCTVDeviceId(deviceId);
        if(child == NULL)
        {
            errcode = 3;
            writer.StartObject();
            writer.Key("classe"); writer.String(webs_Play_class);
            writer.Key("method"); writer.String(webs_PlayBack_method);
            writer.Key("errorcode"); writer.Int(errcode);
            writer.Key("msg"); writer.String("device not found");
            writer.EndObject();
            return errcode;
        }
        std::string streamdId = child->getDeviceId();

        //std::string streamdId = MediaMng::GetInstance().getStreamId(child->getDeviceId(), starttime, endtime);
        bool playok = false;
        MediaStream::Ptr ms = MediaMng::GetInstance().createVodStream(child->getDeviceId(), starttime, endtime);
        if(ms)
        {
            playok = true;
            ms->increasing();
            sinfo->ms = ms;
            sinfo->speed = 1.0;
            sinfo->ctrlType = 0;
            sinfo->readhandle = ms->createReader();

            errcode = 0;
            writer.StartObject();
            writer.Key("classe"); writer.String(webs_Play_class);
            writer.Key("method"); writer.String(webs_PlayBack_method);
            writer.Key("errorcode"); writer.Int(errcode);
            writer.Key("msg"); writer.String("");
            writer.Key("fps"); writer.Int(25);
            writer.EndObject();
            return errcode;
        }
        else
        {
            errcode = -2;
            writer.StartObject();
            writer.Key("classe"); writer.String(webs_Play_class);
            writer.Key("method"); writer.String(webs_PlayBack_method);
            writer.Key("errorcode"); writer.Int(errcode);
            writer.Key("playhandle"); writer.Int(0);
            writer.Key("msg");writer.String("create Vod Stream failed");
            writer.Key("fps");writer.Int(25);
            writer.EndObject();
            return errcode;
        }
    }
    else
    {
        writer.StartObject();
        writer.Key("classe"); writer.String(webs_Play_class);
        writer.Key("method"); writer.String(webs_PlayBack_method);
        writer.Key("errorcode"); writer.Int(6);
        writer.Key("playhandle"); writer.Int(0);
        writer.Key("msg");writer.String("params parse failed");
        writer.EndObject();
        return 0;
    }
    return errcode;
}
int WsServer::PlayBackControlAction(WsStreamInfo* sinfo, rapidjson_sip::Document& document, rapidjson_sip::Writer<rapidjson_sip::StringBuffer>& writer)
{
    int errcode = 0;
    unsigned int readhandle;
    int streamno = 0;
    bool playok = false;
    if(document.HasMember("params"))
    {
        //回放控制类型：ctrlType 0：开始1：暂停2：倍速3：拖动
        //"params":{"ctrlType":1,"speed":0.5,"pointTime":1631868217,"handle":2}
        rapidjson_sip::Value& msbody = document["params"];
        int ctrlType = json_check_int32(msbody, "ctrlType");
        float speed = json_check_float(msbody, "speed");
        uint64_t pointTime = json_check_uint32(msbody, "pointTime");
        std::string uuid = json_check_string(msbody, "uuid");
        uint32_t handle = json_check_uint32(msbody, "handle");

        if(sinfo->IsLive == 1)
        {
            sinfo->ctrlType = ctrlType;
            if(!sinfo->ms)
            {
                errcode = 2;
                writer.StartObject();
                writer.Key("classe"); writer.String(webs_Play_class);
                writer.Key("method"); writer.String(web_RealTime_method);
                writer.Key("errorcode"); writer.Int(errcode);
                writer.Key("msg"); writer.String("media not found");
                writer.EndObject();
                return errcode;
            }
            BaseChildDevice* child = DeviceMng::Instance().findChildDevice(sinfo->ms->getDeviceId());
            if(child == NULL)
            {
                errcode = 3;
                writer.StartObject();
                writer.Key("classe"); writer.String(webs_Play_class);
                writer.Key("method"); writer.String(web_RealTime_method);
                writer.Key("errorcode"); writer.Int(errcode);
                writer.Key("msg"); writer.String("device not found");
                writer.EndObject();
                return errcode;
            }
            BaseDevice::Ptr parentDev = NULL;
            if(child)
            {
                parentDev = child->getParentDev();
            }
            if(parentDev == NULL)
            {
                errcode = 3;
                writer.StartObject();
                writer.Key("classe"); writer.String(webs_Play_class);
                writer.Key("method"); writer.String(web_RealTime_method);
                writer.Key("errorcode"); writer.Int(errcode);
                writer.Key("msg"); writer.String("parent device not found");
                writer.EndObject();
                return errcode;
            }
            int err = 0;
            if(ctrlType == 0)
            {
                sinfo->speed = 1;
                //Dev_PlayBackCtrl()
            }
            else if(ctrlType == 1)
            {
                //connectinfo->speed = 0xffffffff;
                //Dev_PlayBackCtrl
                sinfo->speed = 0;
                //connectinfo->prevTime = 0xFFFFFFFFFFFFF
            }
            else if(ctrlType == 2)
            {
                //Dev_PlayBackCtrl
                sinfo->speed = speed;
            }
            else if(ctrlType == 3)
            {
                sinfo->nFrameTime = pointTime * 1000;
                if(parentDev->devType == BaseDevice::JSON_NVR)
                {
                    auto JsonNvr = std::dynamic_pointer_cast<JsonNvrDevic>(parentDev);
                    if(JsonNvr)
                    {
                        LogOut("BLL", L_DEBUG, "play back ctrl %ju", pointTime);
                        //JsonNvr->Dev_PbCtrlTimePos(sinfo->ms->getStreamHandle(), pointTime, err);
                        JsonNvr->Dev_PlayBackCtrl(sinfo->ms->getStreamHandle(), JsonNvrDevic::JsonPbCtrl_TimePos, (uint32_t)pointTime, 0, err);
                    }
                }
            }
            else
            {
                errcode = -1;
            }
        }
        writer.StartObject();
        writer.Key("classe"); writer.String(webs_Play_class);
        writer.Key("method"); writer.String(webs_PBControl_method);
        writer.Key("errorcode"); writer.Int(errcode);
        //writer.Key("msg");writer.String(ErrStr(errcode));
        writer.EndObject();
    }
    else
    {
        errcode = 6;
        writer.StartObject();
        writer.Key("classe"); writer.String(webs_Play_class);
        writer.Key("method"); writer.String(webs_PBControl_method);
        writer.Key("errorcode"); writer.Int(errcode);
        writer.Key("msg");writer.String("params parse failed");
        writer.EndObject();
    }
    return errcode;
}
void WsServer::RtPreviewThread(WsStreamInfo* smInfo)
{
    if(smInfo == NULL)
        return;

    uint64_t m_clock = 0;
    int mGap = 40;
    uint64_t frameNum = 0;

    uint8_t* pBuffer = (uint8_t*)malloc(1024 * 1024);
    int offset = 0;
    pthread_setname_np(pthread_self(), "PreviewThread");
    while(smInfo->readhandle > 0)
    {
        int64_t clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        //if (0 == m_clock || m_clock < clock)
        {
            vframe_t frame;
            if(smInfo->ms && 0 == smInfo->ms->GetNextFrame(smInfo->readhandle, frame))
            {
                if(!frame.nalu)
                    continue;
                if(0 == m_clock)
                    m_clock = clock;
                if(frame.nalu && frame.nalu[0] != 0x00)
                {
                    LogOut("BLL", L_ERROR, "data err %x,%x,%x,%x,nalu:%d", frame.nalu[0], frame.nalu[1], frame.nalu[2], frame.nalu[3], (frame.nalu[4] & 0x1F));
                    continue;
                }
                if(frame.frameType == GB_CODEC_H264 || frame.frameType == GB_CODEC_H265)
                {
                    frameNum++;
                }
                if(frame.idr)
                {
                    mGap = frame.gap;
                    LogOut("BLL", L_ERROR, "--------------- idr frame time:%ld, gap:%d, frameNum:%ju", time(0), mGap, frameNum);
                }

                int type = 0, nfps = 1000 / mGap;
                short auChl = 0, auRate = 0;
                uint64_t pts = clock;
                if(frame.frameType == GB_CODEC_H264)
                {
                    type = 0;
                }
                else if(frame.frameType == GB_CODEC_H265)
                {
                    type = 1;
                }
                else
                {
                    /*if (frame.frameType == PT_AAC)
                    {
                        type = 37;
                    }
                    else if (frame.frameType == PT_PCMA)
                    {
                        type = 19;
                    }
                    else if (frame.frameType == PT_PCMU)
                    {
                        type = 20;
                    }
                    else
                    {
                        type = frame.frameType;
                    }*/
                    auChl = 1;// pFrame->GetChls();
                    //auRate = pFrame->GetFramRate() / 1000;
                    auRate = auRate == 0 ? 8000 : auRate;
                }
                type = htonl(type);
                pts = (((uint64_t)htonl(pts)) << 32) + htonl(pts >> 32);;
                auChl = htons(auChl);
                auRate = htons(auRate);
                nfps = htonl(nfps);

                offset = 0;
                memcpy(pBuffer + offset, (unsigned char*)&type, sizeof(type));
                offset += sizeof(type);

                memcpy(pBuffer + offset, (unsigned char*)&pts, sizeof(pts));
                offset += sizeof(pts);

                if(frame.frameType == GB_CODEC_H264 || frame.frameType == GB_CODEC_H265)
                {
                    memcpy(pBuffer + offset, (unsigned char*)&nfps, sizeof(nfps));
                    offset += sizeof(nfps);
                }
                else
                {
                    memcpy(pBuffer + offset, (unsigned char*)&auChl, sizeof(auChl));
                    offset += sizeof(auChl);

                    memcpy(pBuffer + offset, (unsigned char*)&auRate, sizeof(auRate));
                    offset += sizeof(auRate);
                }

                memcpy(pBuffer + offset, frame.nalu, frame.bytes);
                offset += frame.bytes;

                //size_t len = mg_ws_send(smInfo->c, pBuffer, offset, WEBSOCKET_OP_BINARY);
                websocketpp::lib::error_code ec;
                smInfo->s->send(smInfo->hdl, pBuffer, offset, websocketpp::frame::opcode::binary, ec);
                if(ec)
                {
                    break;
                }
                //send(connection_hdl hdl, void const* payload, size_t len, frame::opcode::value op, lib::error_code & ec)
                m_clock += mGap;

            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    free(pBuffer);
    LogOut("BLL", L_DEBUG, "------------------- Preview exit -------ms info :%p", smInfo);
}
void WsServer::PlayBackThread(WsStreamInfo* smInfo)
{
    if(smInfo == NULL)
        return;

    uint64_t m_clock = 0;
    int gap = 40;
    uint64_t frameNum = 0;

    uint8_t* pBuffer = (uint8_t*)malloc(1024 * 1024 * 2);
    int offset = 0;
    av_Frame frame;
    pthread_setname_np(pthread_self(), "PlayBackThread");
    while(smInfo->readhandle > 0)
    {
        uint64_t curClock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        if(smInfo->speed != 0 && m_clock <= curClock)
        {
            ushort index = 0;
            if(smInfo->ms)
            {
                int ret = smInfo->ms->GetNextFrameEx(smInfo->readhandle, frame, index);
                if(ret == 0)
                {
                    // LogOut("BLL", L_DEBUG, "get frame index:%d, buf:%p", index, frame.Data());
                    if(smInfo->ctrlType == 3 && frame.PTS() < smInfo->nFrameTime)
                    {
                        frame.dereference();
                        smInfo->ms->freeFrameByIndex(index);
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        continue;
                    }
                    smInfo->ctrlType = 0;
                    uint64_t pts = 0;
                    int type = 0, nfps = 0;
                    if(frame.GetCodecType() == CODEC_H264 || frame.GetCodecType() == CODEC_H265)
                    {
                        frameNum++;
                        pts = frame.PTS();
                        nfps = frame.GetFramRate();
                        gap = 1000 / (nfps * smInfo->speed);
                    }

                    short auChl = 0, auRate = 0;
                    if(frame.GetCodecType() == CODEC_H264)
                    {
                        type = 0;
                    }
                    else if(frame.GetCodecType() == CODEC_H265)
                    {
                        type = 1;
                    }
                    else
                    {
                        if(frame.GetCodecType() == CODEC_AAC)
                        {
                            type = 37;
                        }
                        else if(frame.GetCodecType() == CODEC_PCMA)
                        {
                            type = 19;
                        }
                        else if(frame.GetCodecType() == CODEC_PCMU)
                        {
                            type = 20;
                        }
                        else
                        {
                            type = frame.GetCodecType();
                        }
                        auChl = frame.GetChls() > 0 ? frame.GetChls() : 1;// pFrame->GetChls();
                        //auRate = pFrame->GetFramRate() / 1000;
                        auRate = frame.GetSampleRate() == 0 ? 8000 : frame.GetSampleRate();
                    }
                    type = htonl(type);
                    pts = (((uint64_t)htonl(pts)) << 32) + htonl(pts >> 32);
                    auChl = htons(auChl);
                    auRate = htons(auRate);
                    nfps = htonl(nfps);

                    offset = 0;
                    memcpy(pBuffer + offset, (unsigned char*)&type, sizeof(type));
                    offset += sizeof(type);

                    memcpy(pBuffer + offset, (unsigned char*)&pts, sizeof(pts));
                    offset += sizeof(pts);
                    if(frame.IsKeyFram())
                    {
                        LogOut("BLL", L_DEBUG, "ws send I frame time:%llu: frame time:%llu", m_clock, frame.PTS());
                    }
                    else
                    {
                        // LogOut("BLL", L_DEBUG, "ws send P frame time:%llu: frame time:%llu", m_clock, frame.PTS());
                    }
                    if(frame.GetCodecType() == CODEC_H264 || frame.GetCodecType() == CODEC_H265)
                    {
                        memcpy(pBuffer + offset, (unsigned char*)&nfps, sizeof(nfps));
                        offset += sizeof(nfps);
                    }
                    else
                    {
                        memcpy(pBuffer + offset, (unsigned char*)&auChl, sizeof(auChl));
                        offset += sizeof(auChl);

                        memcpy(pBuffer + offset, (unsigned char*)&auRate, sizeof(auRate));
                        offset += sizeof(auRate);
                    }

                    memcpy(pBuffer + offset, frame.Data(), frame.DataLen());
                    offset += frame.DataLen();

                    frame.dereference();
                    smInfo->ms->freeFrameByIndex(index);

                    //size_t len = mg_ws_send(smInfo->c, pBuffer, offset, WEBSOCKET_OP_BINARY);
                    if(m_clock == 0)
                    {
                        m_clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                    }
                    websocketpp::lib::error_code ec;
                    smInfo->s->send(smInfo->hdl, pBuffer, offset, websocketpp::frame::opcode::binary, ec);
                    if(ec)
                    {
                        LogOut("BLL", L_DEBUG, "ws send data :%d", ec.value());
                        break;
                    }
                    m_clock += gap;
                }
                else
                {
                    //LogOut("BLL", L_DEBUG, "ws send data read frame error:%d", ret);
                }
            }
            else
            {
                LogOut("BLL", L_DEBUG, "ws send data ms is null");
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    free(pBuffer);
    frame.dereference();
    LogOut("BLL", L_DEBUG, "------------------- play back exit -------ms info :%p, handle:%u", smInfo, smInfo->readhandle);
}