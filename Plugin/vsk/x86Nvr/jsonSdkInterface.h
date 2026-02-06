#pragma once
#include <stdint.h>
#include <time.h>
#include <list>

//#include "include/VskClientSdkJson.h"
#include "../../PluginInterface.h"

typedef void (CALLBACK* DataVideoAudioCallBackEx)(unsigned int PlayHandle, uint8_t* pBuffer, unsigned int BufferSize, unsigned int DateType, time_t systime, unsigned int TimeSpace, void* pUser);
typedef void (CALLBACK* DataPlayCallBack)(unsigned int PlayHandle, unsigned int DateType, uint8_t* pBuffer, unsigned int BufferSize, void* pUser);
typedef void (CALLBACK* PlayBackEndCallBack)(unsigned int pbhandle, int errorcode, void* puser);

typedef struct
{
	uint64_t	file_id;
	uint32_t	file_type;
	std::string file_name;
	uint64_t	file_size;
	uint32_t start_time;
	uint32_t end_time;
	uint32_t chid;
	uint32_t stream_type;
	uint32_t disk_no;
	uint32_t partition_no;
	uint32_t	record_type;
	uint32_t status;
	int trace_id;
	std::string face_file_path;
	std::string depkey_file_path;
	std::string identy_id;
	int identy_type;
	uint32_t threshold;
	std::string main_logo;
	std::string sub_logo;
	uint32_t color;
	/* data */
}JsonRecordInfo;

using JSONLONG = unsigned long;
class JsonSdkInterface : public PluginInter
{
public:
	JsonSdkInterface();
	~JsonSdkInterface();

	static JsonSdkInterface& Instance();

	bool SdkIsInit();

	void InitSdk(int &err);
	void SdkClear(int& err);

	JSONLONG JsonSdkLogIn(const char* ip, int port, const char* name, const char* pswd, int& err);
	void JsonSdkLogOut(JSONLONG, int& err);

	void GetChannelEncoderParam(JSONLONG UserID, JSONLONG chid, char* pInfo, uint32_t* pInfoSize, int& err);
	JSONLONG Preview(JSONLONG UserID, int channel, int streamId, DataVideoAudioCallBackEx VideoTranCallBack, void* pUser, int& err);
	JSONLONG VskPreview(JSONLONG UserID, int channel, int streamId, DataPlayCallBack VideoTranCallBack, void* pUser, int& err);
	void StopPreview(JSONLONG, int &err);

	int SearchRecordFileList(JSONLONG UserID, int channel, long start, long end, std::list<JsonRecordInfo> &recordInfos, int &err);

	JSONLONG PlayBack(JSONLONG UserID, int channel, long start, long end, DataPlayCallBack VideoTranCallBack, PlayBackEndCallBack fun, void* pUser, int& err);
	void StopPlayBack(JSONLONG, int& err);
	void PlayBackCtrl(JSONLONG, int cmd, int param1, int param2, int& err);
	void setTimePos(JSONLONG, time_t t, int& err);

	JSONLONG Download(JSONLONG UserID, int channel, long start, long end, DataPlayCallBack VideoTranCallBack, PlayBackEndCallBack fun, void* pUser, int& err);
	void StopDownload(JSONLONG, int& err);

	void PTZCtrl(JSONLONG UserID, uint32_t Channel, uint32_t PTZCommand, uint32_t Argument, int& err);

	void ListIPC(JSONLONG UserID, char *pIPCServerList, uint32_t *pIPCServerListSize, int &err);
	void strRecordInfoToJsonRecordInfo(std::string str, JsonRecordInfo &recordInfo);

	typedef int (CALLBACK* Sdk_Initate)();
	typedef int (CALLBACK* Sdk_InitLog)(char* Path, int ConsoleOut, int FileOut);
	typedef int (CALLBACK* Sdk_Clear)();

