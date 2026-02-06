#ifndef  VSK_CLIENT_SDK_JSON_H
#define  VSK_CLIENT_SDK_JSON_H

#ifndef __cplusplus
#include <stdbool.h> 
#endif

#include "SdkCommon.h"

/// @brief	初始化 SDK
/// @return 成功返回0,失败返回错误码

/// @note	其他接口调用的前提。
VSK_CSDK int CALL_METHOD JsonSdk_Initate();

/// @brief	初始化日志输出等级
/// @param	ConsoleOut:	控制台输出等级, 0-8
/// @param	FileOut:	文件输出等级, 0-8
VSK_CSDK int CALL_METHOD JsonSdk_InitLog(char *Path, int ConsoleOut, int FileOut);

/// @brief	初始化SDK操作请求时间, 超过该时间返回超时
/// @param	nTimeOut:	超时时间, 单位毫秒
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetReqTimeOut(DWORD nTimeOut);

/// @brief	释放 SDK 资源
/// @return 成功返回0,失败返回错误码
/// @note	资源释放后, 不能再次调用除了初始化之外的接口函数
VSK_CSDK int CALL_METHOD JsonSdk_Clear();

/// @brief	设置异常消息回调函数
/// @param	nMessage:	消息码EXCEPTION_TYPE 
/// @param	hWnd:		接收异常信息消息的窗口句柄
/// @param	exceptionCallBack:	接收异常消息的回调函数，回调当前异常的相关信息
/// @param	pUser:		用户数据
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ExceptionCallBack(UINT nMessage, uint64 nUserData,
	ExceptionCallBack exceptionCallBack, void *pUser);

/// @brief	获取最后操作的错误码描述信息
/// @param	ErrorNo:	错误码
/// @param	ErrorInfo:	错误码描述信息
/// @param	BufLen:		错误码描述地址最大长度
VSK_CSDK void CALL_METHOD JsonSdk_GetErrMsg(DWORD ErrorNo,CHAR *ErrorInfo, UINT BufLen);

/// @brief	启动监听==>修改为设置报警回调函数
/// @param	DataCallBack:	报警信息回调函数，处理回调出来的报警信息
/// @param	pUser:			用户数据
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ListenStart(AlarmMsgCallBack DataCallBack, void *pUser);

/// @brief	启动发送Nvr调试日志
/// @param	UserID:		用户 ID，JsonSdk_Login函数的返回值。
/// @param	DataCallBack:	回调函数，处理回调出来的Nvr调试日志
/// @param	pUser:			用户数据
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_StartSendDebugLog(DWORD UserID,
	NvrDebugLogCallBack DataCallBack, void *pUser);

/// @brief	停止发送Nvr调试日志
/// @param	UserID:		用户 ID，JsonSdk_Login函数的返回值。
/// @param	DataCallBack:	回调函数，处理回调出来的Nvr调试日志
/// @param	pUser:			用户数据
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_StopSendDebugLog(DWORD UserID);

/// @brief	开启接受 一台NVR的报警
/// @param	UserID:		用户 ID，JsonSdk_Login函数的返回值。
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ListenStartOneNvr(DWORD UserID);

/// @brief	关闭接受 一台NVR的报警
/// @param	UserID:		用户 ID，JsonSdk_Login函数的返回值。
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ListenStopOneNvr(DWORD UserID);

/// @brief	关闭监听
/// @param	UserID:		用户 ID，JsonSdk_Login函数的返回值。
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ListenStop();

/// @brief	用户登录
/// @param	pReqClientLogin:	登录Json串, Json: ReqClientLogin
/// @retval	pUserRightConfigBuf:	获取用户的权限信息, Json: UserRightConfig
/// @retval	pnLoginID:				用户登录id
/// @retval	pUserRightConfigBufSize:	用户权限缓冲区大小, 空间不够时, 不返回权限信息
/// @retval	pNvrCapabilitiesBuf:	获取Nvr能力信息, Json: NvrCapabilities
/// @retval	pNvrCapabilitiesBufSize:	缓冲区大小, 空间不够时, 不返回权限信息
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_Login(char* pReqClientLogin, DWORD* pnLoginID,
	char* pUserRightConfigBuf, UINT* pUserRightConfigBufSize,
	char* pNvrCapabilitiesBuf, UINT* pNvrCapabilitiesBufSize);

/// @brief	用户注销
/// @param	UserID:	用户 ID，JsonSdk_Login函数的返回值。
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_Logout(DWORD UserID);

/// @brief	启动语音对讲
/// @param	UserID:		用户 ID，JsonSdk_Login函数的返回值。
/// @param	Channel:	对讲的通道，目前保留
/// @param	CBEncDataType:		是否对音频进行编码，目前保留，采用G711编码
/// @param	VoiceDataCallBack:	音频数据回调函数
/// @param	pUser:		用户信息
/// @retval	pnTalkID:	返回对讲ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_TalkStart(DWORD UserID, DWORD Channel,
	BOOL CBEncDateType, DataTalkCallBack VoiceDataCallBack, void *pUser, DWORD* pnTalkID);

/// @brief	设置语音对讲客户端的音量
/// @param	VoiceDataHandle:	JsonSdk_TalkStart返回的 对讲ID
/// @param	Volume:	音量大小
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetVolume(DWORD VoiceDataHandle,WORD Volume);

/// @brief	停止语音对讲
/// @param	VoiceDataHandle:	JsonSdk_TalkStart返回的 对讲ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_TalkStop(DWORD VoiceDataHandle);

/// @brief	获取文件数量
/// @param	UserID:	登录返回的ID
/// @param	pSearchCond: 欲查找文件的json串;FileSearchCondtion欲查找文件的信息结构;
/// @retval	pnFileCount: 返回搜索文件总数量
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_QueryFileCount(DWORD UserID, const char* pSearchCond, UINT *pnFileCount);

/// @brief	判断指定时间段的文件是否存在
/// @param	UserID:	登录返回的ID
/// @param	pSearchCond: 欲查找文件的json串;FileSearchCondtion欲查找文件的信息结构;
/// @retval	pnFileExisted: 0: 没有文件, 1:有文件
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SearchFileExisted(DWORD UserID, const char* pSearchCond, UINT *pnFileExisted);

/// @brief	录像文件检索 
/// @param	UserID:	登录返回的ID
/// @param	pSearchCond: 欲查找文件的json串;FileSearchCondtion欲查找文件的信息结构;
/// @retval	pnFileCount: 返回搜索文件数量
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SearchFile(DWORD UserID, const char* pSearchCond, UINT *pnFileCount);

/// @brief	获取录像文件信息 
/// @param	UserID:	登录返回的ID
/// @retval	pFileInfo: 文件的信息结构；返回文件信息FileInfo的json串；空间不够时，不返回
/// @retval	pFileInfoSize:	返回文件信息大小(不含结尾字符'\0')，空间不够时返回实际占用大小(含结尾字符'\0'), 输入不能为NULL.
/// @return	0 表示成功，其他值为当前获取状态信息
/// @note	参数修改为登录用户id, 不再是搜索id!
VSK_CSDK DWORD CALL_METHOD JsonSdk_SearchNextFile(DWORD UserID, char* pFileInfo, UINT *pFileInfoSize);

/// @brief	关闭文件检索
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
/// @note	用于清空没有获取完毕的文件信息
VSK_CSDK int CALL_METHOD JsonSdk_SearchFileClose(DWORD UserID);

///PTRANSPOND_INFO
/// @brief	开启预览
/// @param	UserID:		登录返回的ID
/// @param	VideoTranCallBack: 数据回调
/// @param	pTranInfo:	ReqStartPreviewStream结构的json串
/// @param	nUserData:	用户数据, 暂时没有使用
/// @param	pUser:		调用参数
/// @retval	pnPeviewID:	预览句柄ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_VideoTranspondStart(DWORD UserID, DataPlayCallBack VideoTranCallBack,
	const char *pTranInfo, uint64 nUserData, LPVOID pUser, DWORD* pnPeviewID);
VSK_CSDK int CALL_METHOD JsonSdk_VideoTranspondStartWithoutViskhead(DWORD UserID,
	DataVideoAudioCallBackEx VideoTranCallBack, const char *pTranInfo, uint64 nUserData, LPVOID pUser, DWORD* pnPeviewID);

/// @brief	关闭预览视频
/// @param	TranHandle:	开启预览返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int  CALL_METHOD JsonSdk_VideoTranspondStop(DWORD TranHandle);

/// @brief	录像回放	根据文件信息回放
/// @param	UserID:	用户 ID
/// @param	pInfo:	ReqStartPlaybackStreamFile格式的json串
/// @param	DataPlayCallBack: 视频回放的回调函数
/// @param	fun:	回放结束回调函数
/// @param	hWnd:		回放视频的句柄，暂时保留
/// @param	pUser:		用户信息 调用参数
/// @retval	pnPlaybackID:	回放句柄ID, 0表示失败
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_PlayBackStartByFile(DWORD UserID, const char* pInfo,
	DataPlayCallBack VideoDataCallBack, PlayBackEndCallBack fun, uint64 nUserData, LPVOID pUser, DWORD* pnPlaybackID);

