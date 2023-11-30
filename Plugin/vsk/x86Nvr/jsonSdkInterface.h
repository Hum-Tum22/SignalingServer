#pragma once


#include "include/VskClientSdkJson.h"
#include "../../PluginInterface.h"



class JsonSdkInterface : public PluginInter
{
public:
	//using DWORD = unsigned long;
	JsonSdkInterface();
	~JsonSdkInterface();

	static JsonSdkInterface& Instance();

	bool SdkIsInit();

	void InitSdk(int &err);
	void SdkClear(int& err);

	DWORD LogIn(const char* ip, int port, const char* name, const char* pswd, int& err);
	void LogOut(DWORD, int& err);

	DWORD Preview(DWORD UserID, int channel, int streamId, DataVideoAudioCallBackEx VideoTranCallBack, LPVOID pUser, int& err);
	DWORD VskPreview(DWORD UserID, int channel, int streamId, DataPlayCallBack VideoTranCallBack, LPVOID pUser, int& err);
	void StopPreview(DWORD, int& err);

	DWORD PlayBack(DWORD UserID, int channel, long start, long end, DataPlayCallBack VideoTranCallBack, PlayBackEndCallBack fun, LPVOID pUser, int& err);
	void StopPlayBack(DWORD, int& err);

	DWORD Download(DWORD UserID, int channel, long start, long end, DataPlayCallBack VideoTranCallBack, PlayBackEndCallBack fun, LPVOID pUser, int& err);
	void StopDownload(DWORD, int& err);

	void PTZCtrl(DWORD UserID, uint32_t Channel, uint32_t PTZCommand, uint32_t Argument, int& err);

	void ListIPC(DWORD UserID, char* pIPCServerList, UINT* pIPCServerListSize, int& err);

	typedef int (CALLBACK* Sdk_Initate)();
	typedef int (CALLBACK* Sdk_InitLog)(char* Path, int ConsoleOut, int FileOut);
	typedef int (CALLBACK* Sdk_Clear)();

	typedef int (CALLBACK* Sdk_Login)(char* pReqClientLogin, DWORD* pnLoginID, char* pUserRightConfigBuf, UINT* pUserRightConfigBufSize,
										char* pNvrCapabilitiesBuf, UINT* pNvrCapabilitiesBufSize);
	typedef int (CALLBACK* Sdk_Logout)(DWORD UserID);

	typedef int (CALLBACK* Sdk_Preview)(DWORD UserID, DataVideoAudioCallBackEx VideoTranCallBack, const char* pTranInfo, 
										uint64 nUserData, LPVOID pUser, DWORD* pnPeviewID);
	typedef int (CALLBACK* Sdk_VskPreview)(DWORD UserID, DataPlayCallBack VideoTranCallBack,
		const char* pTranInfo, uint64 nUserData, LPVOID pUser, DWORD* pnPeviewID);
	typedef int (CALLBACK* Sdk_StopPreview)(DWORD UserID);

	typedef int (CALLBACK* Sdk_PlayBackStartByTime)(DWORD UserID, const char* pInfo, DataPlayCallBack VideoDataCallBack, PlayBackEndCallBack fun,
												uint64 nUserData, LPVOID pUser, uint64* pTotalSize, DWORD* pnPlaybackID);
	typedef int (CALLBACK* Sdk_PlayBackStop)(DWORD UserID);

	typedef int (CALLBACK* Sdk_Download)(DWORD userID, const char* pInfo, DataPlayCallBack DownloadDataCallBack, PlayBackEndCallBack fun,
											char* pSavedFileName, LPVOID pUser, INT FileType, uint64* pTotalSize, DWORD* pnDownID);
	typedef int (CALLBACK* Sdk_DownloadStop)(DWORD UserID);

	typedef int (CALLBACK* Sdk_PTZCtrl)(DWORD UserID, UINT Channel, UINT PTZCommand, UINT Argument);
	typedef int (CALLBACK* Sdk_ListIPC)(DWORD UserID, char* pIPCServerList, UINT* pIPCServerListSize);
private:
#ifdef _WIN32
	HMODULE  m_hDll;
#else
	void *m_hDll;
#endif
	bool IsInit;


	Sdk_Initate InitFun;
	Sdk_Clear ClearFun;

	Sdk_Login LoginFun;
	Sdk_Logout LogOutFun;

	Sdk_Preview PreviewFun;
	Sdk_VskPreview VskPreviewFun;
	Sdk_StopPreview StopPreviewFun;

	Sdk_PlayBackStartByTime PlayBackFun;
	Sdk_PlayBackStop StopPlayBackFun;

	Sdk_Download DownloadFun;
	Sdk_DownloadStop StopDownloadFun;

	Sdk_PTZCtrl PtzCtrlFun;

	Sdk_ListIPC ListIPCFun;
};