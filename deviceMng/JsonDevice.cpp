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
		ULHandle ulPreviewHandle = JsonSdkInterface::Instance().Preview(mLoginId, channelId, streamType, (DataVideoAudioCallBackEx)VideoTranCallBack, pUser, err);
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

ULHandle JsonNvrDevic::Dev_PlayBack(int channelId, long start, long end, PbCbData VideoTranCallBack, PbCbEnd fun, void* pUser, int& err)
{
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
void JsonNvrDevic::Dev_ListIPC(char* Buffer, uint32_t BufSize, int& err)
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
		JsonSdkInterface::Instance().ListIPC(mLoginId, Buffer, &BufSize, err);
		if (err == 3001)
		{
			mLoginId = 0;
		}
	}
	return;
}
void __stdcall JsonNvrDevic::VskX86NvrRtPreDataCb(uint32_t PlayHandle, uint8_t* pBuffer, uint32_t BufferSize, uint32_t DateType, time_t systime, uint32_t TimeSpace, void* pUser)
{
	JsonNvrDevic* pThis = (JsonNvrDevic*)pUser;
	if (pThis)
	{
		//pThis->
	}
}









JsonChildDevic::JsonChildDevic(const char* Id) :BaseChildDevice(Id)
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
