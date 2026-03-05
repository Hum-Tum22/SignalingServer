#include "JsonDevice.h"
#include "SelfLog.h"
#include "../media/mediaIn/JsonStream.h"
#include "../tools/CodeConversion.h"

JsonNvrDevic::JsonNvrDevic(const char* Id, const char* Ip, int port, const char* user, const char* pswd) :BaseDevice(Id, JSON_NVR)
{
    setIp(Ip);
    setPort(port);
    setUser(user);
    setPswd(pswd);
    setStatus(0);
    mLoginId = 0;
};
JsonNvrDevic::~JsonNvrDevic()
{
    DevDisConnect();
}
bool JsonNvrDevic::DevConnect()
{
    int err = 0;
    JsonNvrLogIn(err);
    return err ? false : true;
}
bool JsonNvrDevic::DevDisConnect()
{
    int err = 0;
    JsonNvrLogOut(err);
    return err ? false : true;
}

JsonNvrDevic::PTZCONTROL_COMMAND JsonNvrDevic::switchFromGB28181(PTZCMDType::GB28181PtzCmd cmd)
{
    switch (cmd)
    {
    case PTZCMDType::PTZ_NO:
    case PTZCMDType::PTZ_STOP:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_UNKNOWN;
    }
    case PTZCMDType::PTZ_UP:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_UP;
    }
    case PTZCMDType::PTZ_DOWN:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_DOWN;
    }
    case PTZCMDType::PTZ_LEFT:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_LEFT;
    }
    case PTZCMDType::PTZ_RIGHT:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_RIGHT;
    }
    case PTZCMDType::PTZ_UP_LEFT:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_UP_LEFT;
    }
    case PTZCMDType::PTZ_UP_RIGHT:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_UP_RIGHT;
    }
    case PTZCMDType::PTZ_DOWN_LEFT:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_DOWN_LEFT;
    }
    case PTZCMDType::PTZ_DOWN_RIGHT:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_DOWN_RIGHT;
    }
    case PTZCMDType::PTZ_ZOOM_IN:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_ZOOM_IN;
    }
    case PTZCMDType::PTZ_ZOOM_OUT:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_ZOOM_OUT;
    }
    case PTZCMDType::FOCUS_IN:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_FOCUS_NEAR;
    }
    case PTZCMDType::FOCUS_OUT:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_FOCUS_FAR;
    }
    case PTZCMDType::IRIS_IN:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_IRIS_OPEN;
    }
    case PTZCMDType::IRIS_OUT:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_IRIS_CLOSE;
    }
    case PTZCMDType::PTZ_SETPRESET:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_SAVE_PRESET;
    }
    case PTZCMDType::PTZ_GTOPRESET:
    case PTZCMDType::PTZ_DELPRESET:
    {
        break;
    }
    case PTZCMDType::PTZ_MOVPRESET:
    {
        return JsonNvrDevic::JSON_PTZCOMMAND_CALL_PRESET;
    }
    default:
        break;
    }
    return JsonNvrDevic::JSON_PTZCOMMAND_UNKNOWN;
}

