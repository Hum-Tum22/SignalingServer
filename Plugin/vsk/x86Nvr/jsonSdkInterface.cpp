#include "jsonSdkInterface.h"
#include "myjsondef.h"
#include "../../../lib/rapidjson/document.h"
#include "../../../lib/rapidjson/stringbuffer.h"
#include "../../../lib/rapidjson/writer.h"
#include "include/base64.h"
#include "SelfLog.h"
#include <errno.h>
#include <exception>
#include <iostream>
//#include "include/base64.cpp"

JsonSdkInterface& JsonSdkInterface::Instance()
{
    static JsonSdkInterface* g_JsonSdkInterface = new JsonSdkInterface();
    return *g_JsonSdkInterface;
}
JsonSdkInterface::JsonSdkInterface() :PluginInter(PluginInter::JSON_SDK), m_hDll(NULL), IsInit(false)
{
    InitFun = NULL;
    ClearFun = NULL;
    LoginFun = NULL;
    LogOutFun = NULL;
    PreviewFun = NULL;
    VskPreviewFun = NULL;
    StopPreviewFun = NULL;
    NewDataLinkFun = NULL;
	FreeDataLinkFun = NULL;
	SearchFileFun = NULL;
	SearchNextFileFun = NULL;
	SearchFileCloseFun = NULL; 
    PlayBackFun = NULL;
    StopPlayBackFun = NULL;
    PlayBackCtrlFun = NULL;
    setTimePosFun = NULL;
    DownloadFun = NULL;
    StopDownloadFun = NULL;
    PtzCtrlFun = NULL;
    ListIPCFun = NULL;
    GetChannelParam = NULL;
#ifdef _WIN32
    m_hDll = LoadLibrary(L"sdkJson.dll");
#else
    m_hDll = dlopen("./libsdkJson.so", RTLD_NOW);
    if(!m_hDll)
    {
        LogOut(SDK, L_ERROR, "sdkJson.so load err:%s", dlerror());
    }
#endif
}
JsonSdkInterface::~JsonSdkInterface()
{
    if(m_hDll)
    {
        int err;
        SdkClear(err);
        if(err == 0)
        {
#ifdef _WIN32
            FreeLibrary(m_hDll);
#else
            dlclose(m_hDll);
#endif
            m_hDll = NULL;
        }
    }
}
bool JsonSdkInterface::SdkIsInit()
{
    return IsInit;
}
void JsonSdkInterface::InitSdk(int& err)
{
    err = 0;
    if(m_hDll)
    {
        if(!InitFun)
        {
            InitFun = (Sdk_Initate)LoadSharedLibFun(m_hDll, "JsonSdk_Initate");
        }
        if(InitFun)
        {
            int ret = InitFun();
            if(ret == 0)
            {
                IsInit = true;
            }
            else
            {
                LogOut(SDK, L_ERROR, "json sdk init err:%d", err);
                err = ret;
            }
        }
        else
        {
            LogOut(SDK, L_ERROR, "get JsonSdk_Initate err:%d", err);
            err = -2;
        }
    }
    else
    {
        err = -1;
        LogOut(SDK, L_ERROR, "Load json sdk err:%d", err);
    }
}
void JsonSdkInterface::SdkClear(int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!ClearFun)
        {
            ClearFun = (Sdk_Clear)LoadSharedLibFun(m_hDll, "JsonSdk_Clear");
        }
        if(ClearFun)
        {
            int ret = ClearFun();
            if(ret == 0)
            {
                IsInit = false;
                return;
            }
            else
            {
                err = ret;
            }
        }
        else
            err = -2;
    }
    else
        err = -1;
}
JSONLONG JsonSdkInterface::JsonSdkLogIn(const char* ip, int port, const char* name, const char* pswd, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!LoginFun)
        {
            LoginFun = (Sdk_Login)LoadSharedLibFun(m_hDll, "JsonSdk_Login");
        }
        if(LoginFun)
        {
            //ReqClientLogin
            rapidjson_sip::StringBuffer buffer;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(buffer);
            writer.StartObject();

            writer.Key("user_info");
            writer.StartObject();
            writer.Key("user_name");
            writer.String(BinToBase64(name).c_str());
            writer.Key("password");
            writer.String(BinToBase64(pswd).c_str());
            writer.EndObject();

            writer.Key("strNvrIp");
            writer.String(ip);
            writer.Key("nNvrPort");
            writer.Int(port);
            writer.EndObject();

            std::string strJsonReq = buffer.GetString();
            try
            {
                JSONLONG nLoginID;
                int ret = LoginFun((char*)strJsonReq.c_str(), &nLoginID, NULL, NULL, NULL, NULL);
                if(ret == 0)
                {
                    LogOut(SDK, L_DEBUG, "json sdk  login ok %ld", nLoginID);
                    return nLoginID;
                }
                else
                {
                    LogOut(SDK, L_ERROR, "json sdk  login err:%d", ret);
                    err = ret;
                }
            }
            catch(std::exception& e)
            {
                LogOut(SDK, L_ERROR, "Standard exception:%s", e.what());
            }
        }
        else
        {
            LogOut(SDK, L_ERROR, "get JsonSdk_Login err:%d", err);
            err = -2;
        }
    }
    else
    {
        err = -1;
        LogOut(SDK, L_ERROR, "Load json sdk err:%d", err);
    }
    return 0;
}
void JsonSdkInterface::JsonSdkLogOut(JSONLONG loginId, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!LogOutFun)
        {
            LogOutFun = (Sdk_Logout)LoadSharedLibFun(m_hDll, "JsonSdk_Logout");
        }
        if(LogOutFun && loginId)
        {
            int ret = LogOutFun(loginId);
            if(ret == 0)
            {
                LogOut(SDK, L_ERROR, "json sdk  logout %ld", loginId);
                return;
            }
            else
                err = ret;
        }
        else
            err = -2;
    }
    else
        err = -1;
    return;
}
void JsonSdkInterface::GetChannelEncoderParam(JSONLONG UserID, JSONLONG chid, char* pInfo, uint32_t* pInfoSize, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!GetChannelParam)
        {
            GetChannelParam = (Sdk_GetVideoEncoderChannelParam)LoadSharedLibFun(m_hDll, "JsonSdk_GetVideoEncoderChannelParam");
        }
        if(UserID && GetChannelParam)
        {
            int ret = GetChannelParam(UserID, chid, pInfo, pInfoSize);
            if(ret == 0)
            {
                LogOut(SDK, L_DEBUG, "json sdk  get channel encoder param ok");
                return;
            }
            else
                err = ret;
        }
        else
            err = -2;
    }
    else
        err = -1;
    return;
}
JSONLONG JsonSdkInterface::Preview(JSONLONG UserID, int channel, int streamId, DataVideoAudioCallBackEx VideoTranCallBack, void* pUser, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!PreviewFun)
        {
            PreviewFun = (Sdk_Preview)LoadSharedLibFun(m_hDll, "JsonSdk_VideoTranspondStartWithoutViskhead");
        }
        if(UserID && PreviewFun)
        {
            rapidjson_sip::StringBuffer buffer;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(buffer);
            writer.StartObject();

            writer.Key("chid");
            writer.Int(channel);
            writer.Key("rate_type");
            writer.Int(streamId);
            writer.Key("trans_mode");
            writer.Int(0);
            writer.Key("rate_size");
            writer.Int(0);

            writer.EndObject();

            std::string strJsonReq = buffer.GetString();
            JSONLONG hHandle = 0;
            int ret = PreviewFun(UserID, VideoTranCallBack, (char*)strJsonReq.c_str(), 0, pUser, &hHandle);
            if(ret == 0)
            {
                LogOut(SDK, L_ERROR, "json sdk  preview ok %ld", hHandle);
                return hHandle;
            }
            else
                err = ret;
        }
        else
            err = -2;
    }
    else
        err = -1;
    return 0;
}
JSONLONG JsonSdkInterface::VskPreview(JSONLONG UserID, int channel, int streamId, DataPlayCallBack VideoTranCallBack, void* pUser, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!VskPreviewFun)
        {
            VskPreviewFun = (Sdk_VskPreview)LoadSharedLibFun(m_hDll, "JsonSdk_VideoTranspondStart");
        }
        if(UserID && VskPreviewFun)
        {
            rapidjson_sip::StringBuffer buffer;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(buffer);
            writer.StartObject();

            writer.Key("chid");
            writer.Int(channel);
            writer.Key("rate_type");
            writer.Int(streamId);
            writer.Key("trans_mode");
            writer.Int(0);
            writer.Key("rate_size");
            writer.Int(0);

            writer.EndObject();

            std::string strJsonReq = buffer.GetString();
            JSONLONG hHandle = 0;
            int ret = VskPreviewFun(UserID, VideoTranCallBack, (char*)strJsonReq.c_str(), 0, pUser, &hHandle);
            if(ret == 0)
            {
                LogOut(SDK, L_DEBUG, "json sdk vsk preview ok %ld", hHandle);
                return hHandle;
            }
            else
            {
                err = ret;
                LogOut(SDK, L_ERROR, "preview err %d", ret);
            }
        }
        else
            err = -2;
    }
    else
        err = -1;
    return 0;
}
void JsonSdkInterface::StopPreview(JSONLONG rthandle, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!StopPreviewFun)
        {
            StopPreviewFun = (Sdk_StopPreview)LoadSharedLibFun(m_hDll, "JsonSdk_VideoTranspondStop");
        }
        if(rthandle && StopPreviewFun)
        {
            int ret = StopPreviewFun(rthandle);
            if(ret == 0)
            {
                LogOut(SDK, L_ERROR, "json sdk stop preview ok %ld", rthandle);
                return;
            }
            err = ret;
            LogOut(SDK, L_ERROR, "stop preview err %d", ret);
            return;
        }
        err = -2;
        return;
    }
    err = -1;
    return;
}
int JsonSdkInterface::SearchRecordFileList(JSONLONG UserID, int channel, long start, long end, std::list<JsonRecordInfo> &recordInfos, int &err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!NewDataLinkFun)
        {
            NewDataLinkFun = (Sdk_NewDataLink)LoadSharedLibFun(m_hDll, "JsonSdk_NewDataLink");
        }
        if(!FreeDataLinkFun)
        {
            FreeDataLinkFun = (Sdk_FreeDataLink)LoadSharedLibFun(m_hDll, "JsonSdk_FreeDataLink");
        }
        if(!SearchFileFun)
        {
            SearchFileFun = (Sdk_SearchFile)LoadSharedLibFun(m_hDll, "JsonSdk_SearchFile");
        }
        if(!SearchNextFileFun)
        {
            SearchNextFileFun = (Sdk_SearchNextFile)LoadSharedLibFun(m_hDll, "JsonSdk_SearchNextFile");
        }
        if(!SearchFileCloseFun)
        {
            SearchFileCloseFun = (Sdk_SearchFileClose)LoadSharedLibFun(m_hDll, "JsonSdk_SearchFileClose");
        }
        if(!NewDataLinkFun || !FreeDataLinkFun || !SearchFileFun || !SearchNextFileFun || !SearchFileCloseFun)
        {
            LogOut(SDK, L_DEBUG, "load function failed %p, %p, %p", SearchFileFun, SearchNextFileFun, SearchFileCloseFun);
            err = -1;
            return 0;
        }
        if(UserID)
        {
            JSONLONG nNewLinkID = 0;
            int nRet = NewDataLinkFun(UserID, &nNewLinkID);
            if(nNewLinkID == 0)
            {
                LogOut(SDK, L_ERROR, "new link error:%d", nRet);
                err = nRet;
                return 0;
            }
            
            rapidjson_sip::StringBuffer buffer;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(buffer);
            writer.StartObject();

            writer.Key("chid"); writer.Int(channel);
            writer.Key("stream_type"); writer.Int(0);
            writer.Key("start_time"); writer.Uint(start);
            writer.Key("end_time"); writer.Uint(end);
            writer.Key("file_type"); writer.Uint(1); //DiskFileType::SearchFileType_vsk_data
            writer.Key("record_type"); writer.Uint(1);
            writer.Key("record_type"); writer.Uint(1);
            writer.Key("start_file_id"); writer.Uint(0);
            writer.Key("max_count"); writer.Uint(20000);
            writer.EndObject();

            std::string strJsonReq = buffer.GetString();
            uint32_t nSearchCount = 0;
            nRet = SearchFileFun(nNewLinkID, strJsonReq.c_str(), &nSearchCount);
            LogOut(BLL, L_DEBUG, "file size:%u cond:%s", nSearchCount, strJsonReq.c_str());

            const static uint32_t nBufSize = 4 * 1024 * 1024;
            char * pOutBuffer = new char[nBufSize];
            if (pOutBuffer == NULL)
            {
                LogOut(SDK, L_ERROR, "mem error:%d", errno);
                return -3;
            }
            memset(pOutBuffer, 0, nBufSize);
            //成功检索
            if (nSearchCount > 0)
            {
                uint32_t dwRet = 0;
                do
                {
                    uint32_t nFileSize = nBufSize;
                    dwRet = SearchNextFileFun(nNewLinkID, pOutBuffer, &nFileSize);
                    if (dwRet == 0)
                    {
                        JsonRecordInfo fileInfo;
                        fileInfo.file_id = 0;
                        LogOut(SDK, L_DEBUG, "record file info:%s", pOutBuffer);
                        strRecordInfoToJsonRecordInfo(std::string(pOutBuffer), fileInfo);
                        if(fileInfo.file_id > 0)
                        {
                            recordInfos.push_back(fileInfo);
                        }
                    }	
                    else
                    {
                        break;
                    }
                    
                } while(dwRet != 1 && dwRet != 3 && dwRet != 4);
                // typedef enum
                // {
                //     NS_QUERYINFO_SUCCESS,
                //     NS_QUERYINFO_NOFIND,
                //     NS_QUERYINFO_FINDING,
                //     NS_QUERYINFO_FINISH,
                //     NS_QUERYINFO_EXCEPTION,
                //     NS_QUERYINFO_MORE_BUFFER,
                // }QUERY_REPLY;
                SearchFileCloseFun(nNewLinkID);
                LogOut(BLL, L_DEBUG, "recordInfos size:%Zu", recordInfos.size());
            }
            else
            {
                LogOut(SDK, L_WARN, "not search file error code :%d", nRet);
            }
            FreeDataLinkFun(nNewLinkID);
            delete[] pOutBuffer;  pOutBuffer = NULL;
        }
        else
            err = -2;
    }
    else
        err = -1;
    return 0;
}
JSONLONG JsonSdkInterface::PlayBack(JSONLONG UserID, int channel, long start, long end, DataPlayCallBack VideoTranCallBack, PlayBackEndCallBack fun, void *pUser, int &err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!PlayBackFun)
        {
            PlayBackFun = (Sdk_PlayBackStartByTime)LoadSharedLibFun(m_hDll, "JsonSdk_PlayBackStartByTime");
        }
        if(UserID && PlayBackFun)
        {
            //ReqStartPlaybackStreamTime
            rapidjson_sip::StringBuffer buffer;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(buffer);
            writer.StartObject();

            writer.Key("chid");
            writer.Int(channel);
            writer.Key("stream_type");
            writer.Int(0);
            writer.Key("start_time");
            writer.Int(start);
            writer.Key("end_time");
            writer.Int(end);

            writer.EndObject();

            std::string strJsonReq = buffer.GetString();
            JSONLONG hHandle = 0;
            uint64_t  nTotalFileSize = 0;
            int ret = PlayBackFun(UserID, strJsonReq.c_str(), VideoTranCallBack, fun, 0, pUser, &nTotalFileSize, &hHandle);
            if(ret == 0)
            {
                return hHandle;
            }
            err = ret;
            LogOut(SDK, L_ERROR, "play back err %d", ret);
            return 0;
        }
        LogOut(SDK, L_ERROR, "play back err loginId:%lu, %p", UserID, (void*)PlayBackFun);
        err = -2;
        return 0;
    }
    err = -1;
    return 0;
}
void JsonSdkInterface::StopPlayBack(JSONLONG UserID, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!StopPlayBackFun)
        {
            StopPlayBackFun = (Sdk_PlayBackStop)LoadSharedLibFun(m_hDll, "JsonSdk_PlayBackStop");
        }
        if(UserID && StopPlayBackFun)
        {
            int ret = StopPlayBackFun(UserID);
            if(ret == 0)
            {
                return;
            }
            err = ret;
            LogOut(SDK, L_ERROR, "stop play back err %d", ret);
            return;
        }
        err = -2;
        return;
    }
    err = -1;
    return;
}
void JsonSdkInterface::PlayBackCtrl(JSONLONG UserID, int cmd, int param1, int param2, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!PlayBackCtrlFun)
        {
            PlayBackCtrlFun = (Sdk_PlayBackCtrl)LoadSharedLibFun(m_hDll, "JsonSdk_PlayBackCtrl");
        }
        if(UserID && PlayBackCtrlFun)
        {
            //ReqStartPlaybackStreamTime
            rapidjson_sip::StringBuffer buffer;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(buffer);
            writer.StartObject();
            writer.Key("code");
            writer.Int(cmd);
            writer.Key("param1");
            writer.Int(param1);
            writer.Key("param2");
            writer.Int(param2);
            writer.EndObject();

            std::string strJsonReq = buffer.GetString();
            int ret = PlayBackCtrlFun(UserID, strJsonReq.c_str());
            if(ret != 0)
            {
                err = ret;
                LogOut(SDK, L_ERROR, "play back ctrl err %d", ret);
            }
            return;
        }
        err = -2;
        return;
    }
    err = -1;
    return;
}
void JsonSdkInterface::setTimePos(JSONLONG pbhandle, time_t t, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!setTimePosFun)
        {
            setTimePosFun = (Sdk_TimePos)LoadSharedLibFun(m_hDll, "JsonSdk_SetTimePlayBackPos");
        }
        if(pbhandle && setTimePosFun)
        {
            int ret = setTimePosFun(pbhandle, t);
            if(ret != 0)
            {
                err = ret;
                LogOut(SDK, L_ERROR, "set time pos err %d", ret);
            }
            return;
        }
        err = -2;
        return;
    }
    err = -1;
    return;
}
JSONLONG JsonSdkInterface::Download(JSONLONG UserID, int channel, long start, long end, DataPlayCallBack VideoTranCallBack, PlayBackEndCallBack fun, void* pUser, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!DownloadFun)
        {
            DownloadFun = (Sdk_Download)LoadSharedLibFun(m_hDll, "JsonSdk_DownloadStartByTime");
        }
        if(UserID && DownloadFun)
        {
            //ReqStartPlaybackStreamTime
            rapidjson_sip::StringBuffer buffer;
            rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(buffer);
            writer.StartObject();

            writer.Key("chid");
            writer.Int(channel);
            writer.Key("stream_type");
            writer.Int(0);
            writer.Key("start_time");
            writer.Int(start);
            writer.Key("end_time");
            writer.Int(end);

            writer.EndObject();

            std::string strJsonReq = buffer.GetString();
            JSONLONG hHandle = 0;
            uint64_t  nTotalFileSize = 0;
            int ret = DownloadFun(UserID, strJsonReq.c_str(), VideoTranCallBack, fun, NULL, pUser, 0, &nTotalFileSize, &hHandle);
            if(ret == 0)
            {
                return hHandle;
            }
            err = ret;
            LogOut(SDK, L_ERROR, "download err %d", ret);
            return 0;
        }
        err = -2;
        return 0;
    }
    err = -1;
    return 0;
}
void JsonSdkInterface::StopDownload(JSONLONG UserID, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!StopDownloadFun)
        {
            StopDownloadFun = (Sdk_DownloadStop)LoadSharedLibFun(m_hDll, "JsonSdk_DownloadStop");
        }
        if(UserID && StopDownloadFun)
        {
            int ret = StopDownloadFun(UserID);
            if(ret == 0)
            {
                return;
            }
            err = ret;
            LogOut(SDK, L_ERROR, "stop download err %d", ret);
            return;
        }
        err = -2;
        return;
    }
    err = -1;
    return;
}
void JsonSdkInterface::PTZCtrl(JSONLONG UserID, uint32_t Channel, uint32_t PTZCommand, uint32_t Argument, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!PtzCtrlFun)
        {
            PtzCtrlFun = (Sdk_PTZCtrl)LoadSharedLibFun(m_hDll, "JsonSdk_PTZCtrl");
        }
        if(UserID && PtzCtrlFun)
        {
            try
            {
                int ret = PtzCtrlFun(UserID, Channel, PTZCommand, Argument);
                if(ret == 0)
                {
                    return;
                }
                err = ret;
                LogOut(SDK, L_ERROR, "ptz ctrl err %d", ret);
                return;
            }
            catch(std::exception& e)
            {
                LogOut(SDK, L_ERROR, "Standard exception:%s", e.what());
            }
        }
        err = -2;
        return;
    }
    err = -1;
    return;
}
void JsonSdkInterface::ListIPC(JSONLONG UserID, char* pIPCServerList, uint32_t* pIPCServerListSize, int& err)
{
    err = 0;
    if(m_hDll && IsInit)
    {
        if(!ListIPCFun)
        {
            ListIPCFun = (Sdk_ListIPC)LoadSharedLibFun(m_hDll, "JsonSdk_ListIPC");
        }
        if(UserID && ListIPCFun)
        {
            try
            {
                int ret = ListIPCFun(UserID, pIPCServerList, pIPCServerListSize);
                if(ret == 0)
                {
                    return;
                }
                else
                {
                    LogOut(SDK, L_ERROR, "json sdk  get ipc list err:%d", err);
                    err = ret;
                }
            }
            catch(std::exception& e)
            {
                LogOut(SDK, L_ERROR, "Standard exception:%s", e.what());
            }

            return;
        }
        else
        {
            LogOut(SDK, L_ERROR, "get JsonSdk_ListIPC err:%d", err);
            err = -2;
        }
        return;
    }
    else
    {
        err = -1;
        LogOut(SDK, L_ERROR, "Load json sdk err:%d", err);
    }
    return;
}
void JsonSdkInterface::strRecordInfoToJsonRecordInfo(std::string str, JsonRecordInfo &recordInfo)
{
    if(!str.empty())
    {
        rapidjson_sip::Document document;
        document.Parse((char*)str.c_str());
        if(!document.HasParseError())
        {
            recordInfo.file_id = json_check_uint64(document, "file_id");
            recordInfo.file_type = json_check_uint32(document, "file_type");
            recordInfo.file_name = json_check_string(document, "file_name");
            recordInfo.file_size = json_check_uint64(document, "file_size");
            recordInfo.start_time = json_check_uint32(document, "start_time");
            recordInfo.end_time = json_check_uint32(document, "end_time");
            recordInfo.chid = json_check_uint32(document, "chid");
            recordInfo.stream_type = json_check_uint32(document, "stream_type");
            recordInfo.disk_no = json_check_uint32(document, "disk_no");
            recordInfo.partition_no = json_check_uint32(document, "partition_no");
            recordInfo.record_type = json_check_uint32(document, "record_type");
            recordInfo.status = json_check_uint32(document, "status");
            recordInfo.trace_id = json_check_int32(document, "trace_id");
            recordInfo.face_file_path = json_check_string(document, "face_file_path");

            recordInfo.depkey_file_path = json_check_string(document, "depkey_file_path");
            recordInfo.identy_id = json_check_string(document, "identy_id");
            recordInfo.identy_type = json_check_int32(document, "identy_type");
            recordInfo.threshold = json_check_uint32(document, "threshold");
            if(document.HasMember("carplateinfo") && document["carplateinfo"].IsObject())
            {
                rapidjson_sip::Value &msbody = document["carplateinfo"];
                recordInfo.main_logo = json_check_string(msbody, "main_logo");
                recordInfo.sub_logo = json_check_string(msbody, "sub_logo");
                recordInfo.color = json_check_uint32(msbody, "color");
            }
        }
    }
}