/// @brief	设置文件的回放进度
/// @param	PlayHandle:	JsonSdk_PlayBackStartByFile的返回值。
/// @param	nFileOffset:	文件偏移量
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetFilePlayBackPos(DWORD PlayHandle, uint64 nFileOffset);

/// @brief	录像回放	按时间回放
/// @param	UserID:	用户 ID
/// @param	pInfo:	ReqStartPlaybackStreamTime格式的json串
/// @param	DataPlayCallBack: 视频回放的回调函数
/// @param	fun:	回放结束回调函数
/// @param	hWnd:		回放视频的句柄，暂时保留
/// @param	pUser:		用户信息 调用参数
/// @param	pTotalSize:	返回按时间回放视频的总大小.  pInfo存在get_file_size>0时才返回文件大小
/// @retval	pnPlaybackID:	回放句柄ID, 0表示失败
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_PlayBackStartByTime(DWORD UserID, const char* pInfo, DataPlayCallBack VideoDataCallBack,
	PlayBackEndCallBack fun, uint64 nUserData, LPVOID pUser, uint64* pTotalSize, DWORD* pnPlaybackID);

/// @brief	设置时间回放的 回放位置
/// @param	PlayHandle:	JsonSdk_PlayBackStartByTime的返回值，播放句柄。
/// @param	postime:	时间位置
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetTimePlayBackPos(DWORD PlayHandle ,time_t postime);

/// @brief	获取回放的位置, 已经下载的字节数
/// @param	PlayHandle:	JsonSdk_PlayBackStartByTime的返回值，播放句柄。
VSK_CSDK uint64 CALL_METHOD JsonSdk_PlayBackGetPos(DWORD pbHandle);

/// @brief	停止回放录像
/// @param	PlayHandle:	JsonSdk_PlayBackStartByName的返回值。
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_PlayBackStop(DWORD PlayHandle);

/// @brief	回放控制 
/// @param	PlayHandle:	JsonSdk_PlayBackStartByName返回值。
/// @param	pInfo:	ReqControlMsg消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_PlayBackCtrl(DWORD PlayHandle, const char* pInfo);

/// @brief	录像下载	根据文件信息下载
/// @param	UserID:	用户 ID
/// @param	pInfo:	ReqStartPlaybackStreamFile格式的json串
/// @param	DataPlayCallBack: 视频下载的回调函数
/// @param	fun:	下载结束回调函数
/// @param	pSavedFileName:	保存文件的名称，可以为空
/// @param	pUser:		用户信息 调用参数
/// @param	FileType:	0 表示 visking 私有数据头， 1：PS格式
/// @retval	pnDownID:	下载句柄ID, 0 表示失败，其他值作为JsonSdk_DownloadStop的句柄参数。
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DownloadStart(DWORD userID, const char* pInfo, 
	DataPlayCallBack DownloadDataCallBack, PlayBackEndCallBack fun, 
	char * pSavedFileName, LPVOID pUser, INT FileType, DWORD* pnDownID);

/// @brief	录像下载	根据时间段下载
/// @param	UserID:	用户 ID
/// @param	pInfo:	ReqStartPlaybackStreamTime格式的json串
/// @param	DataPlayCallBack: 视频下载的回调函数
/// @param	fun:	下载结束回调函数
/// @param	pSavedFileName:	保存文件的名称，可以为空
/// @param	pUser:		用户信息 调用参数
/// @param	FileType:	0 表示 visking 私有数据头， 1：PS格式
/// @param	pTotalSize:	返回按时间回放视频的总大小.
/// @retval	pnDownID:	下载句柄ID, 0 表示失败，其他值作为JsonSdk_DownloadStop的句柄参数。
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DownloadStartByTime(DWORD userID, const char* pInfo,
	DataPlayCallBack DownloadDataCallBack, PlayBackEndCallBack fun,
	char * pSavedFileName, LPVOID pUser, INT FileType, uint64* pTotalSize, DWORD* pnDownID);

/// @brief	录像下载的控制
/// @param	PlayHandle:	JsonSdk_PlayBackStartByName返回值。
/// @param	pInfo:	ReqFileTransferControl消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DownloadCtrl(DWORD PlayHandle, const char* pInfo);

/// @brief	停止下载录像文件
/// @param	PlayHandle:	JsonSdk_PlayBackStartByName返回值。
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DownloadStop(DWORD DownloadHandle);

/// @brief	获取当前下载录像文件的进度
/// @param	DownloadHandle:	JsonSdk_DownloadStart的返回值
/// @return 成功返回0,失败返回错误码
VSK_CSDK uint64 CALL_METHOD JsonSdk_DownloadGetPos(DWORD DownloadHandle);

/// @brief		锁定录像文件
/// @param		UserID:		登录返回的ID
/// @param		pFileInfo:	要锁定的录像文件信息的json串 sdk没有更新时, 结构为FileInfo, 新的sdk该结构为ReqStartPlaybackStreamFile的前3个字段
/// @return		成功返回TRUE,失败返回FALSE
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_FileLock(DWORD UserID, const char* pFileInfo);

/// @brief		解锁文件信息
/// @param		UserID:		登录返回的ID
/// @param		pFileInfo:	要锁定的录像文件信息的json串 FileInfo(老), ReqStartPlaybackStreamFile(新)
/// @return		成功返回TRUE,失败返回FALSE
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_FileUnlock(DWORD UserID, const char* pFileInfo);

/// @brief		远程开启手动录像
/// @param		Channel:	IPC 通道
/// @param		RecordType:	录像类型
/// @return		成功返回TRUE,失败返回FALSE
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RecordStart(DWORD UserID, UINT Channel, UINT  RecordType);

/// @brief		远程关闭手动录像
/// @param		Channel:	IPC 通道
/// @return		成功返回TRUE,失败返回FALSE
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RecordStop(DWORD UserID, UINT Channel);

