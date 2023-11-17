#include "jsonSdkInterface.h"

#include "../../../lib/rapidjson/document.h"
#include "../../../lib/rapidjson/stringbuffer.h"
#include "../../../lib/rapidjson/writer.h"
#include "include/base64.h"
#include "include/base64.cpp"

JsonSdkInterface& JsonSdkInterface::Instance()
{
	static JsonSdkInterface g_JsonSdkInterface;
	return g_JsonSdkInterface;
}
JsonSdkInterface::JsonSdkInterface() :PluginInter(PluginInter::JSON_SDK), m_hDll(NULL), IsInit(false)
{
	InitFun = NULL;
	ClearFun = NULL;
	LoginFun = NULL;
	LogOutFun = NULL;
	PreviewFun = NULL;
	StopPreviewFun = NULL;
	PlayBackFun = NULL;
	StopPlayBackFun = NULL;
	DownloadFun = NULL;
	StopDownloadFun = NULL;
	PtzCtrlFun = NULL;
#ifdef _WIN32
	m_hDll = LoadLibrary(L"sdkJson.dll");
#else
	m_hDll = dlopen("sdkJson.so", RTLD_LOCAL);
#endif
}
JsonSdkInterface::~JsonSdkInterface()
{
	if (m_hDll)
	{
		int err;
		SdkClear(err);
		if (err == 0)
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
	if (m_hDll)
	{
		if (!InitFun)
		{
			InitFun = (Sdk_Initate)LoadSharedLibFun(m_hDll, "JsonSdk_Initate");
		}
		if (InitFun)
		{
			int ret = InitFun();
			if (ret == 0)
			{
				IsInit = true;
			}
			else
			{
				err = ret;
			}
		}
		else
		{
			err = -2;
		}
	}
	else
	{
		err = -1;
	}
}
void JsonSdkInterface::SdkClear(int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!ClearFun)
		{
			ClearFun = (Sdk_Clear)LoadSharedLibFun(m_hDll, "JsonSdk_Clear");
		}
		if (ClearFun)
		{
			int ret = ClearFun();
			if (ret == 0)
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
DWORD JsonSdkInterface::LogIn(const char* ip, int port, const char* name, const char* pswd, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!LoginFun)
		{
			LoginFun = (Sdk_Login)LoadSharedLibFun(m_hDll, "JsonSdk_Login");
		}
		if (LoginFun)
		{
			//ReqClientLogin
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
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
			DWORD nLoginID;
			int ret = LoginFun((char*)strJsonReq.c_str(), &nLoginID, NULL, NULL, NULL, NULL);
			if (ret == 0)
			{
				return nLoginID;
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
void JsonSdkInterface::LogOut(DWORD loginId, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!LogOutFun)
		{
			LogOutFun = (Sdk_Logout)LoadSharedLibFun(m_hDll, "JsonSdk_Logout");
		}
		if (LogOutFun && loginId)
		{
			int ret = LogOutFun(loginId);
			if (ret == 0)
			{
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
	return ;
}
DWORD JsonSdkInterface::Preview(DWORD UserID, int channel, int streamId, DataVideoAudioCallBackEx VideoTranCallBack, LPVOID pUser, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!PreviewFun)
		{
			PreviewFun = (Sdk_Preview)LoadSharedLibFun(m_hDll, "JsonSdk_VideoTranspondStartWithoutViskhead");
		}
		if (UserID && PreviewFun)
		{
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
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
			DWORD hHandle = 0;
			int ret = PreviewFun(UserID, VideoTranCallBack, (char*)strJsonReq.c_str(), NULL, pUser, &hHandle);
			if (ret == 0)
			{
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
void JsonSdkInterface::StopPreview(DWORD UserID, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!StopPreviewFun)
		{
			StopPreviewFun = (Sdk_StopPreview)LoadSharedLibFun(m_hDll, "JsonSdk_VideoTranspondStop");
		}
		if (UserID && StopPreviewFun)
		{
			int ret = StopPreviewFun(UserID);
			if (ret == 0)
			{
				return ;
			}
			err = ret;
			return ;
		}
		err = -2;
		return ;
	}
	err = -1;
	return ;
}
DWORD JsonSdkInterface::PlayBack(DWORD UserID, int channel, long start, long end, DataPlayCallBack VideoTranCallBack, PlayBackEndCallBack fun, LPVOID pUser, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!PlayBackFun)
		{
			PlayBackFun = (Sdk_PlayBackStartByTime)LoadSharedLibFun(m_hDll, "JsonSdk_PlayBackStartByTime");
		}
		if (UserID && PlayBackFun)
		{
			//ReqStartPlaybackStreamTime
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
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
			DWORD hHandle = 0;
			uint64_t  nTotalFileSize = 0;
			int ret = PlayBackFun(UserID, strJsonReq.c_str(), VideoTranCallBack, fun, NULL, this, &nTotalFileSize, &hHandle);
			if (ret == 0)
			{
				return hHandle;
			}
			err = ret;
			return 0;
		}
		err = -2;
		return 0;
	}
	err = -1;
	return 0;
}
void JsonSdkInterface::StopPlayBack(DWORD UserID, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!StopPlayBackFun)
		{
			StopPlayBackFun = (Sdk_PlayBackStop)LoadSharedLibFun(m_hDll, "JsonSdk_PlayBackStop");
		}
		if (UserID && StopPlayBackFun)
		{
			int ret = StopPlayBackFun(UserID);
			if (ret == 0)
			{
				return ;
			}
			err = ret;
			return ;
		}
		err = -2;
		return ;
	}
	err = -1;
	return ;
}

DWORD JsonSdkInterface::Download(DWORD UserID, int channel, long start, long end, DataPlayCallBack VideoTranCallBack, PlayBackEndCallBack fun, LPVOID pUser, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!DownloadFun)
		{
			DownloadFun = (Sdk_Download)LoadSharedLibFun(m_hDll, "JsonSdk_DownloadStartByTime");
		}
		if (UserID && DownloadFun)
		{
			//ReqStartPlaybackStreamTime
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
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
			DWORD hHandle = 0;
			uint64_t  nTotalFileSize = 0;
			int ret = DownloadFun(UserID, strJsonReq.c_str(), VideoTranCallBack, fun, NULL, pUser, 0, &nTotalFileSize, &hHandle);
			if (ret == 0)
			{
				return hHandle;
			}
			err = ret;
			return 0;
		}
		err = -2;
		return 0;
	}
	err = -1;
	return 0;
}
void JsonSdkInterface::StopDownload(DWORD UserID, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!StopDownloadFun)
		{
			StopDownloadFun = (Sdk_DownloadStop)LoadSharedLibFun(m_hDll, "JsonSdk_DownloadStop");
		}
		if (UserID && StopDownloadFun)
		{
			int ret = StopDownloadFun(UserID);
			if (ret == 0)
			{
				return;
			}
			err = ret;
			return;
		}
		err = -2;
		return;
	}
	err = -1;
	return;
}
void JsonSdkInterface::PTZCtrl(DWORD UserID, uint32_t Channel, uint32_t PTZCommand, uint32_t Argument, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!PtzCtrlFun)
		{
			PtzCtrlFun = (Sdk_PTZCtrl)LoadSharedLibFun(m_hDll, "JsonSdk_PTZCtrl");
		}
		if (UserID && PtzCtrlFun)
		{
			int ret = PtzCtrlFun(UserID, Channel, PTZCommand, Argument);
			if (ret == 0)
			{
				return;
			}
			err = ret;
			return;
		}
		err = -2;
		return;
	}
	err = -1;
	return;
}
void JsonSdkInterface::ListIPC(DWORD UserID, char* pIPCServerList, UINT* pIPCServerListSize, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!ListIPCFun)
		{
			ListIPCFun = (Sdk_ListIPC)LoadSharedLibFun(m_hDll, "JsonSdk_ListIPC");
		}
		if (UserID && ListIPCFun)
		{
			int ret = ListIPCFun(UserID, pIPCServerList, pIPCServerListSize);
			if (ret == 0)
			{
				return;
			}
			err = ret;
			return;
		}
		err = -2;
		return;
	}
	err = -1;
	return;
}