void JsonNvrDevic::JsonNvrLogIn(int& err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
    }
    if (err == 0 && mLoginId == 0)
    {
        mLoginId = JsonSdkInterface::Instance().JsonSdkLogIn(mIP.c_str(), mPort, mUser.c_str(), mPswd.c_str(), err);
        if (err != 0)
        {

        }
    }
}
void JsonNvrDevic::JsonNvrLogOut(int& err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
    }
    if (err == 0 && mLoginId == 0)
    {
        JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
        if (err == 3001 || err == 2007)
        {
            JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
            mLoginId = 0;
        }
    }
}
void JsonNvrDevic::GetChannelEncoderParam(int channel, char* pBuffer, uint32_t* pInfoSize, int& err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
    }
    if (err == 0 && mLoginId == 0)
    {
        mLoginId = JsonSdkInterface::Instance().JsonSdkLogIn(mIP.c_str(), mPort, mUser.c_str(), mPswd.c_str(), err);
        if (err != 0)
        {
            return;
        }
    }
    if (mLoginId > 0)
    {
        JsonSdkInterface::Instance().GetChannelEncoderParam(mLoginId, channel, pBuffer, pInfoSize, err);
        if (err == 3001 || err == 2007)
        {
            JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
            mLoginId = 0;
        }
    }
}
ULHandle JsonNvrDevic::Dev_Preview(int channelId, int streamType, void* VideoTranCallBack, void* pUser, int& err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
    }
    if (err == 0 && mLoginId == 0)
    {
        mLoginId = JsonSdkInterface::Instance().JsonSdkLogIn(mIP.c_str(), mPort, mUser.c_str(), mPswd.c_str(), err);
        if (err != 0)
        {

        }
    }
    if (mLoginId > 0)
    {
        ULHandle ulPreviewHandle = JsonSdkInterface::Instance().VskPreview(mLoginId, channelId, streamType, (DataPlayCallBack)VideoTranCallBack, pUser, err);
        if (err == 3001 || err == 2007)
        {
            JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
            mLoginId = 0;
        }
        return ulPreviewHandle;
    }
    return 0;
}
void JsonNvrDevic::Dev_StopPreview(ULHandle handle, int& err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
    }
    if (mLoginId > 0)
    {
        JsonSdkInterface::Instance().StopPreview(handle, err);
        if (err == 3001 || err == 2007)
        {
            JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
            mLoginId = 0;
        }
    }
}
void JsonNvrDevic::SearchRecordInfoList(int channel, uint64_t st, uint64_t et, std::list<JsonRecordInfo> &records, int &err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
    }
    if (err == 0 && mLoginId == 0)
    {
        mLoginId = JsonSdkInterface::Instance().JsonSdkLogIn(mIP.c_str(), mPort, mUser.c_str(), mPswd.c_str(), err);
        if (err != 0)
        {

        }
    }
    if (mLoginId > 0)
    {
        int ret = JsonSdkInterface::Instance().SearchRecordFileList(mLoginId, channel, st, et, records, err);
        if(err == 3001 || err == 2007)
        {
            JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
            mLoginId = 0;
        }
    }
}
ULHandle JsonNvrDevic::Dev_PlayBack(int channelId, long start, long end, void *VideoTranCallBack, void *fun, void *pUser, int &err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
    }
    if (err == 0 && mLoginId == 0)
    {
        mLoginId = JsonSdkInterface::Instance().JsonSdkLogIn(mIP.c_str(), mPort, mUser.c_str(), mPswd.c_str(), err);
        if (err != 0)
        {

        }
    }
    if (mLoginId > 0)
    {
        ULHandle ulPreviewHandle = JsonSdkInterface::Instance().PlayBack(mLoginId, channelId, start, end, (DataPlayCallBack)VideoTranCallBack, (PlayBackEndCallBack)fun, pUser, err);
        if (err == 3001 || err == 2007)
        {
            JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
            mLoginId = 0;
        }
        return ulPreviewHandle;
    }
    return 0;
}
void JsonNvrDevic::Dev_StopPlayBack(ULHandle handle, int& err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
    }
    if (mLoginId > 0)
    {
        JsonSdkInterface::Instance().StopPlayBack(handle, err);
        if (err == 3001 || err == 2007)
        {
            JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
            mLoginId = 0;
        }
    }
}
void JsonNvrDevic::Dev_PlayBackCtrl(ULHandle handle, int cmd, int param1, int param2, int& err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
    }
    if (mLoginId > 0)
    {
        JsonSdkInterface::Instance().PlayBackCtrl(handle, cmd, param1, param2, err);
        if (err == 3001 || err == 2007)
        {
            JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
            mLoginId = 0;
        }
    }
}
void JsonNvrDevic::Dev_PbCtrlTimePos(ULHandle handle, time_t pos, int& err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
    }
    if (mLoginId > 0)
    {
        JsonSdkInterface::Instance().setTimePos(handle, pos, err);
        if (err == 3001 || err == 2007)
        {
            JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
            mLoginId = 0;
        }
    }
}
ULHandle JsonNvrDevic::Dev_Download(int channelId, long start, long end, void* VideoTranCallBack, void* fun, void* pUser, int& err)
{
    return 0;
}
void JsonNvrDevic::Dev_StopDownload(ULHandle, int& err)
{
}

void JsonNvrDevic::Dev_PTZCtrl(int channelId, uint32_t PTZCommand, uint32_t Argument, int& err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
        if (err != 0)
        {
            LogOut(SDK, L_ERROR, "json sdk init err:%d", err);
        }
    }
    if (err == 0 && mLoginId == 0)
    {
        mLoginId = JsonSdkInterface::Instance().JsonSdkLogIn(mIP.c_str(), mPort, mUser.c_str(), mPswd.c_str(), err);
        if (err != 0)
        {
            LogOut(SDK, L_ERROR, "json sdk login err:%d,ip:%s, port:%d, user:%s,pswd:%s", err, mIP.c_str(), mPort, mUser.c_str(), mPswd.c_str());
        }
    }
    if (mLoginId > 0)
    {
        LogOut(SDK, L_INFO, "json sdk ptz ctrl ip:%s, chid:%d, cmd:%d", mIP.c_str(), channelId, PTZCommand);
        JsonSdkInterface::Instance().PTZCtrl(mLoginId, channelId, PTZCommand, Argument, err);
        if (err != 0)
        {
            LogOut(SDK, L_ERROR, "json sdk ptz ctrl err:%d, ip:%s", err, mIP.c_str());
            if (err == 3001 || err == 2007)
            {
                JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
                mLoginId = 0;
            }
        }
    }
    return;
}
void JsonNvrDevic::Dev_ListIPC(char* Buffer, uint32_t& BufSize, int& err)
{
    err = 0;
    if (!JsonSdkInterface::Instance().SdkIsInit())
    {
        JsonSdkInterface::Instance().InitSdk(err);
        if (err != 0)
        {
            LogOut(SDK, L_ERROR, "json sdk init err:%d", err);
        }
    }
    if (err == 0 && mLoginId == 0)
    {
        mLoginId = JsonSdkInterface::Instance().JsonSdkLogIn(mIP.c_str(), mPort, mUser.c_str(), mPswd.c_str(), err);
        if (err != 0)
        {
            LogOut(SDK, L_ERROR, "json sdk login err:%d,ip:%s, port:%d, user:%s,pswd:%s", err, mIP.c_str(), mPort, mUser.c_str(), mPswd.c_str());
        }
    }
    if (mLoginId > 0)
    {
        uint32_t size = BufSize;
        JsonSdkInterface::Instance().ListIPC(mLoginId, Buffer, &size, err);
        if (err != 0)
        {
            LogOut(SDK, L_ERROR, "json sdk get ipc list err:%d, ip:%s", err, mIP.c_str());
            if (err == 3001 || err == 2007)
            {
                JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
                mLoginId = 0;
            }
        }
        if (err == 0)
        {
            BufSize = size;
        }
    }
    return;
}










JsonChildDevic::JsonChildDevic(const char* Id) :BaseChildDevice(Id)
{
    setChannel(-1);
    setLastPtzCmd(-1);
    setStatus(0);
}
JsonChildDevic::~JsonChildDevic()
{
}