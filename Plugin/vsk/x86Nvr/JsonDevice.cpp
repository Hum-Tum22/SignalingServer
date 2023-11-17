

#include "JsonDevice.h"
#include "jsonSdkInterface.h"

JsonSdkInterface& GetJsonSdkInterface()
{
	static JsonSdkInterface g_JsonSdk;
	return g_JsonSdk;
}
JsonNvrDevic::~JsonNvrDevic()
{
	int err = 0;
	JsonNvrLogOut(err);
	if (err != 0)
	{

	}
}
void JsonNvrDevic::JsonNvrLogIn(int& err)
{
	err = 0;
	if (!GetJsonSdkInterface().SdkIsInit())
	{
		GetJsonSdkInterface().InitSdk(err);
	}
	if (err == 0 && mLoginId == 0)
	{
		mLoginId = GetJsonSdkInterface().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
		if (err != 0)
		{

		}
	}
}
void JsonNvrDevic::JsonNvrLogOut(int& err)
{
	err = 0;
	if (!GetJsonSdkInterface().SdkIsInit())
	{
		GetJsonSdkInterface().InitSdk(err);
	}
	if (err == 0 && mLoginId == 0)
	{
		GetJsonSdkInterface().LogOut(mLoginId, err);
	}
}

JsonDevice::JsonDevice(const char* Ip, int port, const char* name, const char* pswd, int chl, int stream):BaseDevice(BaseDevice::SRC_DEVTYPE_X86_NVR),Ip(Ip)
	, nPort(port), UserName(name), PassWd(pswd), channel(chl), streamId(stream)
{
	ulPreviewHandle = 0;
	pvCb = NULL;
	pPvUser = NULL;

	ulPlaybackHandle = 0;
	PbCb = NULL;
	PbEnd = NULL;
	pPbUser = NULL;

	ulDownloadHandle = 0;
	DlCb = NULL;
	DlEnd = NULL;
	pDlUser = NULL;
	mParent = nullptr;
}
JsonDevice::~JsonDevice()
{
	int err = 0;
	if (ulPreviewHandle > 0)
	{
		Dev_StopPlayBack(ulPreviewHandle, err);
		if (err != 0)
		{
		}
	}
	if (ulPlaybackHandle > 0)
	{
		Dev_StopPlayBack(ulPlaybackHandle, err);
		if (err != 0)
		{
		}
	}
	if (ulDownloadHandle > 0)
	{
		Dev_StopDownload(ulPreviewHandle, err);
		if (err != 0)
		{
		}
	}
	if (psSource)
	{
		delete psSource;
		psSource = NULL;
	}
}
void JsonDevice::setParent(JsonNvrDevic::Ptr parent)
{
	mParent = parent;
}
BaseDevice::ULHandle JsonDevice::Dev_Preview(BaseDevice::previewCb VideoTranCallBack, void* pUser, int& err)
{
	err = 0;
	if (!GetJsonSdkInterface().SdkIsInit())
	{
		GetJsonSdkInterface().InitSdk(err);
	}
	if (err == 0 && mParent->mLoginId == 0)
	{
		mParent->JsonNvrLogIn(err);
	}
	if (mParent->mLoginId > 0)
	{
		pvCb = VideoTranCallBack;
		pPvUser = pUser;
		ulPreviewHandle = GetJsonSdkInterface().Preview(mParent->mLoginId, channel, streamId, DataCallBack, this, err);
		return ulPreviewHandle;
	}
	return 0;
}
void JsonDevice::Dev_StopPreview(ULHandle playhandle, int& err)
{
	err = 0;
	if (!GetJsonSdkInterface().SdkIsInit())
	{
		GetJsonSdkInterface().InitSdk(err);
	}
	if (mParent->mLoginId > 0)
	{
		GetJsonSdkInterface().StopPreview(playhandle, err);
	}
}

BaseDevice::ULHandle JsonDevice::Dev_PlayBack(long start, long end, BaseDevice::PbCbData VideoTranCallBack, BaseDevice::PbCbEnd fun, void* pUser, int& err)
{
	err = 0;
	if (!GetJsonSdkInterface().SdkIsInit())
	{
		GetJsonSdkInterface().InitSdk(err);
	}
	if (err == 0 && mParent->mLoginId == 0)
	{
		mParent->JsonNvrLogIn(err);
	}
	if (mParent->mLoginId > 0)
	{
		//DWORD Preview(DWORD UserID, int channel, int streamId, DataVideoAudioCallBackEx VideoTranCallBack, LPVOID pUser, int& err);
		//return GetJsonSdkInterface().PlayBack(nLoginID, channel, start, end, DataCallBack, this, err);
	}
	return 0;
}
void JsonDevice::Dev_StopPlayBack(ULHandle pbHandle, int& err)
{
	err = 0;
	if (!GetJsonSdkInterface().SdkIsInit())
	{
		GetJsonSdkInterface().InitSdk(err);
	}
	if (mParent->mLoginId > 0)
	{
		GetJsonSdkInterface().StopPlayBack(pbHandle, err);
	}
}

BaseDevice::ULHandle JsonDevice::Dev_Download(long start, long end, BaseDevice::DownloadData VideoTranCallBack, BaseDevice::DownloadEnd fun, void* pUser, int& err)
{
	err = 0;
	if (!GetJsonSdkInterface().SdkIsInit())
	{
		GetJsonSdkInterface().InitSdk(err);
	}
	if (err == 0 && mParent->mLoginId == 0)
	{
		mParent->JsonNvrLogIn(err);
	}
	if (mParent->mLoginId > 0)
	{
		//DWORD Preview(DWORD UserID, int channel, int streamId, DataVideoAudioCallBackEx VideoTranCallBack, LPVOID pUser, int& err);
		//return GetJsonSdkInterface().Download(nLoginID, channel, start, end, DataCallBack, NULL, this, err);
	}
	return 0;
}
void JsonDevice::Dev_StopDownload(BaseDevice::ULHandle dlHandle, int& err)
{
	err = 0;
	if (!GetJsonSdkInterface().SdkIsInit())
	{
		GetJsonSdkInterface().InitSdk(err);
	}
	if (mParent->mLoginId > 0)
	{
		return GetJsonSdkInterface().StopDownload(dlHandle, err);
	}
}

void JsonDevice::Dev_PTZCtrl(uint32_t PTZCommand, uint32_t Argument, int& err)
{
	err = 0;
	if (!GetJsonSdkInterface().SdkIsInit())
	{
		GetJsonSdkInterface().InitSdk(err);
	}
	if (err == 0 && mParent->mLoginId == 0)
	{
		mParent->JsonNvrLogIn(err);
	}
	if (mParent->mLoginId > 0)
	{
		return GetJsonSdkInterface().PTZCtrl(mParent->mLoginId, channel, PTZCommand, Argument, err);
	}
}

void CALL_METHOD JsonDevice::DataCallBack(unsigned int PlayHandle, uint8_t* pBuffer, unsigned int BufferSize, unsigned int DateType, time_t systime, unsigned int TimeSpace, LPVOID pUser)
{
	JsonDevice* pThis = (JsonDevice*)pUser;
	if (pThis)
	{
		if (DateType == 0)
		{
			pThis->OnPreviewData(PlayHandle, pBuffer, BufferSize);
		}
	}
}
void JsonDevice::OnPreviewData(unsigned int PlayHandle, const uint8_t* pBuffer, unsigned int BufferSize)
{
	if (pvCb)
	{
		pvCb(PlayHandle, pBuffer, BufferSize, pPvUser);
	}
}