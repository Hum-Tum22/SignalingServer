#include "jsonSdkInterface.h"

#include "../../../lib/rapidjson/document.h"
#include "../../../lib/rapidjson/stringbuffer.h"
#include "../../../lib/rapidjson/writer.h"
#include "include/base64.h"
#include <errno.h>
#include <exception>
#include <iostream>
//#include "include/base64.cpp"

JsonSdkInterface& JsonSdkInterface::Instance()
{
	static JsonSdkInterface *g_JsonSdkInterface = new JsonSdkInterface();
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
	m_hDll = dlopen("libsdkJson.so", RTLD_NOW);
	if (!m_hDll)
	{
		printf("sdkJson.so load err:%s\n", dlerror());
	}
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
				printf("json sdk init err:%d\n", err);
				err = ret;
			}
		}
		else
		{
			printf("get JsonSdk_Initate err:%d\n", err);
			err = -2;
		}
	}
	else
	{
		err = -1;
		printf("Load json sdk err:%d\n", err);
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
JSONLONG JsonSdkInterface::LogIn(const char* ip, int port, const char* name, const char* pswd, int& err)
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
				if (ret == 0)
				{
					printf("json sdk  login ok %ld\n", nLoginID);
					return nLoginID;
				}
				else
				{
					printf("json sdk  login err:%d\n", ret);
					err = ret;
				}
			}
			catch (std::exception& e)
			{
				std::cout << "Standard exception: " << e.what() << std::endl;
			}
		}
		else
		{
			printf("get JsonSdk_Login err:%d\n", err);
			err = -2;
		}
	}
	else
	{
		err = -1;
		printf("Load json sdk err:%d\n", err);
	}
	return 0;
}
void JsonSdkInterface::LogOut(JSONLONG loginId, int& err)
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
				printf("json sdk  logout %ld\n", loginId);
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
void JsonSdkInterface::GetChannelEncoderParam(JSONLONG UserID, JSONLONG chid, char* pInfo, uint32_t* pInfoSize, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!GetChannelParam)
		{
			GetChannelParam = (Sdk_GetVideoEncoderChannelParam)LoadSharedLibFun(m_hDll, "JsonSdk_GetVideoEncoderChannelParam");
		}
		if (UserID && GetChannelParam)
		{
			int ret = GetChannelParam(UserID, chid, pInfo, pInfoSize);
			if (ret == 0)
			{
				printf("json sdk  get channel encoder param ok \n");
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
	if (m_hDll && IsInit)
	{
		if (!PreviewFun)
		{
			PreviewFun = (Sdk_Preview)LoadSharedLibFun(m_hDll, "JsonSdk_VideoTranspondStartWithoutViskhead");
		}
		if (UserID && PreviewFun)
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
			if (ret == 0)
			{
				printf("json sdk  preview ok %ld\n", hHandle);
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
	if (m_hDll && IsInit)
	{
		if (!VskPreviewFun)
		{
			VskPreviewFun = (Sdk_VskPreview)LoadSharedLibFun(m_hDll, "JsonSdk_VideoTranspondStart");
		}
		if (UserID && VskPreviewFun)
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
			if (ret == 0)
			{
				printf("json sdk vsk preview ok %ld\n", hHandle);
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
void JsonSdkInterface::StopPreview(JSONLONG rthandle, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!StopPreviewFun)
		{
			StopPreviewFun = (Sdk_StopPreview)LoadSharedLibFun(m_hDll, "JsonSdk_VideoTranspondStop");
		}
		if (rthandle && StopPreviewFun)
		{
			int ret = StopPreviewFun(rthandle);
			if (ret == 0)
			{
				printf("json sdk stop preview ok %ld\n", rthandle);
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
JSONLONG JsonSdkInterface::PlayBack(JSONLONG UserID, int channel, long start, long end, DataPlayCallBack VideoTranCallBack, PlayBackEndCallBack fun, void* pUser, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!PlayBackFun)
		{
			PlayBackFun = (Sdk_PlayBackStartByTime)LoadSharedLibFun(m_hDll, "JsonSdk_PlayBackStartByTimeEx");
		}
		if (UserID && PlayBackFun)
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
void JsonSdkInterface::StopPlayBack(JSONLONG UserID, int& err)
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
void JsonSdkInterface::PlayBackCtrl(JSONLONG UserID, int cmd, int param1, int param2, int& err)
{
	err = 0;
	if (m_hDll && IsInit)
	{
		if (!PlayBackCtrlFun)
		{
			PlayBackCtrlFun = (Sdk_PlayBackCtrl)LoadSharedLibFun(m_hDll, "JsonSdk_PlayBackCtrlEx");
		}
		if (UserID && PlayBackCtrlFun)
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
			JSONLONG hHandle = 0;
			uint64_t  nTotalFileSize = 0;
			int ret = PlayBackCtrlFun(UserID, strJsonReq.c_str());
			if (ret != 0)
			{
				err = ret;
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
	if (m_hDll && IsInit)
	{
		if (!setTimePosFun)
		{
			setTimePosFun = (Sdk_TimePos)LoadSharedLibFun(m_hDll, "JsonSdk_SetTimePlayBackPos");
		}
		if (pbhandle && setTimePosFun)
		{
			uint64_t  nTotalFileSize = 0;
			int ret = setTimePosFun(pbhandle, t);
			if (ret != 0)
			{
				err = ret;
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
	if (m_hDll && IsInit)
	{
		if (!DownloadFun)
		{
			DownloadFun = (Sdk_Download)LoadSharedLibFun(m_hDll, "JsonSdk_DownloadStartByTime");
		}
		if (UserID && DownloadFun)
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
void JsonSdkInterface::StopDownload(JSONLONG UserID, int& err)
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
void JsonSdkInterface::PTZCtrl(JSONLONG UserID, uint32_t Channel, uint32_t PTZCommand, uint32_t Argument, int& err)
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
void JsonSdkInterface::ListIPC(JSONLONG UserID, char* pIPCServerList, uint32_t* pIPCServerListSize, int& err)
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
			try
			{
				int ret = ListIPCFun(UserID, pIPCServerList, pIPCServerListSize);
				if (ret == 0)
				{
					return;
				}
				else
				{
					printf("json sdk  get ipc list err:%d\n", err);
					err = ret;
				}
			}
			catch (std::exception& e)
			{
				std::cout << "Standard exception: " << e.what() << std::endl;
			}
			
			return;
		}
		else
		{
			printf("get JsonSdk_ListIPC err:%d\n", err);
			err = -2;
		}
		return;
	}
	else
	{
		err = -1;
		printf("Load json sdk err:%d\n", err);
	}
	return;
}