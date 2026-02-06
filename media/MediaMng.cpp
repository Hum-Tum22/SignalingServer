#include "MediaMng.h"
#include "../deviceMng/JsonDevice.h"
#include "../deviceMng/deviceMng.h"
#include "mediaIn/JsonStream.h"
#include "SelfLog.h"

#include "myjsondef.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"

#include <iomanip>
#include <sstream>
#include <rutil/Log.hxx>
#include <rutil/Logger.hxx>

using namespace resip;
#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

MediaMng& MediaMng::GetInstance()
{
    static MediaMng* g_MediaMng = new MediaMng();
    return *g_MediaMng;
}
MediaMng::MediaMng() :statusRun(false)
{
}
MediaMng::~MediaMng()
{
}

void MediaMng::addStream(MediaStream::Ptr stream)
{
    if (!stream)
        return;
    GMUTEX lock(stmMtx);
    mStreamMap[stream->getStreamId()] = stream;
}
void MediaMng::removeStream(std::string Id)
{
    GMUTEX lock(stmMtx);
    auto it = mStreamMap.find(Id);
    if (it != mStreamMap.end())
    {
        if (it->second.use_count() <= 2)
        {
            mStreamMap.erase(Id);
        }
        else
        {
            LogOut("BLL", L_WARN, "stream:%s, use_count:%ld", it->second->getStreamId().c_str(), it->second.use_count());
        }
    }
}
MediaStream::Ptr MediaMng::findStream(std::string Id)
{
    GMUTEX lock(stmMtx);
    auto it = mStreamMap.find(Id);
    if (it != mStreamMap.end())
    {
        return it->second;
    }
    return NULL;
}
void MediaMng::checkStreamStatus()
{
    int count = 0;
    std::map<std::string, MediaStream::Ptr> tmpMap;
    while (statusRun)
    {
        if (count++ > 5)
        {
            {
                GMUTEX lock(stmMtx);
                tmpMap = mStreamMap;
            }
            for (auto& it : tmpMap)
            {
                if (time(0) - it.second->LastFrameTime() >= 10)
                {
                    BaseChildDevice* childDev = DeviceMng::Instance().findChildDevice(it.second->getDeviceId());
                    if (childDev)
                    {
                        BaseDevice::Ptr parentDev = childDev->getParentDev();
                        if (parentDev && parentDev->devType == BaseDevice::JSON_NVR)
                        {
                            auto JsonNvr = std::dynamic_pointer_cast<JsonNvrDevic>(parentDev);
                            if (JsonNvr)
                            {
                                int err = 0;
                                JsonNvr->Dev_StopPreview(it.second->getStreamHandle(), err);
                                if (err == 0)
                                {
                                    MediaMng::GetInstance().removeStream(it.second->getStreamId());
                                }
                            }
                        }
                    }
                }
            }
            tmpMap.clear();
        }
    }
}
MediaStream::Ptr MediaMng::createLiveStream(std::string deviceId, int streamNo)
{
    BaseDevice::Ptr parentDev = NULL;
    BaseChildDevice* childDev = NULL;
    if (!deviceId.empty())
    {
        childDev = DeviceMng::Instance().findChildDevice(deviceId);
        if (childDev)
        {
            parentDev = childDev->getParentDev();
        }
        else
        {
            DebugLog(<< deviceId << "  child device not found");
            return NULL;
        }
    }
    else
    {
        DebugLog(<< "deviceId is null");
        return NULL;
    }

    if (parentDev)
    {
        if (parentDev->devType == BaseDevice::JSON_NVR)
        {
            auto Nvr = std::dynamic_pointer_cast<JsonNvrDevic>(parentDev);
            if (Nvr)
            {
                int err = 0, chl = -1;
                uint32_t msgSize = 4 * 1024 * 1024;
                char* Buffer = new char[msgSize];
                Nvr->Dev_ListIPC(Buffer, msgSize, err);
                if (err == 0)
                {
                    rapidjson_sip::Document document;
                    document.Parse(Buffer);
                    if (!document.HasParseError())
                    {
                        if (document.HasMember("ipc_list") && document["ipc_list"].IsArray())
                        {
                            rapidjson_sip::Value& body = document["ipc_list"];
                            for (uint32_t i = 0; i < body.Size(); i++)
                            {
                                std::string devNum = json_check_string(body[i], "device_number");
                                std::size_t sPos = devNum.find(" ");
                                if (sPos != std::string::npos)
                                {
                                    devNum = devNum.substr(0, sPos);
                                }
                                JsonChildDevic* pChild = dynamic_cast<JsonChildDevic*>(childDev);
                                if (pChild && devNum == pChild->getName())
                                {
                                    int enable_flag = json_check_int32(body[i], "enable_flag");
                                    if (enable_flag == 2)
                                    {
                                        int child = json_check_int32(body[i], "chid");
                                        int online = json_check_int32(body[i], "online_status");
                                        chl = child;
                                        if (pChild->getChannel() != child)
                                        {
                                            pChild->setChannel(child);
                                        }
                                        if (online == 1)
                                        {
                                            std::string Id = json_check_string(body[i], "device_id");
                                            pChild->setStatus(1);
                                        }
                                        else
                                        {
                                            pChild->setStatus(0);
                                            DebugLog(<< "child device offline " << deviceId << " " << devNum);
                                        }
                                    }
                                    else
                                    {
                                        DebugLog(<< deviceId << " " << devNum << " child device enable_flag :" << enable_flag);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    DebugLog(<< deviceId << " " << err << "  get channel info err");
                }
                delete[] Buffer; Buffer = NULL;
                if (chl >= 0)
                {
                    //get channel encoder param
                    char Buffer[2048] = { 0 };
                    uint32_t bufSize = 2048;
                    int fps = 25;
                    Nvr->GetChannelEncoderParam(chl, Buffer, &bufSize, err);
                    if (err == 0)
                    {
                        DebugLog(<< "  get channel encoder param ok:" << Buffer);
                        rapidjson_sip::Document document;
                        document.Parse(Buffer);
                        if (!document.HasParseError())
                        {

                        }
                    }
                    std::string streamId = deviceId + "_" + std::to_string(streamNo);
                    JsonStream::Ptr streamIn = std::make_shared<JsonStream>(deviceId.c_str(), streamId.c_str());
                    if (streamIn)
                    {
                        ULHandle playhandle = Nvr->Dev_Preview(chl, streamNo, (void*)JsonStream::DataPlayCallBack, (void*)streamIn.get(), err);
                        if (err == 0)
                        {
                            DebugLog(<< "child device pull stream ok streameId:" << deviceId << " " << streamId);
                            streamIn->setStreamHandle(playhandle);
                            streamIn->setStreamType(0);
                            streamIn->setFrameRate(fps);
                            MediaMng::GetInstance().addStream(streamIn);
                            return streamIn;
                        }
                        else
                        {
                            DebugLog(<< "child device pull stream failed err:" << deviceId << ", " << err);
                            return NULL;
                        }
                    }
                    else
                    {
                        DebugLog(<< "create media stream failed ");
                    }
                }
                else
                {
                    DebugLog(<< " child device channel not found:" << deviceId);
                }
            }
        }
    }
    return NULL;
}
MediaStream::Ptr MediaMng::createVodStream(std::string deviceId, time_t start, time_t end)
{
    BaseDevice::Ptr parentDev = NULL;
    BaseChildDevice* childDev = NULL;
    if (!deviceId.empty())
    {
        childDev = DeviceMng::Instance().findChildDevice(deviceId);
        if (childDev)
        {
            parentDev = childDev->getParentDev();
        }
        else
        {
            LogOut("BLL", L_ERROR, "%s child device not found", deviceId.c_str());
            return NULL;
        }
    }
    else
    {
        LogOut("BLL", L_ERROR, "deviceId is null");
        return NULL;
    }

    if (parentDev)
    {
        if (parentDev->devType == BaseDevice::JSON_NVR)
        {
            auto Nvr = std::dynamic_pointer_cast<JsonNvrDevic>(parentDev);
            if (Nvr)
            {
                int err = 0, chl = -1;
                uint32_t msgSize = 4 * 1024 * 1024;
                char* Buffer = new char[msgSize];
                Nvr->Dev_ListIPC(Buffer, msgSize, err);
                if (err == 0)
                {
                    // LogOut("BLL", L_ERROR, "nvr ipc list:%s,%s", Nvr->getIp().c_str(), Buffer);
                    rapidjson_sip::Document document;
                    document.Parse(Buffer);
                    if (!document.HasParseError())
                    {
                        if (document.HasMember("ipc_list") && document["ipc_list"].IsArray())
                        {
                            rapidjson_sip::Value& body = document["ipc_list"];
                            for (uint32_t i = 0; i < body.Size(); i++)
                            {
                                std::string devNum = json_check_string(body[i], "device_number");
                                std::size_t sPos = devNum.find(" ");
                                if (sPos != std::string::npos)
                                {
                                    devNum = devNum.substr(0, sPos);
                                }
                                JsonChildDevic* pChild = dynamic_cast<JsonChildDevic*>(childDev);
                                const char* devName = devNum.c_str();
                                const char* chidName = pChild->getName().c_str();
                                if (devNum == pChild->getName())
                                {
                                    int enable_flag = json_check_int32(body[i], "enable_flag");
                                    chl = json_check_int32(body[i], "chid");
                                    if (pChild->getChannel() != chl)
                                    {
                                        pChild->setChannel(chl);
                                    }
                                    if (enable_flag != 2)
                                    {
                                        LogOut("BLL", L_WARN, "%s %s child device enable_flag :%d", deviceId.c_str(), devNum.c_str(), enable_flag);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    LogOut("BLL", L_ERROR, "%s get channel info err: %d", deviceId.c_str(), err);
                }
                delete[] Buffer; Buffer = NULL;
                if (chl >= 0)
                {
                    ////get channel encoder param
                    //char Buffer[1024] = { 0 };
                    //uint32_t bufSize = 1024;
                    //int fps = 25;
                    //Nvr->GetChannelEncoderParam(chl, Buffer, &bufSize, err);
                    //if (err == 0)
                    //{
                    //	printf("get channel encoder param ok:%s\n", Buffer);
                    //	rapidjson_sip::Document document;
                    //	document.Parse(Buffer);
                    //	if (!document.HasParseError())
                    //	{

                    //	}
                    //}

                    std::string streamId = CreateStreamId(deviceId, start, end);

                    JsonStream::Ptr streamIn = std::make_shared<JsonStream>(deviceId.c_str(), streamId.c_str());
                    ULHandle playhandle = Nvr->Dev_PlayBack(chl, start, end, (void*)JsonStream::DataPlayCallBack2, NULL/*(void*)JsonStream::PlayBackEndCb*/, (void*)streamIn.get(), err);
                    if (err == 0)
                    {
                        //Nvr->Dev_PlayBackCtrl(playhandle, JsonNvrDevic::JsonPbCtrl_Speed, 1024, 0, err);
                        LogOut("BLL", L_INFO, "%s child device pull stream ok handle:%lu streameId:%s", deviceId.c_str(), playhandle, streamId.c_str());
                        streamIn->setStreamHandle(playhandle);
                        streamIn->setStreamType(1);
                        streamIn->setFrameRate(25);
                        MediaMng::GetInstance().addStream(streamIn);
                        return streamIn;
                    }
                    else
                    {
                        LogOut("BLL", L_ERROR, "%s child device pull stream failed err:%d", deviceId.c_str(), err);
                        return NULL;
                    }
                }
                else
                {
                    LogOut("BLL", L_ERROR, "%s child device channel not found\n", deviceId.c_str());
                }
            }
        }
    }
    return NULL;
}
std::string MediaMng::CreateStreamId(const std::string& deviceId, time_t start, time_t end)
{
    std::stringstream ss;
    ss << std::put_time(localtime(&start), "%Y-%m-%dT%H-%M-%S");
    std::string startTime = ss.str();
    ss.str("");
    ss << std::put_time(localtime(&end), "%Y-%m-%dT%H-%M-%S");
    std::string endTime = ss.str();
    ss.str("");
    ss << deviceId << "_" << startTime << "_" << endTime;

    return ss.str();
}
bool MediaMng::CloseStreamByStreamId(MediaStream::Ptr& ms)
{
    if (ms)
    {
        ms->reduction();
        LogOut("BLL", L_ERROR, "xxxxxxxxxxxxxxxxx close stream:%s ref:%d", ms->getStreamId().c_str(), ms->refNum());
        if (ms->refNum() == 0)
        {
            BaseChildDevice* childDev = DeviceMng::Instance().findChildDevice(ms->getDeviceId());
            if (childDev)
            {
                BaseDevice::Ptr parentDev = childDev->getParentDev();
                if (parentDev && parentDev->devType == BaseDevice::JSON_NVR)
                {
                    auto JsonNvr = std::dynamic_pointer_cast<JsonNvrDevic>(parentDev);
                    if (JsonNvr)
                    {
                        int err = 0;
                        if (ms->getStreamType() == 0)
                        {
                            JsonNvr->Dev_StopPreview(ms->getStreamHandle(), err);
                            LogOut("BLL", L_WARN, "stop preview handle:%lu,err:%d", ms->getStreamHandle(), err);
                        }
                        else if (ms->getStreamType() == 1)
                        {
                            JsonNvr->Dev_StopPlayBack(ms->getStreamHandle(), err);
                            LogOut("BLL", L_WARN, "stop play back handle:%lu,err:%d", ms->getStreamHandle(), err);
                        }
                        else
                        {
                            JsonNvr->Dev_StopDownload(ms->getStreamHandle(), err);
                            LogOut("BLL", L_WARN, "stop download handle:%lu,err:%d", ms->getStreamHandle(), err);
                        }
                        if (err == 0)
                        {
                            MediaMng::GetInstance().removeStream(ms->getStreamId());
                            LogOut("BLL", L_WARN, "remove stream:%s", ms->getStreamId().c_str());
                        }
                        else
                        {
                            LogOut("BLL", L_ERROR, "stop stream:%s, failed:%d", ms->getStreamId().c_str(), err);
                        }
                    }
                }
            }
        }
    }
    return true;
}
bool MediaMng::GB28181QueryRecordInfo(RecordInfoQueryMsg recordQuery, std::list<RecordInfoResponseItem> &recordlist)
{
    BaseDevice::Ptr parentDev = NULL;
    BaseChildDevice* childDev = NULL;
    if (!recordQuery.DeviceID.empty())
    {
        childDev = DeviceMng::Instance().findChildDevice(recordQuery.DeviceID);
        if (childDev)
        {
            parentDev = childDev->getParentDev();
        }
        else
        {
            DebugLog(<< recordQuery.DeviceID << "  child device not found");
            return false;
        }
    }
    else
    {
        DebugLog(<< "deviceId is null");
        return false;
    }

    if (parentDev)
    {
        if (parentDev->devType == BaseDevice::JSON_NVR)
        {
            auto Nvr = std::dynamic_pointer_cast<JsonNvrDevic>(parentDev);
            if (Nvr)
            {
                int err = 0, chl = -1;
                std::string chlName;
                uint32_t msgSize = 4 * 1024 * 1024;
                char* Buffer = new char[msgSize];
                Nvr->Dev_ListIPC(Buffer, msgSize, err);
                if (err == 0)
                {
                    rapidjson_sip::Document document;
                    document.Parse(Buffer);
                    if (!document.HasParseError())
                    {
                        if (document.HasMember("ipc_list") && document["ipc_list"].IsArray())
                        {
                            rapidjson_sip::Value& body = document["ipc_list"];
                            for (uint32_t i = 0; i < body.Size(); i++)
                            {
                                std::string devNum = json_check_string(body[i], "device_number");
                                std::size_t sPos = devNum.find(" ");
                                if (sPos != std::string::npos)
                                {
                                    devNum = devNum.substr(0, sPos);
                                }
                                JsonChildDevic* pChild = dynamic_cast<JsonChildDevic*>(childDev);
                                if (pChild && devNum == pChild->getName())
                                {
                                    int enable_flag = json_check_int32(body[i], "enable_flag");
                                    if (enable_flag == 2)
                                    {
                                        int child = json_check_int32(body[i], "chid");
                                        int online = json_check_int32(body[i], "online_status");
                                        chl = child;
                                        chlName = devNum;
                                        if(pChild->getChannel() != child)
                                        {
                                            pChild->setChannel(child);
                                        }
                                        if (online == 1)
                                        {
                                            std::string Id = json_check_string(body[i], "device_id");
                                            pChild->setStatus(1);
                                        }
                                        else
                                        {
                                            pChild->setStatus(0);
                                            DebugLog(<< "child device offline " << recordQuery.DeviceID << " " << devNum);
                                        }
                                    }
                                    else
                                    {
                                        DebugLog(<< recordQuery.DeviceID << " " << devNum << " child device enable_flag :" << enable_flag);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    DebugLog(<< recordQuery.DeviceID << " " << err << "  get channel info err");
                }
                delete[] Buffer; Buffer = NULL;
                if(chl >= 0)
                {
                    struct tm tm_time;
                    time_t stimestamp, etimestamp;
                    if(strptime(recordQuery.StartTime.c_str(), "%Y-%m-%dT%H:%M:%S", &tm_time) != nullptr)
                    {
                        tm_time.tm_isdst = -1; // 设置 DST 为自动检测
                        stimestamp = mktime(&tm_time);
                    }
                    if(strptime(recordQuery.EndTime.c_str(), "%Y-%m-%dT%H:%M:%S", &tm_time) != nullptr)
                    {
                        tm_time.tm_isdst = -1; // 设置 DST 为自动检测
                        etimestamp = mktime(&tm_time);
                    }
                    std::list<JsonRecordInfo> records;
                    Nvr->SearchRecordInfoList(chl, stimestamp, etimestamp, records, err);
                    if(err == 0)
                    {
                        LogOut("BLL", L_DEBUG, "records size:%Zu", records.size());
                        for(auto &it : records)
                        {
                            RecordInfoResponseItem recordItem;
                            recordItem.DeviceID = recordQuery.DeviceID;
                            recordItem.Name = chlName;
                            recordItem.FilePath = recordQuery.DeviceID;
                            recordItem.Address = "";
                            LogOut("BLL", L_DEBUG, "file start time:%u, end time:%u", it.start_time, it.end_time);
                            struct tm tmtime;
                            char strTime[32] = { 0 };
                            time_t ttime = it.start_time;
                            localtime_r(&ttime, &tmtime);
                            strftime(strTime, sizeof(strTime), "%Y-%m-%dT%H:%M:%S", &tmtime);
                            recordItem.StartTime = strTime;

                            ttime = it.end_time;
                            localtime_r(&ttime, &tmtime);
                            strftime(strTime, sizeof(strTime), "%Y-%m-%dT%H:%M:%S", &tmtime);
                            recordItem.EndTime = strTime;
                            recordItem.Secrecy = 0;
                            recordItem.RecordType = "all";
                            recordItem.RecorderID = recordQuery.DeviceID;//Nvr->deviceId;
                            recordlist.push_back(recordItem);
                        }
                        return true;
                    }
                }
                else
                {
                    DebugLog(<< " child device channel not found:" << recordQuery.DeviceID);
                }
            }
        }
    }
    return false;
}