	typedef int (CALLBACK* Sdk_Login)(char* pReqClientLogin, JSONLONG* pnLoginID, char* pUserRightConfigBuf, uint32_t* pUserRightConfigBufSize,
										char* pNvrCapabilitiesBuf, uint32_t* pNvrCapabilitiesBufSize);
	typedef int (CALLBACK* Sdk_Logout)(JSONLONG UserID);

	typedef int (CALLBACK* Sdk_GetVideoEncoderChannelParam)(JSONLONG UserID, JSONLONG chid, char* pInfo, uint32_t* pInfoSize);
	typedef int (CALLBACK* Sdk_Preview)(JSONLONG UserID, DataVideoAudioCallBackEx VideoTranCallBack, const char* pTranInfo, 
										uint64_t nUserData, void* pUser, JSONLONG* pnPeviewID);
	typedef int (CALLBACK* Sdk_VskPreview)(JSONLONG UserID, DataPlayCallBack VideoTranCallBack,
		const char* pTranInfo, uint64_t nUserData, void* pUser, JSONLONG* pnPeviewID);
	typedef int (CALLBACK *Sdk_StopPreview)(JSONLONG rthandle);

	typedef int (CALLBACK *Sdk_NewDataLink)(JSONLONG UserID, JSONLONG *pnNewLinkID);
	typedef int (CALLBACK *Sdk_FreeDataLink)(JSONLONG nLinkID);
	typedef int (CALLBACK *Sdk_SearchFile)(JSONLONG UserID, const char *pSearchCond, uint32_t *pnFileCount);
	typedef JSONLONG (CALLBACK* Sdk_SearchNextFile)(JSONLONG UserID, char *pFileInfo, uint32_t *pFileInfoSize);
	typedef int (CALLBACK* Sdk_SearchFileClose)(JSONLONG UserID);

	typedef int (CALLBACK* Sdk_PlayBackStartByTime)(JSONLONG UserID, const char* pInfo, DataPlayCallBack VideoDataCallBack, PlayBackEndCallBack fun,
		uint64_t nUserData, void* pUser, uint64_t* pTotalSize, JSONLONG* pnPlaybackID);
	typedef int (CALLBACK* Sdk_PlayBackStop)(JSONLONG UserID);
	typedef int (CALLBACK* Sdk_PlayBackCtrl)(JSONLONG PlayHandle, const char* pInfo);
	typedef int (CALLBACK* Sdk_TimePos)(JSONLONG PlayHandle, time_t postime);

	typedef int (CALLBACK* Sdk_Download)(JSONLONG userID, const char* pInfo, DataPlayCallBack DownloadDataCallBack, PlayBackEndCallBack fun,
											char* pSavedFileName, void* pUser, int FileType, uint64_t* pTotalSize, JSONLONG* pnDownID);
	typedef int (CALLBACK* Sdk_DownloadStop)(JSONLONG UserID);

	typedef int (CALLBACK* Sdk_PTZCtrl)(JSONLONG UserID, uint32_t Channel, uint32_t PTZCommand, uint32_t Argument);
	typedef int (CALLBACK* Sdk_ListIPC)(JSONLONG UserID, char* pIPCServerList, uint32_t* pIPCServerListSize);
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

	Sdk_GetVideoEncoderChannelParam GetChannelParam;
	Sdk_Preview PreviewFun;
	Sdk_VskPreview VskPreviewFun;
	Sdk_StopPreview StopPreviewFun;

	Sdk_NewDataLink NewDataLinkFun;
	Sdk_FreeDataLink FreeDataLinkFun;
	Sdk_SearchFile SearchFileFun;
	Sdk_SearchNextFile SearchNextFileFun;
	Sdk_SearchFileClose SearchFileCloseFun; 

	Sdk_PlayBackStartByTime PlayBackFun;
	Sdk_PlayBackStop StopPlayBackFun;
	Sdk_PlayBackCtrl PlayBackCtrlFun;
	Sdk_TimePos setTimePosFun;

	Sdk_Download DownloadFun;
	Sdk_DownloadStop StopDownloadFun;

	Sdk_PTZCtrl PtzCtrlFun;

	Sdk_ListIPC ListIPCFun;
};