#include "JsonDevice.h"
#include "../media/mediaIn/JsonStream.h"

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

void JsonNvrDevic::setStatus(int statu)
{
	status = statu;
}
const int JsonNvrDevic::getStatus()
{
	return status;
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
		mLoginId = JsonSdkInterface::Instance().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
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
		JsonSdkInterface::Instance().LogOut(mLoginId, err);
		if (err == 3001)
		{
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
		mLoginId = JsonSdkInterface::Instance().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
		if (err != 0)
		{

		}
	}
	if (mLoginId > 0)
	{
		ULHandle ulPreviewHandle = JsonSdkInterface::Instance().VskPreview(mLoginId, channelId, streamType, (DataPlayCallBack)VideoTranCallBack, pUser, err);
		if (err == 3001)
		{
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
	if (err == 0 && mLoginId == 0)
	{
		mLoginId = JsonSdkInterface::Instance().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
		if (err != 0)
		{

		}
	}
	if (mLoginId > 0)
	{
		JsonSdkInterface::Instance().StopPreview(handle, err);
		if (err == 3001)
		{
			mLoginId = 0;
		}
	}
}

ULHandle JsonNvrDevic::Dev_PlayBack(int channelId, long start, long end, void* VideoTranCallBack, void* fun, void* pUser, int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
	}
	if (err == 0 && mLoginId == 0)
	{
		mLoginId = JsonSdkInterface::Instance().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
		if (err != 0)
		{

		}
	}
	if (mLoginId > 0)
	{
		ULHandle ulPreviewHandle = JsonSdkInterface::Instance().PlayBack(mLoginId, channelId, start, end, (DataPlayCallBack)VideoTranCallBack, (PlayBackEndCallBack)fun, pUser, err);
		if (err == 3001)
		{
			mLoginId = 0;
		}
		return ulPreviewHandle;
	}
	return 0;
}
void JsonNvrDevic::Dev_StopPlayBack(ULHandle, int& err)
{
}

ULHandle JsonNvrDevic::Dev_Download(int channelId, long start, long end, DownloadData VideoTranCallBack, DownloadEnd fun, void* pUser, int& err)
{
	return 0;
}
void JsonNvrDevic::Dev_StopDownload(ULHandle, int& err)
{
}

void JsonNvrDevic::Dev_PTZCtrl(int channelId, uint32_t PTZCommand, uint32_t Argument, int& err)
{
}
void JsonNvrDevic::Dev_ListIPC(char* Buffer, uint32_t &BufSize, int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
		if (err != 0)
		{
			printf("json sdk init err:%d\n", err);
		}
	}
	if (err == 0 && mLoginId == 0)
	{
		mLoginId = JsonSdkInterface::Instance().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
		if (err != 0)
		{
			printf("json sdk login err:%d\n", err);
		}
	}
	if (mLoginId > 0)
	{
		uint32_t size = BufSize;
		JsonSdkInterface::Instance().ListIPC(mLoginId, Buffer, &size, err);
		if (err != 0)
		{
			printf("json sdk get ipc list err:%d\n", err);
			if (err == 3001)
			{
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










JsonChildDevic::JsonChildDevic(const char* Id) :BaseChildDevice(Id), channel(-1)
{
}
JsonChildDevic::~JsonChildDevic()
{
}
//void JsonChildDevic::setParentDev(JsonNvrDevic::Ptr pdev)
//{
//	mParentDev = pdev;
//}
//const JsonNvrDevic::Ptr JsonChildDevic::getParentDev()
//{
//	return mParentDev;
//}
void JsonChildDevic::setName(std::string nm)
{
	name = nm;
}
const std::string JsonChildDevic::getName()
{
	return name;
}
void JsonChildDevic::setStatus(int statu)
{
	status = statu;
}
const int JsonChildDevic::getStatus()
{
	return status;
}
void JsonChildDevic::setParentId(std::string pId)
{
	parentId = pId;
}
const std::string JsonChildDevic::getParentId()
{
	return parentId;
}
void JsonChildDevic::setChildIp(std::string Ip)
{
	ChildIp = Ip;
}
const std::string JsonChildDevic::getChildIp()
{
	return ChildIp;
}
void JsonChildDevic::setChannel(int chl)
{
	channel = chl;
}
const int JsonChildDevic::getChannel()
{
	return channel;
}