/// @brief		获取所有通道的录像状态 NvrRecordState
/// @retval		pInfo:	返回NvrRecordState消息json串,空间不足时无返回
/// @retval pInfoSize:	返回消息json串的大小
/// @return		成功返回TRUE,失败返回FALSE
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetAllRecordState(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief	导出配置文件
/// @param	UserID:		登录返回的ID
/// @param	OutSize:	配置内容缓冲区长度
/// @retval OutBuffer:	返回导出的配置内容地址
/// @retval ReturnSize:	返回实际导出的配置内容长度
/// @return 成功返回0,失败返回错误码
/// @note	OutSize < ReturnSize 或者 OutBuffer为NULL时, 返回FALSE, ReturnSize为实际配置长度
/// @note	返回的数据为所有配置的json串, 缓冲区要设置的足够大.
VSK_CSDK int CALL_METHOD JsonSdk_ConfigExport(DWORD UserID,char * OutBuffer, DWORD OutSize,
                                                   DWORD *ReturnSize);

/// @brief	导入配置文件  NvrConfigList
/// @param	UserID:		登录返回的ID
/// @param	InBuffer:	配置内容地址
/// @return	0 表示失败， 1 表示成功， 2表示需要重启
VSK_CSDK INT CALL_METHOD JsonSdk_ConfigImport(DWORD UserID, const char *InBuffser);

/// @brief	恢复出厂设置
/// @param	UserID:		登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreAllConfig(DWORD UserID);

/// @brief	远程校时
/// @param	UserID:		登录返回的ID
/// @param	nUtcTime:	系统时间
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNVRTime(DWORD UserID, UINT nUtcTime);

/// @brief	获取NVR设备时间
/// @param	UserID:		登录返回的ID
/// @param	psystime:	系统时间结构体
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNVRTime(DWORD UserID,UINT* psystime);

/// @brief	更改NVR监听端口
/// @param	UserID:	登录返回的ID
/// @param	Port:	端口号
/// @return 成功返回0,失败返回错误码
 VSK_CSDK int CALL_METHOD JsonSdk_ModifyListenPort(DWORD UserID,WORD Port);
 
/// @brief		重启设备	 
/// @param		UserID:		登录返回的ID
/// @return		成功返回TRUE,失败返回FALSE
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_Reboot(DWORD UserID);

/// @brief	关闭设备
/// @param	UserID:		登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ShutDown(DWORD UserID);

/// @brief 获取升级文件信息
/// @param	pFilePathName:	升级文件路径
/// @retval	pInfo:			升级文件中的json信息
/// @retval	pInfoSize：		json信息大小，空间不足时返回实际大小
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetUpgradeFileInfo(char* pFilePathName, char *pInfo, UINT *pInfoSize);

/// @brief 升级文件上传
/// @param	UserID:		登录返回的ID
/// @param	pFilePathName:	升级文件包的绝对路径
/// @retval retval	上传文件ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_UploadUpgradeFile(DWORD UserID, char* pFilePathName, DWORD* pnUploadID);

/// @brief 结束升级文件上传, 用于清理上传信息, 如果已经没有上传完成, 则结束上传过程
/// @param	nUploadFileID:	上传文件ID
/// @return 成功返回0,失败返回错误码
/// @note 文件上传失败时, 会删除已经上传的文件
VSK_CSDK int CALL_METHOD JsonSdk_StopUploadFile(DWORD nUploadFileID);

/// @brief 获取上传文件进度和状态
/// @param	nUploadFileID:	上传文件ID
/// @retval	pStatus:		上传状态: FILE_TRANSFER_STATUS
/// @retval	pPercent:		进度: 有效范围[0,100]
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetUploadFileStatus(DWORD nUploadFileID, int* pStatus, int* pPercent);

/// @brief 执行远程升级, 需要首先上传升级文件
/// @param	UserID:		登录返回的ID
/// @retval	pnUpgradeID 升级操作ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_UpgradeStart(DWORD UserID, DWORD* pnUpgradeID);

/// @brief 获取远程升级进度和状态
/// @param	nUpgradeHandle:	升级操作ID
/// @retval	pStatus:		升级状态: ENUM_NVR_UPGRADE_STATE
/// @retval	pPercent:		进度: 有效范围[0,100]
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetUpgradeStatus(DWORD nUpgradeHandle, int* pStatus, int* pPercent);

/// @brief 停止升级，或完成升级
/// @param	nUpgradeHandle:	升级操作ID
/// @return 成功返回0,失败返回错误码
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_UpgradeStop(DWORD nUpgradeHandle);

/// @brief 获取nvr备份版本列表
/// @param	pIP:		nvr所在ip
/// @retval	pInfo:		获取的nvr版本列表信息，空间不足时不返回NvrBackupVersionList
/// @retval	pInfoSize：	返回信息大小，空间不足时返回实际大小
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrBackupList(char*pIP, char *pInfo, UINT *pInfoSize);

/// @brief 恢复nvr备份版本
/// @param	pIP:		nvr所在ip
/// @retval	strVersion:	要恢复的nvr版本字符串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RecoverNvrBackup(char*pIP, char *strVersion);

/// @brief 删除nvr备份版本
/// @param	pIP:		nvr所在ip
/// @retval	strVersion:	要删除的nvr版本字符串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DeleteNvrBackup(char*pIP, char *strVersion);

 /// @brief		获取磁盘信息 NvrDiskList
 /// @param		UserID:		登录返回的ID
 /// @retval	pDiskInfo:	获取的磁盘信息，空间不足时不返回
 /// @retval	pDiskInfoSize：返回磁盘信息大小，空间不足时返回实际大小
 /// @return 成功返回0,失败返回错误码
 VSK_CSDK int CALL_METHOD JsonSdk_GetDiskInfo(DWORD UserID, char *pDiskInfo, UINT *pDiskInfoSize);

  /// @brief		硬盘加锁
  /// @param		UserID:		登录返回的ID
  /// @param		diskno:		磁盘号
  /// @return 成功返回0,失败返回错误码
 VSK_CSDK int CALL_METHOD JsonSdk_LockDisk(DWORD UserID, int diskno);

 /// @brief		硬盘解锁
 /// @param		UserID:		登录返回的ID
 /// @param		diskIndex:	磁盘序列
 /// @return 成功返回0,失败返回错误码
 VSK_CSDK int CALL_METHOD JsonSdk_UnlockDisk(DWORD UserID, int diskno);

 /// @brief	格式化硬盘及分区
 /// @param	UserID:		登录返回的ID
 /// @param	DiskNo:		硬盘号
 /// @return 成功返回0,失败返回错误码
 VSK_CSDK int CALL_METHOD JsonSdk_FormatDiskByDiskNo(DWORD UserID, int DiskNo);

 /// @brief	获取硬盘格式化进度
 /// @param	UserID:		登录返回的ID
 /// @param	DiskNo:		硬盘号
 /// @retval	precent:	硬盘格式化进度
 /// @return 成功返回0,失败返回错误码 
 VSK_CSDK int CALL_METHOD JsonSdk_GetFormatDiskPrecentByDiskNo(DWORD UserID, int DiskNo, int *precent);

 /// @brief 查询系统日志
 /// @param	UserID:	登录返回的ID
 /// @param	pInfo:	ReqGetNvrSyslogList消息json串
 /// @retval pnLogCount:	获取日志数量
 /// @return 成功返回0,失败返回错误码
 VSK_CSDK int CALL_METHOD JsonSdk_QueryNVRLog(DWORD UserID, const char* pInfo, UINT *pnLogCount);
 
 /// @brief 获取下一条日志记录 NvrSysLog
 /// @param		UserID:		登录返回的ID
 /// @retval	pNVRLog:	返回的日志记录NvrSysLog，空间不足时不返回，
 /// @retval	pNVRLogSize：返回日志记录大小，空间不足时返回实际大小
 /// @return	当前日志的状态, 0: 返回的日志有效, 其它为错误码
 VSK_CSDK DWORD CALL_METHOD JsonSdk_QueryNextLog(DWORD UserID, char* pNVRLog, UINT *pNVRLogSize);

/// @brief	关闭日志查询
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_QueryLogClose(DWORD UserID);

/// @brief 查询报警日志
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	AlarmLogSearchCondtion消息json串
/// @retval pnLogCount:	获取日志数量
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_QueryAlarmLog(DWORD UserID, const char* pInfo, UINT *pnLogCount);

/// @brief 获取下一条日志记录 NvrAlarmLog
/// @param		UserID:		登录返回的ID
/// @retval	pLog:	返回的NvrAlarmLog类型的json信息，空间不足时不返回，
/// @retval	pLogSize：返回日志记录大小，空间不足时返回实际大小
/// @return	当前日志的状态, 0: 返回的日志有效, 其它为错误码
VSK_CSDK DWORD CALL_METHOD JsonSdk_QueryNextAlarmLog(DWORD UserID, char* pLog, UINT *pLogSize);

/// @brief	关闭报警日志查询
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_QueryAlarmLogClose(DWORD UserID);

/// @brief	列出所有IPC  NvrMsg::NvrServerUriList
/// @param	UserID:			登录返回的ID
/// @retval	pIPCServerListSize:	返回搜索结果的大小，空间不足时返回，实际结果的大小
/// @retval pIPCServerList:	返回搜索结果，空间不足时不返回
/// @return 成功返回0,失败返回错误码
/// @note	包含已经添加的和没有添加的, 以及已经添加不在线的结果, 
VSK_CSDK int CALL_METHOD JsonSdk_ListIPC(DWORD UserID, char* pIPCServerList, UINT *pIPCServerListSize);

/// @brief	搜索在线的IPC
/// @param	UserID:			登录返回的ID
/// @retval	pIPCServerListSize:	返回搜索结果大小，空间不足时返回实际大小
/// @retval pIPCServerList:	返回搜索结果json串，空间不足时不返回
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SearchIPC(DWORD UserID, char* pIPCServerList, UINT *pIPCServerListSize);

/// @brief	修改 IPC
/// @param	UserID:		登录返回的ID
/// @retval	pIPCServer:	IPC的信息和账号信息ReqSetServerUriInfo
/// @return 成功返回0,失败返回错误码
/// @note	该接口只能修改一个ipc参数, 多个ipc参数同时修改使用接口: JsonSdk_ModifyIPCList
VSK_CSDK int CALL_METHOD JsonSdk_ModifyIPC(DWORD UserID, const char* pIPCServer);

/// @brief	删除IPC
/// @param	UserID:		登录返回的ID
/// @param	Channel:	IPC所在的通道
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RemoveIPC(DWORD UserID,INT Channel);

/// @brief	添加 IPC
/// @param	UserID:		登录返回的ID
/// @retval	pIPCServer:	IpcServerUri信息
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_AddIPC(DWORD UserID, const char* pIPCServer);

/// @brief	用来批量设置摄像头
/// @param	UserID:				登录返回的ID
/// @param	pIPCServerArray:	IPC的信息数组;IpcServerUri数组
/// @param	ArraySize:			IPC的信息数组大小
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetDecIPC(DWORD UserID, char *pIPCServerArray[], UINT ArraySize);

/// @brief	获取所有通道码流码率信息
/// @param	UserID:			登录返回的ID
/// @retval	pIPCList:		返回所有通道码流码率信息: ChannelStreamInfo
/// @retval pIPCListSize:	输入: pIPCList 缓冲区大小, 输出: 实际大小
/// @return 成功返回0,失败返回错误码

/// @note	输出值 >= 缓冲区大小时, 缓冲区数据无效 需要增大缓冲区.
//VSK_CSDK int CALL_METHOD JsonSdk_GetChannelStreamInfo(DWORD UserID, char* pIPCList, UINT *pIPCListSize);

/// @brief	搜索 网络在线用户 NvrOnlineUserInfoList
/// @param	UserID:				登录返回的ID
/// @retval	pOnlineUserListSize:	返回在线用户列表信息大小，空间不足时返回实际大小
/// @retval	pOnlineUserList:		在线用户信息列表NvrOnlineUserInfoList,空间不足时不返回
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SearchOnlineUser(DWORD UserID, char *pOnlineUserList, UINT *pOnlineUserListSize);

/// @brief	断开指定网络用户
/// @param	UserID:				登录返回的ID
/// @param	OnlineUserID:		在线用户的 ID号。
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ClearOnlineUser(DWORD UserID, UINT OnlineUserID);

/// @brief	获取远程控制画面分割,以及切换时间
/// @param	UserID:		登录返回的ID
/// @retval	pInfo:	返回NvrPreviewSplit消息json串,空间不足时无返回
/// @retval pInfoSize:	返回消息json串的大小
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetPreviewSplit(DWORD UserID, char* pInfo, DWORD *pInfoSize);

/// @brief	远程控制画面分割,以及切换时间
/// @param	UserID:		登录返回的ID
/// @param	pInfo:		NvrPreviewSplit消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetPreviewSplit(DWORD UserID, const char* pInfo);

/// @brief	恢复画面分割
/// @param	UserID:		登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int	CALL_METHOD JsonSdk_RestorePreviewSplit(DWORD UserID);

/// @brief	请求配置VMS协议 媒体数据传输接口
/// @param	UserID:		登录返回的ID
/// @param	pInfo:		NvrPreviewSplit消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetIpcStreamTransPort(DWORD UserID, const char* pInfo);

/// @brief	请求清除所有 媒体数据传输接口 配置
/// @param	UserID:		登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ClearIpcStreamTransPort(DWORD UserID);

/// @brief	远程云台控制
/// @param	UserID:		登录返回的ID
/// @param	Channel:	通道号,从0开始
/// @param	PTZCommand:	ptz命令:UNS_PTZCONTROL_COMMAND_E
/// @param	Argument:	ptz命令对应的参数
/// @retval	pInfo:		ReqPtzControl消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_PtzControl(DWORD UserID, const char* pInfo);
VSK_CSDK int CALL_METHOD JsonSdk_PTZCtrl(DWORD UserID, UINT Channel, UINT PTZCommand, UINT Argument);

/// @brief	远程云台3D控制
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	Ptz3DZoomParam消息json串 
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_PTZ3DRoomCtrl(DWORD UserID, const char *pInfo);

/// @brief	对指定通道的巡航路径开始巡航
/// @param	UserID:		登录返回的ID
/// @param	Channel:	指定通道ID, 从0开始
/// @param	Index:		返回巡航路径
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_PTZCruiseStart(DWORD UserID, UINT Channel, UINT Index);

/// @brief	对通道停止巡航
/// @param	UserID:		登录返回的ID
/// @param	Channel:	指定通道ID, 从0开始
/// @return 成功返回0,失败返回错误码
VSK_CSDK int	CALL_METHOD JsonSdk_PTZCruiseStop(DWORD UserID,UINT Channel);

/// @brief	获取指定通道的巡航路径 PtzCruiseParam
/// @param	UserID:		登录返回的ID
/// @param	Channel:	指定通道ID, 从0开始
/// @retval	pCruisePara:返回数据，空间不足时不返回
/// @retval	pCruiseParaSize:返回数据大小，空间不足时返回实际大小
/// @return 成功返回0,失败返回错误码
VSK_CSDK int	CALL_METHOD JsonSdk_GetCruiseParam(DWORD UserID, UINT Channel, char *pCruisePara, UINT *pCruiseParaSize);

/// @brief	设置指定通道的巡航路径 ReqSetPtzCruiseInfo
/// @param	UserID:		登录返回的ID
/// @param	pCruisePara:数据json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK  int	 CALL_METHOD JsonSdk_SetCruiseParam(DWORD UserID, const char *pCruisePara);

/// @brief	获取指Nvr能力集
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回NvrCapabilities消息json串，空间不足时无返回
/// @retval pInfoSize:	返回消息json串的大小
/// @return 成功返回0,失败返回错误码
VSK_CSDK int	CALL_METHOD JsonSdk_GetNvrCapabilities(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief	开始搜索在线NVR设备
/// @param	searchcbfun:	搜索到设备后回调
/// @param	pUser:		用户参数
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_StartSearchNvr(SearchNvrCallbackEx searchcbfun, void *pUser);

/// @brief	停止搜索在线NVR设备
/// @param	UserID:		登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_StopSearchNvr();

//导出通道配置文件 不再支持
VSK_CSDK int CALL_METHOD JsonSdk_ChannelConfigExport(DWORD UserID,char * OutBuffer, DWORD OutSize,DWORD *ReturnSize);


//导入通道配置文件 不再支持
VSK_CSDK INT CALL_METHOD JsonSdk_ChannelConfigImport(DWORD UserID, char *InBuffser,DWORD InBufferSize);

/// @brief	获取NvrModel配置 NvrModelInfo
/// @param	UserID:		登录返回的ID
/// @retval	modelinfo:	返回NvrModelInfo消息Json串，空间不足时无返回
/// @retval	pModelInfoSize：返回消息大小 空间不足时返回实际消息大小
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrModleInfo(DWORD UserID, char *pModelInfo, UINT *pModelInfoSize);

/// @brief	设置NvrModel配置
/// @param	UserID:		登录返回的ID
/// @param	modelinfo:	ReqSetNvrModelInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrModleInfo(DWORD UserID, const char *pModelinfo);

//----------------------------------- 细化配置接口---------------------------------------
/// @brief	获取系统信息配置
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回的NvrSysInfo消息,空间不足时无返回 
/// @retval pInfoSize:	返回消息json串的大小
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrSysConfig(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief	获取版本信息
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回的NvrVersionInfo消息json串，空间不足时无返回 
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrVersionConfig(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief	获取ipc时间同步及侦测信息 NvrFuncState
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	FALSE:获取当前配置, TRUE: 获取默认配置
/// @retval	pInfo:	返回的消息json串，空间不足时无返回 
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrFuncState(DWORD UserID, char* pInfo, DWORD *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置ipc时间同步及侦测信息NvrFuncState
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrFuncState(DWORD UserID, const char* pInfo);

/// @brief	获取基本信息 NvrCommonParam
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	FALSE:获取当前配置, TRUE: 获取默认配置
/// @retval	pInfo:	返回的消息json串，空间不足时无返回 
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrCommonConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置基本信息  ReqSetNvrCommonInfo
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrCommonConfig(DWORD UserID, const char* pInfo);

/// @brief	Nvr基本信息恢复默认配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreNvrCommonConfig(DWORD UserID);

/// @brief	获取Osd通道参数 VideoChannelParam
/// @param	UserID:	登录返回的ID
/// @param	chid:	通道ID, 从0开始
/// @retval	pInfo:	返回的消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetVideoChannelParam(DWORD UserID, DWORD chid, char* pInfo, UINT *pInfoSize);

/// @brief	设置Osd通道参数
/// @param	UserID:	登录返回的ID
/// @param	chid:	通道ID, 从0开始
/// @param	pInfo:	消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetVideoChannelParam(DWORD UserID, const char* pInfo);

/// @brief	获取预览输出通道参数 NvrPreviewParam
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	FALSE:获取当前配置, TRUE: 获取默认配置
/// @retval	pInfo:	返回的消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrPreviewParam(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置预览输出通道参数 ReqSetPreviewInfo
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrPreviewParam(DWORD UserID, const char* pInfo);

/// @brief	预览输出通道参数恢复默认配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreNvrPreviewParam(DWORD UserID);

/// @brief	获取编码参数 VideoEncoderChannelParam
/// @param	UserID:	登录返回的ID
/// @param	chid:	通道ID, 从0开始
/// @retval	pInfo:	返回的消息json串，空间不足时无返回 
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetVideoEncoderChannelParam(DWORD UserID, DWORD chid, char* pInfo, UINT *pInfoSize);

/// @brief	设置编码参数（子码流or主码流）
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetVideoEncoderInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetVideoEncoderChannelParam(DWORD UserID, const char* pInfo);

/// @brief	获取视频检测参数
/// @param	UserID:	登录返回的ID
/// @param	chid:	通道ID, 从0开始
/// @param	nType:	视频检测类型
/// @param	bDefault:	FALSE:获取当前配置, TRUE: 获取默认配置
/// @retval	pInfo:	返回VideoChannelDetectInfo消息,空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetVideoChannelDetectParam(DWORD UserID, DWORD chid, VIDEO_DETECT_TYPE nType, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置视频检测参数
/// @param	UserID:	登录返回的ID
/// @param	nType:	视频检测类型
/// @param	pInfo:	ReqSetVideoChannelDetectInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetVideoChannelDetectParam(DWORD UserID, VIDEO_DETECT_TYPE nType, const char* pInfo);

/// @brief	视频检测参数恢复默认配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreVideoChannelDetectParam(DWORD UserID);

/// @brief	获取告警输入检测参数
/// @param	UserID:	登录返回的ID
/// @param	nAlarmID:	报警输入通道ID, 从0开始
/// @param	bDefault:	false:获取当前配置, true: 获取默认配置
/// @retval	pInfo:	返回NvrAlarmChannelDetectParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetAlarmInDetectParam(DWORD UserID, UINT nAlarmID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置告警输入检测参数
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetAlarmInDetectInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetAlarmInDetectParam(DWORD UserID, const char* pInfo);

/// @brief	获取相机报警参数配置
/// @param	UserID:	登录返回的ID
/// @param	nAlarmID:	报警输入通道ID, 从0开始
/// @retval	pInfo:	返回NvrAlarmChannelDetectParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @param	bDefault:	false:获取当前配置, true: 获取默认配置
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetCameraAlarmDetectParam(DWORD UserID, UINT nAlarmID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置相机报警参数配置
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetAlarmInDetectInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetCameraAlarmDetectParam(DWORD UserID, const char* pInfo);

/// @brief	告警输入检测参数恢复默认配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreAlarmInDetectParam(DWORD UserID);

/// @brief 获取所有报警配置参数
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @return 成功返回0,失败返回错误码
/// @retval	pInfo:	返回NvrAlarmParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrAlarmConfig(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief	获取异常处理参数
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回NvrExceptionDetectParam消息json串，空间不足时无返回 
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @param	bDefault:	FALSE:获取当前配置, TRUE: 获取默认配置
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetExceptionDetectParam(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置异常处理参数
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetExceptionInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetExceptionDetectParam(DWORD UserID, const char* pInfo);

/// @brief	异常处理参数恢复默认配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreExceptionDetectParam(DWORD UserID);

/// @brief	获取系统维护参数
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回NvrSysMaintainParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @param	bDefault:	FALSE:获取当前配置, TRUE: 获取默认配置
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrSysMaintainParam(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置系统维护参数
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNvrSysMaintainParam消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrSysMaintainParam(DWORD UserID, const char* pInfo);

/// @brief	系统维护参数恢复默认配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreNvrSysMaintainParam(DWORD UserID);

/// @brief	获取盘组管理参数
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回DiskGroupConfig消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @param	bDefault:	FALSE:获取当前配置, TRUE: 获取默认配置
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetDiskGroupParam(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	获取通道所属盘组配置
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	RecordChannelDiskGroupConfig消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetDiskGroupChannelParam(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置通道所属盘组配置
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	RecordChannelDiskGroupConfig消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetDiskGroupChannelParam(DWORD UserID, const char* pInfo);

/// @brief	设置硬盘所属盘组配置
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetDiskGroupDisk消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetDiskGroupDiskParam(DWORD UserID, const char* pInfo);

/// @brief	盘组配置恢复默认配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreDiskGroupParam(DWORD UserID);

/// @brief	获取显示模式配置
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回NvrDisplayMode消息json串，空间不足时无返回 
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrDisplayMode(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置显示模式配置
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNvrDisplayMode消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrDisplayMode(DWORD UserID, const char* pInfo);

/// @brief	Nvr网络模块信息恢复默认配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreDefalutAllNetworkInfo(DWORD UserID);

/// @brief	获取dns配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	FALSE:获取当前配置, TRUE: 获取默认配置
/// @return 成功返回0,失败返回错误码
/// @retval	pInfo:	返回NvrDnsParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrNetDnsConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置dns
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNetworkDns消息json串, auto_mode暂不支持, 不设置或者设置为0即可.
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrNetDnsConfig(DWORD UserID, const char* pInfo);

/// @brief 获取network配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	FALSE:获取当前配置, TRUE: 获取默认配置
/// @return 成功返回0,失败返回错误码
/// @retval	pInfo:	返回NvrNetworkList消息json串,空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrNetworkListConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置network
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNetworkList消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrNetworkListConfig(DWORD UserID, const char* pInfo);

/// @brief 获取broadcast配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @return 成功返回0,失败返回错误码
/// @retval	pInfo:	返回NvrBroadcastParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrbroadCastConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置broadcast
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNetworkBroadcast消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrBroadCastConfig(DWORD UserID, const char* pInfo);

/// @brief 获取port配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @return 成功返回0,失败返回错误码
/// @retval	pInfo:	返回NvrNetworkPort消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrPortConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置port
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNetworkPort消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrPortConfig(DWORD UserID, const char* pInfo);

/// @brief 获取ntp配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @return 成功返回0,失败返回错误码
/// @retval	pInfo:	返回NvrNtpParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrNtpConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置ntp
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNetworkNtp消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrNtpConfig(DWORD UserID, const char* pInfo);

/// @brief 获取ddns配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @return 成功返回0,失败返回错误码
/// @retval	pInfo:	返回NvrDdnsParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrDdnsConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置ddns
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNetworkDDNS消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrDdnsConfig(DWORD UserID, const char* pInfo);

/// @brief 获取pppoe配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @return 成功返回0,失败返回错误码
/// @retval	pInfo:	返回NvrPppoeParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrPppoeConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置pppoe
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNetworkPPPOE消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrPppoeConfig(DWORD UserID, const char* pInfo);

/// @brief 获取eamil配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @return 成功返回0,失败返回错误码
/// @retval	pInfo:	返回NvrEmailParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrEmailConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置email
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNetworkEmail消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrEmailConfig(DWORD UserID, const char* pInfo);

/// @brief 获取ftp配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回NvrFtpParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrFtpConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置ftp
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNetworkFtp消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrFtpConfig(DWORD UserID, const char* pInfo);

/// @brief 获取upnp配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @return 成功返回0,失败返回错误码
/// @retval	pInfo:	返回NvrUpnpParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrUpnpConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置upnp
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNetworkUpnp消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrUpnpConfig(DWORD UserID, const char* pInfo);

/// @brief	获取自动录像计划参数配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回NvrRecordScheduleChannelParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrVideoScheduleConfig(DWORD UserID, uint32 chid, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	配置自动录像计划参数
/// @param	UserID:	登录返回的ID
/// @param	chid:	通道id, 从0开始
/// @param	pInfo:	ReqSetRecordScheduleInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrVideoScheduleConfig(DWORD UserID, const char* pInfo);

/// @brief	自动录像计划参数恢复默认配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreVideoScheduleConfig(DWORD UserID);

/// @brief 获取record strategy配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回NvrRecordStrategyParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrRecordStrategyConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置record strategy
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetRecordStrategyInfo消息json串
/// @return 成功返回0,失败返回错误码
/// @todo   可以做单通道保存
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrRecordStrategyConfig(DWORD UserID, const char* pInfo);

/// @brief 获取alarm out配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回NvrAlarmChannelDetectParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrAlarmOutDetectConfig(DWORD UserID, UINT chan, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置alarm out
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetAlarmInDetectInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrAlarmOutDetectConfig(DWORD UserID, const char* pInfo);

/// @brief 获取exception配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回NvrExceptionDetectParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrExceptionDetectConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置exception
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetExceptionInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrExceptionDetectConfig(DWORD UserID, const char* pInfo);

/// @brief	获取报警输出策略 
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	返回NvrAlarmOutStrategyParam消息json串，空间不足时无返回
/// @param pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrAlarmOutStrategy(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/*=false*/);

/// @brief	配置报警输出策略
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	NvrAlarmOutStrategyParam消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrAlarmOutStrategy(DWORD UserID, const char* pInfo);

/// @brief	获取alarm out配置
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回NvrAlarmOutStatusList消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrAlarmOutStatus(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief	获取布防撤防配置
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回NvrDefenceParam消息json串，空间不足时无返回
/// @retval pInfoSize: 输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrDefenceParam(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief	设置布防撤防配置
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetDefenceInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrDefenceParam(DWORD UserID, const char* pInfo);

/// @brief 获取 GB28181 配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回GB28181Param消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrGb28181Param(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置 GB28181
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetGB28181Info消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrGb28181Param(DWORD UserID, const char* pInfo);

/// @brief 获取 alarm center 配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回NvrAlarmCenterParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrAlarmCenterParam(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置 alarm center
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetAlarmCenterInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrAlarmCenterParam(DWORD UserID, const char* pInfo);

/// @brief 获取 phone screen 配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回NvrPhoneParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrPhonescreenParam(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置 phone screen
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	ReqSetPhoneInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrPhonescreenParam(DWORD UserID, const char* pInfo);

/// @brief 获取 Monitor Platfor 联网设备 配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回MonitorParam消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrMonitorParam(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置 Monitor Platfor 联网设备 
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetMonitorInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrMonitorParam(DWORD UserID, const char* pInfo);

/// @brief	恢复 联网设备 配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreNvrMonitorParam(DWORD UserID);

/// @brief 配置验收信息 MonitorChecker
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	MonitorChecker消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetMonitorChecker(DWORD UserID, const char* pInfo);

/// @brief 配置维保信息 MonitorMaintain
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetMonitorMaintain(DWORD UserID, const char* pInfo);

/// @brief 配置报修信息 MonitorRepair
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetMonitorRepair(DWORD UserID, const char* pInfo);

/// @brief 获取视频检测,丢失,遮挡的所有通道配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回VideoChannelDetectConfig消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetVideoChannelDetectConfig(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief 获取 video detect 联网设备 配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回VideoChannelDetectInfo消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrVideodetectParam(DWORD UserID, UINT chn, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置 video detect 联网设备 
/// @param	UserID:	登录返回的ID
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @retval	pInfo:	ReqSetVideoChannelDetectInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrVideodetectParam(DWORD UserID, const char* pInfo);

/// @brief 获取 video loss 联网设备 配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回VideoChannelDetectInfo消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrVideoLossParam(DWORD UserID, UINT chn, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置 video loss 联网设备 
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	ReqSetVideoChannelDetectInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrVideoLossParam(DWORD UserID, const char* pInfo);

/// @brief 获取 video blind 联网设备 配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回VideoChannelDetectInfo消息json串，空间不足时无返回
/// @retval pInfoSize: 输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrVideoBlindParam(DWORD UserID, UINT chn, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置 video blind 联网设备 
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	ReqSetVideoChannelDetectInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrVideoBlindParam(DWORD UserID, const char* pInfo);

/// @brief 获取 hotbackup 联网设备 配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回NvrHotBackupConfig消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrHotBackupConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 配置 hotbackup 联网设备 
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetHotBackupInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrHotBackupConfig(DWORD UserID, char* pInfo);

/// @brief 获取 计数器所有参数配置
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	获取当前配置false   默认true
/// @retval	pInfo:	返回NvrIntCounter消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrIntCounterConfig(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief 获取 单个计数器参数配置
/// @param	UserID:	登录返回的ID
/// @param	nIndex:	计数器索引
/// @retval	pnValue:	返回NvrIntCounter值
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrIntCounterValue(DWORD UserID, int nIndex, UINT *pnValue);

/// @brief 配置 计数器参数配置
/// @param	UserID:	登录返回的ID
/// @param	nIndex:	计数器索引
/// @param	nValue:	计数器要修改的值
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrIntCounterConfig(DWORD UserID, int nIndex, UINT nValue);

/// @brief 获取 NvrUserConfig -- UserBaseConfig 
/// @param	UserID:	登录返回的ID
/// @param	pReqInfo: 请求消息ReqGetUserInfo
/// @retval	pInfo:	返回NvrUserConfig消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
/// @ref	NvrGetUserFileterType
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrUserInfo(DWORD UserID, const char *pReqInfo, char* pInfo, UINT *pInfoSize);

/// @brief 获取 NvrUserConfig 
/// @param	UserID:	登录返回的ID
/// @param	pReqInfo: 请求消息ReqGetUserGroupInfo
/// @retval	pInfo:	返回NvrUserConfig消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
/// @ref	NvrGetUserFileterType
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrUsergroupInfo(DWORD UserID, const char *pReqInfo, char* pInfo, UINT *pInfoSize);

/// @brief 获取 用户权限  AckGetUserPrivilege
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回AckGetUserPrivilege消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
/// @ref	NvrGetUserFileterType
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrUserPrivilege(DWORD UserID,  char* pInfo, UINT *pInfoSize);

/// @brief	请求修改用户信息 
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqModifyUserInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ReqModifyUserInfo(DWORD UserID, const char* pInfo);

/// @brief	请求添加用户信息 
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqAddUserInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_AddUserInfo(DWORD UserID, const char* pInfo);

/// @brief	请求删除用户信息 
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqDeleteUserInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DeleteUserInfo(DWORD UserID, const char* pInfo);

/// @brief	请求修改用户信息  
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqModifyUserGroupInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ReqModifyUserGroupInfo(DWORD UserID, const char* pInfo);

/// @brief	请求添加用户组信息  ReqAddUserGroupBaseConfig
/// @param	UserID:	登录返回的ID
/// @param	pInfo: ReqAddUserGroupBaseConfig消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_AddUserGroupInfo(DWORD UserID, const char* pInfo);

/// @brief	请求删除用户信息 
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqDeleteUserGroupInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DeleteUserGroupInfo(DWORD UserID, const char* pInfo);

/// @brief	请求恢复所有默认用户配置
/// @param	UserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RestoreDefaultUserInfo(DWORD UserID);

/// @brief	获取系统日志
/// @param	UserID:	登录返回的ID
/// @retval	pReqInfo:	ReqGetNvrSyslogList消息json串
/// @retval pInfo:		返回NvrSysLogList空间不足时无返回
/// @retval pInfoSize： 返回NvrSysLogList大小，空间不足时返回NvrSysLogList所需大小
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrSysloglist(DWORD UserID, const char *pReqInfo, char* pInfo, UINT *pInfoSize);

/// @brief	请求修改系统日志
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqModifyUserInfo消息json串
/// @return 成功返回0,失败返回错误码
/// @note 废弃该接口, 不允许修改nvr的日志
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrSyslogInfo(DWORD UserID, const char* pInfo);

/// @brief	获取系统调试信息
/// @param	UserID:	登录返回的ID
/// @param	bDefault:	false:获取当前配置, true: 获取默认配置
/// @retval	pInfo:	返回NvrSystemDebugInfo消息json串，空间不足时无返回
/// @retval pInfoSize:	返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrSysDebugInfo(DWORD UserID, char* pInfo, UINT *pInfoSize, bool bDefault/* = false*/);

/// @brief	设置系统调试信息
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetNvrSysDebugInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetNvrSysDebugInfo(DWORD UserID, const char* pInfo);


/// @brief ipc上传图片到nvr
/// @param	UserID:	登录返回的ID
/// @param	pImageInfo:	ImageInfo消息json串
/// @param	pData:		图片数据
/// @param	nDataLen:	图片数据长度
VSK_CSDK int CALL_METHOD JsonSdk_IPCImageUpload(DWORD UserID, const char* pImageInfo, const uint8* pData, uint32 nDataLen);


/// @brief 申请一个新的连接, 用于文件传输等任务
/// @param	UserID:	登录返回的ID
/// @retval	pnNewLinkID 新的连接ID, 值为0表示失败
VSK_CSDK int CALL_METHOD JsonSdk_NewDataLink(DWORD UserID, DWORD* pnNewLinkID);

/// @brief 释放一个新的连接
/// @param	nLinkID:	连接ID
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_FreeDataLink(DWORD nLinkID);

/// @brief 从Nvr下载小文件
/// @param	nLinkID:	连接ID
/// @param	pFilePathName:	文件在服务器上的路径名
/// @param	ppFileBuf:	文件内容指针的地址
/// @param	pFileBufLen:	文件长度
/// @return 成功返回0,失败返回错误码
/// @note	下载的图片大小不能超过10MB, 删除下载缓存调用JsonSdk_FreeBuffer
VSK_CSDK int CALL_METHOD JsonSdk_DownloadSmallFile(DWORD nLinkID, const char* pFilePathName, unsigned char** ppFileBuf, unsigned * pFileBufLen);

/// @brief 删除下载的文件缓存, 否则会造成内存泄漏!
/// @param	ppFileBuf:	文件内容指针的地址, 执行完成后, 文件内容指针值为NULL
VSK_CSDK int CALL_METHOD JsonSdk_FreeBuffer(unsigned char** ppFileBuf);

/// @brief	获取人脸识别和报警参数配置
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回FacePictureAlarmConfig消息,空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetFacePictureAlarmConfig(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief	获取指定通道人脸识别和报警参数配置
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回FacePictureAlarmInfo消息,空间不足时无返回
/// @retval pInfoSize:	返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetFacePictureAlarmConfigByChinID(DWORD UserID,uint32 nChinID, char* pInfo, UINT *pInfoSize); 

/// @brief	设置人脸识别报警参数配置
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqSetFacePictureAlarmConfig消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetFacePictureAlarmInfo(DWORD UserID, const char* pInfo);

/// @brief	获取人脸识别基本参数配置
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回FacePictureBaseConfig消息,空间不足时无返回
/// @retval pInfoSize:	返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetFacePictureBaseConfig(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief	设置人脸识别基本参数配置
/// @param	UserID:	登录返回的ID
/// @param	chid:	通道ID, 从0开始
/// @param	pInfo:	ReqSetFacePictureBaseConfig消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetFacePictureBaseConfig(DWORD UserID, const char* pInfo);

/// @brief 获取 AI 用户信息列表
/// @param	UserID:	登录返回的ID
/// @param	pReqMsg: 查询消息ReqGetAIUserInfo
/// @retval	pInfo:	返回AIFaceUserList消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetAIUserInfoList(DWORD UserID,const char *pReqMsg, char* pInfo, UINT *pInfoSize);

/// @brief 添加 AI 用户信息 
/// @param	UserID:	登录返回的ID
/// @param	pReqInfo:	请求消息AIFaceUserInfo, user_id填0
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_AddAIUserInfo(DWORD UserID, const char *pReqInfo);

/// @brief 修改 AI 用户信息 
/// @param	UserID:	登录返回的ID
/// @param	pReqInfo:	请求消息AIFaceUserInfo, user_id为有效用户
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ModifyAIUserInfo(DWORD UserID, const char *pReqInfo);

/// @brief 删除 AI 用户信息 
/// @param	UserID:	登录返回的ID
/// @param	pCardID:	用户身份证号
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DelAIUserInfo(DWORD UserID, const char *pCardID);

/// @brief	csv文件导入 AI 用户信息 
/// @param	UserID:	登录返回的ID
/// @param	strImportUserInfo: ReqImportAiUserInfo消息Json串的请求导入的用户信息
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ImportAIUserInfo(DWORD UserID, const char *strImportUserInfo);

/// @brief 导出 AI 用户信息 
/// @param	UserID:	登录返回的ID
/// @param	strFilePath:	导出excel文件路径
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ExportAIUserInfo(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief 删除 AI 用户图片信息 
/// @param	UserID:	登录返回的ID
/// @param	pReqInfo:	请求消息ReqDeleteAIUserPicInfo
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DelAiUserPicInfo(DWORD UserID, const char *pReqInfo);

/// @brief	AI图片上传到nvr	添加和修改都用这个接口
/// @param	UserID:	登录返回的ID
/// @param	pReqMsg:	ReqModifyAIUserPicInfo消息json串
/// @param	pData:		图片数据
/// @param	nDataLen:	图片数据长度
VSK_CSDK int CALL_METHOD JsonSdk_AiUserPicUpload(DWORD UserID, const char* pReqMsg, const uint8* pData, uint32 nDataLen);


// {{ 反向代理相关接口
/// @brief 获取 反向连接的反向代理参数配置
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回ReverseLinkProxyConfig消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetReverseLinkProxyConfig(DWORD UserID, char* pInfo, UINT *pInfoSize);

/// @brief	设置反向代理协议信息配置
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReverseLinkProxyProtocol消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetReverseLinkProxyProtocol(DWORD UserID, const char* pInfo);

/// @brief	添加或者修改被代理的 nvr信息
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReverseLinkProxedInfo消息json串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetReverseLinkProxedInfo(DWORD UserID, const char* pInfo);

/// @brief	删除被代理的 nvr信息
/// @param	UserID:	登录返回的ID
/// @param	pInfo:	ReqDelReverseLinkProxied消息json串: 设备编号
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DelReverseLinkProxedInfo(DWORD UserID, const char* pInfo);

// }} 反向代理相关接口

// tmp func, 空的接口!
VSK_CSDK int CALL_METHOD JsonSdk_GetNVRConfig(DWORD UserID, DWORD Command, UINT Channel,
	LPVOID pOutBuffer, DWORD InBufferSize, DWORD *OutBufferSize);
VSK_CSDK int CALL_METHOD JsonSdk_SetNVRConfig(DWORD UserID, DWORD Command, UINT Channel,
	LPVOID InBuffer, DWORD InBufferSize);



// {{ raid配置相关接口
/// @brief 获取 raid卡列表
/// @param	nUserID:	登录返回的ID
/// @retval	pInfo:	返回RaidCardList消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetRaidCardList(DWORD nUserID, char* pInfo, UINT *pInfoSize);

/// @brief 获取 逻辑磁盘状态和信息
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 查询消息ReqRaidParam : controller_index + logical_disk_index
/// @retval	pInfo:	返回LogicalDiskStateInfo消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetRaidLogicalDiskInfo(DWORD nUserID, const char *pReqMsg, char* pInfo, UINT *pInfoSize);

/// @brief 获取 物理磁盘状态和信息
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam : controller_index
/// @retval	pInfo:	返回RaidPhysicalDiskList消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetRaidPhysicalDiskList(DWORD nUserID, const char *pReqMsg, char* pInfo, UINT *pInfoSize);


/// @brief	转为热备盘
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam : controller_index + disk_slot + eid(只有一个磁盘柜时可以为0, 36盘位有两个磁盘柜)
/// @return 成功返回0,失败返回错误码
/// @note 删除热备盘JsonSdk_DeleteHotDisk
VSK_CSDK int CALL_METHOD JsonSdk_ConvertToHotDisk(DWORD nUserID, const char* pReqMsg);

/// @brief	开启或者关闭是否使用指定raid卡上的缓存
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam: controller_index + logical_disk_index + enable + activity_type(READ_CACHE, WRITE_CACHE, IO_POLICY)
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetRaidCardCache(DWORD nUserID, const char* pReqMsg);

/// @brief	关闭raid卡声音报警
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam: controller_index
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SilenceRaidAlarm(DWORD nUserID, const char* pReqMsg);

/// @brief	开启或者关闭磁盘定位
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam: controller_index + disk_slot(-1表示关闭所有) + enable  + eid(disk_slot != -1时:只有一个磁盘柜时可以为0, 36盘位有两个磁盘柜)
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_RaidLocateDisk(DWORD nUserID, const char* pReqMsg);

/// @brief 获取raid是否开启了指定的活动或功能
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam : controller_index + activity_type(AUTO_REBUILD, COPY_BACK, CC)
/// @retval	pInfo:	返回RaidActivityVaule消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetRaidActivityEnabled(DWORD nUserID, const char *pReqMsg, char* pInfo, UINT *pInfoSize);

/// @brief 获取raid的活动任务占系统性能的百分比值: 30表示30%
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam : controller_index + activity_type(AUTO_REBUILD, CC, BGI)
/// @retval	pInfo:	返回RaidActivityVaule消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetRaidActivityRate(DWORD nUserID, const char *pReqMsg, char* pInfo, UINT *pInfoSize);

/// @brief 获取raid的活动任务的进度 百分比
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam	: controller_index + activity_type(CC, BGI, INIT) + [logical_disk_index]
///										: controller_index + activity_type(AUTO_REBUILD, COPY_BACK) + [logical_disk_index] + disk_slot(-1: 表示不限定磁盘)
/// @retval	pInfo:	返回RaidArrayProgress消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码, 返回的eid可能不正确, 客户端不显示eid!
VSK_CSDK int CALL_METHOD JsonSdk_GetRaidActivityProgress(DWORD nUserID, const char *pReqMsg, char* pInfo, UINT *pInfoSize);

/// @brief	设置raid是否开启指定的活动或功能
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam: controller_index + activity_type + enable   (AUTO_REBUILD, COPY_BACK, CC)
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetRaidActivityEnabled(DWORD nUserID, const char* pReqMsg);

/// @brief	设置raid的活动任务占系统性能的百分比值: 30表示30%
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam: controller_index + activity_type + enable(存放百分比值)(AUTO_REBUILD, CC, BGI)
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetRaidActivityRate(DWORD nUserID, const char* pReqMsg);

/// @brief	开始raid的活动任务  cc, rebuild
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam	: controller_index + activity_type(CC, INIT) + logical_disk_index
///										: controller_index + activity_type(AUTO_REBUILD) + disk_slot + eid(只有一个磁盘柜时可以为0, 36盘位有两个磁盘柜)
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_StartRaidActivity(DWORD nUserID, const char* pReqMsg);

/// @brief	删除逻辑raid盘
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam	: controller_index + logical_disk_index(-1表示删除所有逻辑硬盘) + enable(是否强制删除)
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DeleteRaidLogicalDisk(DWORD nUserID, const char* pReqMsg);

/// @brief	创建逻辑raid盘 目前暂只支持raid5
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqCreateRaidLogicalDisk	: controller_index + disk_slot_list + [raid_type] + eid(只有一个磁盘柜时可以为0, 36盘位有两个磁盘柜)
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_CreateRaidLogicalDisk(DWORD nUserID, const char* pReqMsg);

// }} raid配置相关接口


/// @brief	测试关闭nvrapp程序
/// @param	nUserID:	登录返回的ID
/// @return 成功返回0,失败返回错误码
/// @note 仅仅用于测试
VSK_CSDK int CALL_METHOD JsonSdk_TestExitNvrapp(DWORD nUserID);


// {{ 文件下载相关接口  bytes类型字段 不使用base64编码

/// @brief 获取 指定目录的文件列表
/// @param	nLinkID:	登录或者获取连接返回的ID
/// @param	pReqMsg:	消息ReqCommonDevFileInfoList
/// @retval	pInfo:		返回AckCommonDevFileInfoList消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetDirCommonFileList(DWORD nLinkID, const char *pReqMsg, char* pInfo, UINT *pInfoSize);

/// @brief	压缩指定文件列表
/// @param	nLinkID:	登录或者获取连接返回的ID
/// @param	pReqMsg:	消息ReqCompressCommonFile
/// @param	DataPlayCallBack: 下载数据回调, 可以为NULL
/// @param	pUser:		用户信息 调用参数
/// @return 成功返回0,失败返回错误码
/// @note 回调DataType: 0: 数据开始,pBuffer指向一个uint64地址的文件大小内存,size=0,  1: 正常数据流, 100:数据流开始 2: 数据流结束
VSK_CSDK int CALL_METHOD JsonSdk_DownloadCompressCommonFileList(DWORD nLinkID, const char* pReqMsg, DataPlayCallBack downloadCallBack, LPVOID pUser);

/// @brief 大文件下载
/// @param	nLinkID:	登录或者获取连接返回的ID
/// @param	pReqMsg:	下载数据请求内容, ReqCommonFileDownloadStart
/// @param	DataPlayCallBack: 下载数据回调, 可以为NULL
/// @param	pUser:		用户信息 调用参数
/// @return 成功返回0,失败返回错误码
/// @note 回调DataType: 0: 数据开始,pBuffer指向一个uint64地址的文件大小内存,size=0,  1: 正常数据流, 100:数据流开始 2: 数据流结束
VSK_CSDK int CALL_METHOD JsonSdk_CommonFileDownload(int32 nLinkID, const char *pReqMsg, DataPlayCallBack downloadCallBack, LPVOID pUser);

/// @brief	删除指定文件, 文件必须存在
/// @param	nLinkID:	登录或者获取连接返回的ID
/// @param	pReqMsg:	消息ReqDeleteCommonDevFile
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DeleteCommonFile(DWORD nLinkID, const char* pReqMsg);

/// @brief 小文件下载
/// @param	nLinkID:	连接ID
/// @param	nDirType:	目录类型
/// @param	pFileName:	下载文件名称
/// @param	ppFileBuf:	文件内容指针的地址
/// @param	pFileBufLen:	文件长度
/// @return 成功返回0,失败返回错误码
/// @note	下载的图片大小不能超过10MB, 删除下载缓存调用JsonSdk_FreeBuffer
VSK_CSDK int CALL_METHOD JsonSdk_CommonSmallFileDownload(DWORD nLinkID, unsigned int nDirType, const char *pFileName, unsigned char** ppFileBuf, unsigned * pFileBufLen);

/// @brief 文件上传
/// @param	nLinkID:	登录或者获取连接返回的ID
/// @param	pReqMsg:	上传数据请求内容, ReqCommonFileDownloadStart: {dir_type, file_name}
/// @return 成功返回0,失败返回错误码
/// @note 文件名称相同时, 覆盖; 通过调用JsonSdk_GetUploadFileStatus 获取上传进度
VSK_CSDK int CALL_METHOD JsonSdk_CommonFileUpload(int32 nLinkID, const char *pReqMsg);

// }} 文件下载相关接口

/// @brief	删除热备盘
/// @param	nUserID:	登录返回的ID
/// @param	pReqMsg: 消息ReqRaidParam : controller_index + disk_slot  + eid(只有一个磁盘柜时可以为0, 36盘位有两个磁盘柜)
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_DeleteHotDisk(DWORD nUserID, const char* pReqMsg);

//--------------------------------------------------licence管理--------------------------------------------------
/// @brief	获取当前系统的licence信息
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回IVSystemConfig消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
/// @note cmsSdk对应的接口为: IvSdk_GetIVSystemConfig
VSK_CSDK int CALL_METHOD JsonSdk_GetNvrLicenceInfo(DWORD nUserID, char* pInfo, UINT *pInfoSize);

/// @brief	获取指定licence文件的详细信息
/// @param	UserID:	登录返回的ID
/// @param	pReqInfo: 请求的licence文件的base64编码串
/// @retval	pInfo:	返回IVSystemConfig消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
/// @note 该接口可以不使用! 只是用来查看授权文件的信息,cmsSdk对应的接口为: IvSdk_GetIVLicenceInfo
VSK_CSDK int CALL_METHOD JsonSdk_ParseLicenceInfo(DWORD nUserID, const char *pReqInfo, char* pInfo, UINT *pInfoSize);

/// @brief	请求更新licence文件
/// @param	UserID:	登录返回的ID
/// @param	pReqInfo: 授权licence文件内容的base64编码串
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_UpdateNvrLicenceInfo(DWORD nUserID, const char* pReqInfo);

//--------------------------------------------------录像存储运行信息--------------------------------------------------
/// @brief	获取指定硬盘的录像存储运行信息
/// @param	UserID:	登录返回的ID
/// @param	pReqInfo: 请求指定磁盘参数, ReqDiskRuntimeInfo
/// @retval	pInfo:	返回AckDiskRuntimeInfo消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetDiskRuntimeInfo(DWORD nUserID, const char *pReqInfo, char* pInfo, UINT *pInfoSize);

/// @brief	导出硬盘空间使用情况为bmp文件到/var/log目录
/// @param	UserID:	登录返回的ID
/// @param	pReqInfo: 请求指定磁盘参数, ReqDiskRuntimeInfo
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_ExportDiskRuntimeUsageBitmap(DWORD nUserID, const char *pReqInfo);

//--------------------------------------------------文件上传速度控制--------------------------------------------------
/// @brief 修改升级时文件上传速度, 
/// @param	nUpgradeHandle:	升级操作ID
/// @param	nSpeedRate:	速度万分比, 5000时为 0.5倍速, 20000时为2倍速,  基准速度最大默认为7MB/s
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_SetUploadFildSpeed(DWORD nUpgradeHandle, unsigned int nSpeedRate);

/// @brief 调试命令请求消息, 
/// @param	UserID:	登录返回的ID
/// @param	pReqInfo: 请求指定磁盘参数, ReqDebugNvrCommandMsg
/// @retval	pInfo:	返回字符串消息，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
/// @note code: EDebugNvrCommandCode: =1: 测试卸载指定硬盘, 需要设置str_info, 比如:/dev/sdb
/// @note code=2: 测试添加指定硬盘, 可以设置str_info, 比如:/dev/sdb, 为空时表示添加所有之前测试卸载的硬盘
/// @note code=3: 测试ping命令 参数: str_info参数:ip, nData: 次数, <1 或者大于10时为4次, 返回消息: result_str
VSK_CSDK int CALL_METHOD JsonSdk_DebugNvrCommand(DWORD nUserID, const char *pReqInfo, char* pInfo, UINT *pInfoSize);


/// @brief	同时修改 多个IPC信息
/// @param	UserID:		登录返回的ID
/// @retval	pIpcList:	IPC信息列表: ReqSetServerUriList
/// @return 成功返回0,失败返回错误码
/// @note	只修改一个ipc参数使用接口: JsonSdk_ModifyIP
VSK_CSDK int CALL_METHOD JsonSdk_ModifyIPCList(DWORD nUserID, const char* pIpcList);


/*  JsonSdk_SetVideoNoVskHeadFrameCallBack 使用
1. 首先调用 JsonSdk_SetVideoNoVskHeadFrameCallBack(OnNoVskDataCallBack);
2. 再调用 nRet = JsonSdk_VideoTranspondStart(m_dwLogin, VideoDataPlayCallBack, (char*) strJsonReq.c_str(), (uint64)this, (void*)gnPreviewDecCodePort, &hHandle);
VideoDataPlayCallBack 不会被回调, 直接回调 OnNoVskDataCallBack
3.
void __stdcall OnNoVskDataCallBack(const VideoNoViskHeadFrameInfo *pStream)
{
	FILE* pF = fopen("test.h264", "ab");
	if (pF)
	{
		fwrite(pStream->pBuf, 1, pStream->nFrameLen, pF);
		fclose(pF);
	}
}
4. 使用vlc播放 test.h264
*/

/// @brief	设置没有vsk消息头的视频流消息回调函数
/// @param	videoCallBack:	接收没有vsk消息头的视频数据回调函数
/// @return 返回0
/// @note 设置该回调会把下面几个函数的回调修改为当前的回调
/// @note JsonSdk_VideoTranspondStart,JsonSdk_PlayBackStartByFile,JsonSdk_PlayBackStartByTime,JsonSdk_DownloadStart,JsonSdk_DownloadStartByTime
VSK_CSDK int CALL_METHOD JsonSdk_SetVideoNoVskHeadFrameCallBack(VideoNoViskHeadFrameCallBack videoCallBack);


//--------------------------------------------------系统运行信息--------------------------------------------------
/// @brief	获取系统运行信息
/// @param	UserID:	登录返回的ID
/// @retval	pInfo:	返回SystemRuntimeConfig消息json串，空间不足时无返回
/// @retval pInfoSize:	输入pInfo的大小,返回消息json串的大小, 不能为NULL
/// @return 成功返回0,失败返回错误码
VSK_CSDK int CALL_METHOD JsonSdk_GetSystemRuntimeInfo(DWORD nUserID, char* pInfo, UINT *pInfoSize);

#endif


