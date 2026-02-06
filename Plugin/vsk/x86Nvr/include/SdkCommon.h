#ifndef __SDK_COMMON__H
#define __SDK_COMMON__H

#include "Type.h"
#include "CustomCommon.h"

#ifdef _WIN32
#if (defined  VSK_CLIENT_SDK_EXPORTS) || (defined  SDKIVILLAGE_EXPORTS)
#define VSK_CSDK extern "C"  __declspec(dllexport)
#else
#define VSK_CSDK extern "C"  __declspec(dllimport)
#endif
#else
#define VSK_CSDK extern "C"
#endif

#ifdef _WIN32
#define CALL_METHOD __stdcall
#else
#define CALL_METHOD
#endif

//-----------------------报警消息cmd及type -------------------------
/// 图片报警
typedef enum
{
	ALARM_TYPE_BLACK_LIST = 0,		// 黑名单
	ALARM_TYPE_WHITE_LIST = 1,		// 白名单
	ALARM_TYPE_PASSER_BY = 2,		// 路人
}ALARM_TYPE_FACE;
typedef enum
{
	NUMBER_PLATE_NM_ALARM = 0,		//普通车牌
	NUMBER_PLATE_NEWAN_ALARM = 1,	//新能源车牌
}ALARM_TYPE_CAR;

typedef enum
{
	ALARM_RAID_NOT_NORMAL = 0,				// raid卡不是正常状态报警
	ALARM_RAID_READ_ONLY = 1,				// raid只读报警,不可写报警
	ALARM_RAID_NOT_CONSISTENT = 2,			// raid 不符合一致性校验报警  --> 客户端查看 bgi 或者cc进度

	ALARM_RAID_GET_PHYSIAL_DISK_LIST = 3,	// raid 获取物理磁盘状 错误报警
	ALARM_RAID_PHYSIAL_DISK_STATE = 4,		// raid 物理磁盘状态报警: 不是 online或热备盘都报警
	ALARM_RAID_NO_HOT_DISK = 5,				// raid 没有热备盘报警
	ALARM_RAID_DELETE_DISK_RESULT = 6,		// raid 删除raid逻辑磁盘结果通知
}ALARM_TYPE_RAID;

typedef enum
{
	ALARM_TYPE_X86NVR_REBOOT_IPC = 0,				// nvr重启ipc报警
	ALARM_TYPE_X86NVR_NO_FILE_RECORD_DISK = 1,		// 硬盘状态错误, 没有可录像硬盘报警  --> 录像机没有录像
	ALARM_TYPE_X86NVR_DISK_ADD = 2,					// 添加新硬盘 info: json: NvrMsg::NvrDiskStateInfo
	ALARM_TYPE_X86NVR_DISK_REMOVE = 3,				// 移除硬盘   info: json: NvrMsg::NvrDiskStateInfo
	ALARM_TYPE_X86NVR_DISK_STATE = 4,				// 硬盘状态错误: 单个硬盘状态错误    info: json: NvrMsg::NvrDiskStateInfo
}ALARM_TYPE_X86NVR;

typedef enum
{
	NOTIFY_TYPE_X86NVR_SYSTEM_RUNTIME_INFO = 0,				// nvr系统运行信息
}NOTIFY_TYPE_X86NVR;

// ALARM_LOG_ROOT_SYS_CONTROL 系统控制子类型
typedef enum
{
	ALARM_TYPE_SYS_CONTROL_NVRAPP_START		= 0,				// 启动nvrapp		no info
	ALARM_TYPE_SYS_CONTROL_NVRAPP_STOP		= 1,				// 退出nvrapp		no info
	ALARM_TYPE_SYS_CONTROL_SET_TIME_BEGIN	= 2,				// 开始更新nvr时间	LogSetNvrTimeInfo
	ALARM_TYPE_SYS_CONTROL_SET_TIME_END		= 3,				// 完成更新nvr时间	LogSetNvrTimeInfo
	ALARM_TYPE_SYS_CONTROL_START_NTPD		= 4,				// 启动ntpd			LogNtpTimingInfo	
	ALARM_TYPE_SYS_CONTROL_STOP_NTPD		= 5,				// 关闭ntpd			LogNtpTimingInfo
	ALARM_TYPE_SYS_CONTROL_SET_IPC_TIME		= 6,				// 更新ipc时间		no info
	ALARM_TYPE_DISK_FORMAT					= 7,				// 硬盘格式化
	ALARM_TYPE_RAID_CONTROL					= 8,				// raid控制			LogRaidControlInfo: op_code:	RAID_CONTROL_OP_CODE
	ALARM_TYPE_NETWORK_CONTROL				= 9,				// 网络控制			LogNetworkControlInfo: op_code:	NETWORK_CONTROL_OP_CODE
	//ALARM_TYPE_LIDAR_CONTROL				= 10,				// 雷达配置			LogLidarConfigInfo: op_code:	ESetLidarConfigCmd 雷达nvr预留
	ALARM_TYPE_STORAGE_CONTROL				= 11,				// 存储控制			LogStorageControlInfo: op_code:	STORAGE_CONTROL_OP_CODE

}ALARM_LOG_ROOT_SYS_CONTROL_SUB_TYPE;

// ALARM_TYPE_RAID_CONTROL raid控制操作
typedef enum
{
	RAID_CONTROL_OP_CREATE_LOGICAL_DISK = 0,				// 创建raid逻辑磁盘 参考结构 ReqCreateRaidLogicalDisk
	RAID_CONTROL_OP_DELETE_LOGICAL_DISK = 1,				// 删除raid逻辑磁盘: controller_index, logical_disk_index
	RAID_CONTROL_OP_CONVERT_TO_HOT_DISK = 2,				// 转为热备盘:  controller_index, eid, disk_slot
	RAID_CONTROL_OP_DELETE_HOT_DISK		= 3,				// 删除热备盘:  controller_index, eid, disk_slot
	RAID_CONTROL_OP_ENABLE_RAID_CARD_CACHE = 4,				// 启动或禁用raid卡缓存: controller_index, logical_disk_index, activity_type, enable
	RAID_CONTROL_OP_SILENCE_RAID_CARD_ALARM = 5,			// 关闭raid卡声音报警: controller_index,
	RAID_CONTROL_OP_LOCATE_DISK = 6,						// 对硬盘定位或取消定位: controller_index, eid, disk_slot, enable
	RAID_CONTROL_OP_SET_ACTIVITY_ENABLE = 7,				// 启用或禁用 自动cc, copy_back和rebuild功能:  controller_index, activity_type, enable
	RAID_CONTROL_OP_SET_ACTIVITY_RATE = 8,					// 设置比率: cc, bgi和rebuild功能: controller_index, activity_type, enable(rate)
	RAID_CONTROL_OP_START_ACTIVITY = 9,						// 执行操作:rebuild, cc, init:  controller_index, logical_disk_index, activity_type, [eid, disk_slot](rebuild)
	
}RAID_CONTROL_OP_CODE;

// ALARM_TYPE_NETWORK_CONTROL 网络控制操作
typedef enum
{
	NETWORK_CONTROL_OP_NETWORK = 0,							// 修改网络配置
	NETWORK_CONTROL_OP_PORT = 1,							// 修改网络端口
	NETWORK_CONTROL_OP_DNS = 2,								// 修改dns配置
	NETWORK_CONTROL_OP_NTP = 3,								// 修改NTP配置

}NETWORK_CONTROL_OP_CODE;

// ALARM_TYPE_STORAGE_CONTROL 存储控制操作
typedef enum
{
	STORAGE_CONTROL_OP_FILE_LOCK = 0,						// 文件锁定
	STORAGE_CONTROL_OP_FILE_UNLOCK = 1,						// 文件锁定
	STORAGE_CONTROL_OP_DISK_LOCK = 2,						// 设置硬盘只读
	STORAGE_CONTROL_OP_DISK_UNLOCK = 3,						// 设置硬盘只读

}STORAGE_CONTROL_OP_CODE;
// nvr和cms的报警cmd类型
typedef enum
{
	SDK_ALARM_COMMAND_DAHUA_NVR = 416,	// 大华nvr报警

	SDK_ALARM_INFO_UPGRADE = 417,		// test only!
	SDK_CMD_SEND_IPC_STATUS = 420,		// alarm type: IPC_CHANNEL_STATUS
	SDK_ALARM_START_UPLOAD = 419,		// ipc报警开始 UNS_ALARM_TYPE  EVENT_TYPE_ALARM_IN
	SDK_ALARM_STOP_UPLOAD = 421,		// 停止报警上传: 取消报警
	SDK_ALARM_FACE_AI = 422,			// 图片报警 type:ALARM_TYPE_FACE
	SDK_CMD_CAR_ALARM = 423,			// 车牌报警 type:ALARM_TYPE_CAR 
	
	SDK_CMD_RAID_ALARM = 424,			// raid报警消息 type:ALARM_TYPE_RAID
	SDK_CMD_T_ALARM = 425,				// 测温图片报警

	SDK_CMD_FACE_DETECT_ALARM = 426,	// 人脸检测报警 type:UNS_ALARM_TYPE: UNS_ALARM_HOVER_DETECTION -- UNS_ALARM_FACE_RECOGNITION
	SDK_CMD_CAR_DETECT_ALARM = 427,		// 停车检测 type:UNS_ALARM_TYPE: UNS_ALARM_PARK_DETECT
	SDK_CMD_OBJECT_DETECT_ALARM = 428,	// 物体检测报警 type:UNS_ALARM_TYPE: UNS_ALARM_OBJECT_LOST_DETECTION, UNS_ALARM_OBJECT_LEFT_DETECTION, UNS_ALARM_OBJECT_FALLING ...

	SDK_CMD_X86NVR_ALARM = 490,			// X86NVR报警  ALARM_TYPE_X86NVR
	SDK_CMD_X86NVR_NOTIFY = 491,		// X86NVR主动通知消息, type:NOTIFY_TYPE_X86NVR
	// cms 事件
	SDK_CMD_CW_ALARM = 500,				// 丛文报警平台上传到cms, cms通知给客户端的报警消息
	SDK_CMD_DOWNLOAD_REQUEST_ALARM = 501,				// 下载申请通知客户端
	SDK_CMD_IV_NODE_CHANGE = 1001,			// ivillage node表记录变更消息 type:ALARM_TYPE_IVILLAGE_TABLE
	SDK_CMD_IV_ALARM_LINK_DEC_CHANGE = 1002,// ivillage AlarmLinkDecInfo表记录变更消息 type:ALARM_TYPE_IVILLAGE_TABLE
	SDK_CMD_IV_ALARM_LINK_MATRIX_CHANGE = 1003,// ivillage AlarmLinkMatrixInfo表记录变更消息 type:ALARM_TYPE_IVILLAGE_TABLE
	SDK_CMD_IV_KEY_VALUE_TABLE_CHANGE = 1004,		// key value 表配置变化 set(update), del
	SDK_CMD_IV_DEC_PLAN_STATE_CHANGE = 1005,		// 解码器预案状态变化: ALARM_STATUS_CHANGE_TYPE
}SDK_ALARM_COMMAND;

typedef enum
{
	EALARM_CH_IPC_OFFLINE = 1,					// 2^0	ipc下线
	EALARM_CH_IPC_ONLINE = 2,					// 2^1	ipc 上线
	EALARM_CH_IPC_ADD = 4,						// 2^2	添加ipc
	EALARM_CH_IPC_DEL = 8,						// 2^3	删除ipc
	EALARM_CH_IPC_LOGIN_ERROR = 16,				// 2^4	登录ipc错误
	EALARM_CH_IPC_CONNECT_ERROR = 32,			// 2^5	连接ipc错误
	EALARM_CH_IPC_DISK_ERROR = 64,				// 2^6	ipc硬盘错误
	EALARM_CH_IPC_UPDATE = 128,  				// 2^7	ipc信息更新

	EALARM_UNS_ALARM_MOTION = 256,  			// 2^8	 // 移动侦测
	EALARM_UNS_ALARM_VIDEO_LOSS = 512,			// 2^9	 // 视频丢失
	EALARM_UNS_ALARM_BLIND_DETECT = 1024,		// 2^10	 // 视频遮盖
	EALARM_UNS_ALARM_ALARM_IN = 2048,  			// 2^11	 // nvr报警输入
	EALARM_UNS_ALARM_IPC_ALARMIN = 4096,  		// 2^12	 // ipc报警输入

	EALARM_ALARM_RAID_NOT_NORMAL = 8192,  			// 2^13		// raid卡不是正常状态报警
	EALARM_ALARM_RAID_READ_ONLY = 16384,  			// 2^14		// raid只读报警,不可写报警
	EALARM_ALARM_RAID_NOT_CONSISTENT = 32768,		// 2^15		// raid 不符合一致性校验报警  --> 客户端查看 bgi 或者cc进度
	EALARM_ALARM_RAID_GET_PHYSIAL_DISK_LIST = 65536,// 2^16		// raid 获取物理磁盘状 错误报警
	EALARM_ALARM_RAID_PHYSIAL_DISK_STATE = 131072,  // 2^17		// raid 物理磁盘状态报警: 不是 online或热备盘都报警
	EALARM_ALARM_RAID_NO_HOT_DISK = 262144,  		// 2^18		// raid 没有热备盘报警
	EALARM_ALARM_RAID_DELETE_DISK_RESULT = 524288, 	// 2^19		// raid 删除raid逻辑磁盘结果通知	
	
}E_DisableNvrAlarmByType;

// node表变更
typedef enum
{
	IV_TABLE_ADD_RECORD = 0,		// 添加记录
	IV_TABLE_UPDATE_RECORD = 1,		// 更新记录
	IV_TABLE_DEL_RECORD = 2,		// 删除记录
	IV_TABLE_REFRESH = 3,			// 刷新列表
}ALARM_TYPE_IVILLAGE_TABLE;

// 状态事件索引
typedef enum
{
	IV_ALARM_STATUS_STOP = 0,		// 结束
	IV_ALARM_STATUS_SUSPEND = 1,	// 暂停
	IV_ALARM_STATUS_RESUME = 2,		// 恢复
	IV_ALARM_STATUS_START = 3,		// 开始 startDec
}ALARM_STATUS_CHANGE_TYPE;

// 人脸检测报警类型
//typedef enum
//{
//	EFACE_ALARM_DETECT_SMALL_FACE = 0,		// 人脸检测: 小图片
//	EFACE_ALARM_DETECT_LARGE_FACE = 1,		// 人脸检测: 普通图片
//	EFACE_ALARM_FACERE_COGNITION_FACE = 2,	// 人脸识别: 小图片
//
//
//	EFACE_ALARM_FACERE_MOTION_DETECT	= 3,	// 移动侦测, 对应一个图片
//	EFACE_ALARM_FACERE_VIDEO_LOSS		= 4,	// 视频丢失, 对应一个图片
//	EFACE_ALARM_FACERE_BLIND_DETECT		= 5,		// 视频遮盖, 对应一个图片
//
//	EFACE_ALARM_FACERE_CROSS_ZONE_ALARMIN = 6,		// 进入警戒区
//	EFACE_ALARM_FACERE_CROSS_LINE_ALARMIN = 7,		// 跨越警戒线报警
//	EFACE_ALARM_FACERE_OBJECT_LEFT_DETECTION = 8,	// 物品遗留检测报警
//	EFACE_ALARM_FACERE_OBJECT_LOST_DETECTION = 9,	// 物品遗失
//
//
//	EFACE_ALARM_FACERE_UNKONWN_TYPE = 400,	// 未知类型
//
//
//}ALARM_TYPE_FACE_DETECT_ALARM;
//-----------------------------------------------------------
#define MAX_ALARM_OUT_PORT_COUNT	4

// nvrapp 通道数量允许的最大值
#define VALID_NVRAPP_MAX_CHANNEL_COUNT 65536

//日志最大数量
#define  LOG_MAX_COUNT			2000
// 24盘位型号nvr						101 -- 110
#define NVR_24_LINUX_V1			101
// x86 的 普通 流媒体 版本				111-200
#define X86_MS_LINUX_V1			111
// x86 的 hls 流媒体 版本
#define X86_MS_RTMP_HLS_LINUX_V1	112

#define CMS_IVILLAGEE_V1		900
typedef enum
{
	CH_IPC_OFFLINE,
	CH_IPC_ONLINE,
	CH_IPC_ADD,
	CH_IPC_DEL,
	CH_IPC_LOGIN_ERROR,
	CH_IPC_CONNECT_ERROR,
	CH_IPC_DISK_ERROR,
	CH_IPC_UPDATE,   // ipc信息变化: 被修改
}IPC_CHANNEL_STATUS;

enum PLAY_BACK_RATE
{
	PLAY_BACK_RATE_MIN = 64,

	PLAY_BACK_RATE_L8 = 64,
	PLAY_BACK_RATE_L4 = 256,
	PLAY_BACK_RATE_L2 = 512,
	PLAY_BACK_RATE_NORMAL = 1024,
	PLAY_BACK_RATE_X2 = 1024 * 2,
	PLAY_BACK_RATE_X4 = 1024 * 4,
	PLAY_BACK_RATE_X8 = 1024 * 8,
	PLAY_BACK_RATE_X16 = 1024 * 16,
	PLAY_BACK_RATE_X32 = 1024 * 32,
	PLAY_BACK_RATE_X64 = 1024 * 64,

	PLAY_BACK_RATE_MAX,
};

//24盘位ptz命令
enum UNS_PTZCONTROL_COMMAND_E
{
	UNS_PTZCOMMAND_UNKNOWN = 0,
	UNS_PTZCOMMAND_UP = 1,
	UNS_PTZCOMMAND_DOWN = 2,
	UNS_PTZCOMMAND_LEFT = 3,
	UNS_PTZCOMMAND_RIGHT = 4,
	UNS_PTZCOMMAND_UP_LEFT = 5,
	UNS_PTZCOMMAND_UP_RIGHT = 6,
	UNS_PTZCOMMAND_DOWN_LEFT = 7,
	UNS_PTZCOMMAND_DOWN_RIGHT = 8,

	UNS_PTZCOMMAND_ZOOM_IN = 9,
	UNS_PTZCOMMAND_ZOOM_OUT = 10,
	UNS_PTZCOMMAND_FOCUS_NEAR = 11,
	UNS_PTZCOMMAND_FOCUS_FAR = 12,
	UNS_PTZCOMMAND_IRIS_OPEN = 13,
	UNS_PTZCOMMAND_IRIS_CLOSE = 14,
	UNS_PTZCOMMAND_PAN_AUTO = 15,

	UNS_PTZCOMMAND_AUX_ON = 16,
	UNS_PTZCOMMAND_AUX_OFF = 17,

	UNS_PTZCOMMAND_LIGHT_OPEN = 18,
	UNS_PTZCOMMAND_LIGHT_CLOSE = 19,
	UNS_PTZCOMMAND_WIPER_OPEN = 20, 
	UNS_PTZCOMMAND_WIPER_CLOSE = 21,
	UNS_PTZCOMMAND_CALL_PRESET = 22,        //调用预设点
	UNS_PTZCOMMAND_SAVE_PRESET = 23,        //设置预设点
	UNS_PTZCOMMAND_BUTT,
};
// 编码类型 iv_encode_type_code.type
enum EEncodeTypeCode
{
	EncodeTypeProvince			= 1,  // 省编码
	EncodeTypeInfoSource		= 2,  // 信息来源编码
	EncodeTypeHouseTypeCode		= 3,  // 房屋类别定义  houseLabelCode
	EncodeTypeHousePurposeCode	= 4,  // 房屋用途定义
	EncodeTypeEquipCode			= 5,  // 装备类型编码
	EncodeTypeCompanyTypeCode	= 6,  // 单位性质编码
	EncodeTypeCompanyScaleCode	= 7,  // 单位规模编码
	EncodeTypeTradeCode			= 8,  // 行业编码
	EncodeTypeGisTypeCode		= 9,  // 坐标系编码（getGisType）

	EncodeTypeCredentialTypeCode = 10,  // 证件类型编码
	EncodeTypeEducationCode		= 11,  // 文化程度分类
	EncodeTypeMaritalStatus		= 12,  // 婚姻状况分类
	EncodeTypeNationCode		= 13,  // 民族编码
	EncodeTypePeopleLabelCode	= 14,  // 人员特征分类
	EncodeTypeHouseRelCode		= 15,  // 人屋关系类型编码
	EncodeTypePeopleTypeCode	= 16,  // 人员类型编码
	EncodeTypeSecurityDutiesCode = 17,  // 专业保安职责编码
	EncodeTypePowerCode			= 18,  // 实有力量分类编码
	EncodeTypePublicOrderCode	= 19,  // 治安人员类型编码
	EncodeTypeCardTypeCode		= 20,  // 卡类型

	EncodeTypeEntranceCertifiedTypeCode = 21,  // 出入口控制认证类型编码
	EncodeTypeEntranceEventCode	= 22,  // 事件类型编码
	EncodeTypeEntrancePersonTypeCode = 23,  // 出入人员类型编码
	EncodeTypeCarTypeCode		= 24,  // 车辆类型编码
	EncodeTypeCarPlateTypeCode	= 25,  // 车辆牌照类型
	EncodeTypeCarPlateColorCode = 26,  // 牌照颜色
	EncodeTypeParkingEventCode	= 27,  // 停车场事件编码
	EncodeTypeSnapEventTypeCode = 28,  // 抓拍事件编码
	EncodeTypeAlarmEventCode	= 29,  // 报警事件编码
	EncodeTypeCollectionEventCode = 30,  // 状态采集设备事件编码
	EncodeTypeHandleStateCode	= 31,  // 事件处置状态编码
	// 有参数的编码
	EncodeTypeCityCode			= 100,  // 城市编码（getCityCode）
	EncodeTypeDistrictCode		= 101,  // 区县编码（getDistrictCode）
	EncodeTypeStreetCode		= 102,  // 街道编码（getStreetCode）
	EncodeTypeRoadCode			= 103,  // 取道路编码（getRoadCode）
	EncodeTypePoliceStationCode	= 104,  // 属地派出所编码（getPoliceStationCode）
};

/// 文件传输类型
enum ETRANSFER_FILE_TYPE
{
	TRANSFER_FILE_TYPE_AI_FACE	= 0, // 默认为人脸图片
	TRANSFER_FILE_TYPE_IV_PIC	= 1, // 智慧小区上传的图片
};

/// 时区索引和时区关系
enum TZ_TIME_ZONE_INDEX
{
	TZ_CST_12 = 1,		// 1:GMT-12:00 日界线西
	TZ_CST_11 = 2,		// 2:GMT-11:00 中途岛，萨摩亚群岛
	TZ_CST_10 = 3,		// 3:GMT-10:00 夏威夷
	TZ_CST_9 = 4,		// 4:GMT-09:00 阿拉斯加
	TZ_CST_8 = 5,		// 5:GMT-08:00 太平洋时间(美国和加拿大)
	TZ_CST_7 = 5,		// 5:GMT-07:00 山地时间(美国和加拿大)
	TZ_CST_6 = 6,		// 6:GMT-06:00 中部时间(美国和加拿大)
	TZ_CST_5 = 7,		// 7:GMT-05:00 东部时间(美国和加拿大)
	TZ_CST_4 = 8,		// 8:GMT-04:00 大西洋时间(加拿大)
	TZ_CST_3 = 9,		// 9:GMT-03:00 乔治敦、巴西利亚
	TZ_CST_2 = 11,		// 11GMT-02:00 中大西洋
	TZ_CST_1 = 12,		// 12GMT-01:00 佛得角群岛、亚速尔群岛
	TZ_CST0 = 13,		// 13GMT+00:00 都柏林、爱丁堡、伦敦
	TZ_CST1 = 14,		// 14GMT+01:00 阿姆斯特丹、柏林、罗马、巴黎
	TZ_CST2 = 15,		// 15GMT+02:00 雅典、耶路撒冷、伊斯坦布尔
	TZ_CST3 = 15,		// 15GMT+03:00 巴格达、科威特、莫斯科
	TZ_CST4 = 16,		// 16GMT+04:00 高加索标准时间
	TZ_CST5 = 17,		// 17GMT+05:00 伊斯兰堡、卡拉奇、塔什干
	TZ_CST5_30 = 18,	// 18GMT+05:30 马德拉斯、孟买、新德里
	TZ_CST6 = 19,		// 19GMT+06:00 阿拉木图、新西伯利亚、达卡
	TZ_CST7 = 20,		// 20GMT+07:00 曼谷、河内、雅加达
	TZ_CST8 = 21,		// 21GMT+08:00 北京、乌鲁木齐、新加坡
	TZ_CST9 = 22,		// 22GMT+09:00 首尔、东京、大阪、札幌
	TZ_CST10 = 23,		// 23GMT+10:00 墨尔本、悉尼、堪培拉
	TZ_CST11 = 24,		// 24GMT+11:00 马加丹、索罗门群岛
	TZ_CST12 = 25,		// 25GMT+12:00 奥克兰、惠灵顿
	TZ_CST13 = 26,		// 26GMT+13:00 努库阿洛法
};


/// 调试命令请求码  ReqDebugNvrCommandMsg
typedef enum
{
	EDebugNvrCommandCode_UnKnown = 0,	// 未知
	EDebugNvrCommandCode_RemoveDisk = 1,	// 测试删除硬盘 参数: /dev/sdb: 不再读写, 从硬盘列表删除, umount, 用于检查硬盘
	EDebugNvrCommandCode_AddDisk = 2,	// 测试添加硬盘 参数: /dev/sdb
	EDebugNvrCommandCode_Ping = 3,		// 测试ping命令 参数: str_info参数:ip, nData: 次数, <1 或者大于10时为4次
	EDebugNvrCommandCode_Netstat = 4,	// 测试netstat命令 参数: str_info参数:为空时默认为 -ntlp
	EDebugNvrCommandCode_Ls = 5,		// 测试ls命令 参数: str_info参数:为空时默认为 /,  不安全命令, 可以 执行 ls -aRl / | grep nvr, ls /tmp | top -n 1
	EDebugNvrCommandCode_GetClassCounter = 6,	// 获取硬盘信息

	EDebugNvrCommandCode_RemovePartion = 7,		// 测试删除分区, 禁用分区: 参数:str_info: /dev/sdb; int_data: 分区索引, 从0开始
	EDebugNvrCommandCode_AddPartion = 8,		// 测试添加刚删除的分区: 参数:str_info: /dev/sdb; int_data: 分区索引, 从0开始
	EDebugNvrCommandCode_ReadOnlyPartion = 9,	// 测试设置分区只读: 参数:str_info: /dev/sdb; int_data: 分区索引, 从0开始
	EDebugNvrCommandCode_FsckPartion = 10,		// 测试检查和恢复分区: 参数:str_info: /dev/sdb; int_data: 分区索引, 从0开始
	
	EDebugNvrCommandCode_GetDiskUserConfig = 11,		// 获取硬盘用户配置参数:NvrDisUserConfigkList
	EDebugNvrCommandCode_SetDiskUserConfig = 12,		// 设置硬盘配置参数: 参数: str_info: 硬盘序列号disk_sn; int_value: 0: 正常, 1: 只读
	EDebugNvrCommandCode_SetDiskPartionUserConfig = 13,	// 设置硬盘配置参数: 参数: str_info: 硬盘序列号disk_sn; int_data: 分区号, 从0开始; int_value: 0: 正常, 1: 只读

	EDebugNvrCommandCode_NvrappChannelBackup = 14,	// 通道备份, 参数: str_info: NvrMsg::BackupSrcNvrInfo的json串.
	EDebugNvrCommandCode_GetRecordUnstoreChannelList = 15,	// 获取录像通道请求码流任务列表, 返回: NvrMsg::RecordUnstoreChannelList对应的json串
	EDebugNvrCommandCode_GetRecordStoreChannelList = 16,	// 获取录像通道管理任务, 返回: NvrMsg::RecordStoreChannelList对应的json串

	// ipmi
	EDebugNvrCommandCode_GetIpmiConfigList = 17,	// 获取ipmi配置任务, 返回: NvrMsg::NvrIpmiConfig对应的json串
	EDebugNvrCommandCode_SetIpmiConfigList = 18,	// 设置ipmi配置任务, NvrMsg::NvrIpmiConfig对应的json串
	EDebugNvrCommandCode_SendIpmiCmd = 19,	// 发送ipmi命令, NvrMsg::NvrIpmiBmcParam对应的json串

	// 矩阵参数配置
	EDebugNvrCommandCode_GetMatrixConfigList = 20,	// 获取矩阵配置, 返回: NvrMsg::MatrixParamArrayConfig对应的json串
	EDebugNvrCommandCode_SetMatrixConfigList = 21,	// 设置矩阵配置, NvrMsg::MatrixParamArrayConfig对应的json串
	// ipc到nvr断网续传功能配置
	EDebugNvrCommandCode_GetBackupRecordFromIpcInfo = 22,	// 获取断网续传配置, 返回: NvrMsg::BackupRecordFromIpcInfo对应的json串
	EDebugNvrCommandCode_SetBackupRecordFromIpcInfo = 23,	// 设置断网续传配置, NvrMsg::BackupRecordFromIpcInfo对应的json串

	// 线程卡死测试
	EDebugNvrCommandCode_TestThreadBlockInfo = 24,	// 线程卡死测试配置, NvrMsg::ReqDebugThreadBlockMsg对应的json串
	// 网页安全配置
	EDebugNvrCommandCode_GetAuthLoginConfig = 25,	// 获取网页安全配置配置, 返回NvrMsg::ReqAuthLoginConfigMsg对应的json串
	EDebugNvrCommandCode_SetAuthLoginConfig = 26,	// 网页安全配置配置, NvrMsg::ReqAuthLoginConfigMsg对应的json串

}EDebugNvrCommandCode;


/// 报警通知消息  bool COnvifDllMrg::SetAlarmInfo
/// nCommand=NET_CMD_SEND_IPC_STATUS 时为 NVR下通道状态:IPC_CHANNEL_STATUS
typedef void (CALL_METHOD *AlarmMsgCallBack) (UINT nCommand, UINT nType, UINT nChannel, char* pIp, char *pAlarmInfo, LPVOID pUser);


/// 网络Nvr调试日志消息
typedef void (CALL_METHOD *NvrDebugLogCallBack) (UINT nLogTime, UINT nLogMod, UINT nLogLevel, char* pLogInfo, LPVOID pUser);


/* Audiow flag 1:远程音频，0 本地音频*/
typedef void (CALL_METHOD *DataTalkCallBack) (unsigned int VoiceDateHandle, unsigned char *VoiceDateBuffer,
	unsigned int VoiceDateSize,  BYTE AudioFlag, void *pUser);


//0 系统头 1 混合码流数据  2 最后一包数据  
typedef void (CALL_METHOD *DataPlayCallBack)( unsigned int PlayHandle,unsigned int DateType,BYTE *pBuffer,unsigned int BufferSize,LPVOID pUser);

//DateType(0:H264/1:G711A/2:G711U)
//#ifdef WIN32
//typedef void (CALL_METHOD *DataVideoAudioCallBack)(unsigned int PlayHandle, BYTE *pBuffer, unsigned int BufferSize, unsigned int DateType, SYSTEMTIME systime, unsigned int TimeSpace, LPVOID pUser);
//#else
//typedef void (CALL_METHOD *DataVideoAudioCallBack)(unsigned int PlayHandle, BYTE *pBuffer, unsigned int BufferSize, unsigned int DateType, struct tm systime, unsigned int TimeSpace, LPVOID pUser);
//#endif
typedef void (CALL_METHOD *DataVideoAudioCallBackEx)(unsigned int PlayHandle, BYTE *pBuffer, unsigned int BufferSize, unsigned int DateType, time_t systime, unsigned int TimeSpace, LPVOID pUser);

typedef void (CALL_METHOD *ExceptionCallBack)(unsigned int Type,unsigned int UserID,unsigned int Handle,void *pUser);

typedef void (CALL_METHOD *SearchNvrCallbackEx) (MCAST_DEVICE_PACKET_EX* pDevinfo, LPVOID pUser);

typedef void (CALL_METHOD *PlayBackEndCallBack)(unsigned int pbhandle,INT errorcode,void *puser);

// 调用返回
typedef void (CALL_METHOD *RemoteExecuteCallBack)(unsigned int nHandle, int nRet, const char* pJsonInfo,  LPVOID pUser);


typedef enum//编码类型
{
	E_PT_MPEG2TS = 1,  // MPEG-2 Transport Stream	
	E_PT_QCELP = 2,// QCELP audio
	E_PT_AMR = 3,// AMR audio (narrowband)
	E_PT_AMR_WB = 4,// AMR audio (wideband)
	E_PT_MPEGAUDIO = 5,// MPEG-1 or 2 audio
	E_PT_MP4AUDIO = 6,// MPEG-4 LATM audio
	E_PT_PCM8 = 7,// PCM u-law audio
	E_PT_AC3 = 8,// AC3 audio
	E_PT_MPEG4V_ES = 9,// MPEG-4 ES
	E_PT_MPVIDEO = 10,// MPEG-1 or 2 video

	E_PT_PCMA = 14,		// VSK_G711A
	E_PT_AAC = 15,		// VSK_AAC
	E_PT_PCMU = 22,		// VSK_G711U

	E_PT_H261 = 24,// H.261
	E_PT_H263 = 25,// H.263
	E_PT_H264 = 26,// H.264 视频帧类型
	E_PT_MJPEG = 27,// motion JPEG

	E_PT_G726_16 = 28,// G.726, 16 kbps
	E_PT_G726_24 = 29,// G.726, 24 kbps
	E_PT_G726_32 = 30,// G.726, 32 kbps
	E_PT_G726_40 = 31,// G.726, 40 kbps
	E_PT_H265 = 35,// H.265 视频帧类型
	// 其它: 未知类型
}VIDEO_ENCODE_TYPE_E;  // PAYLOAD_TYPE_E

/*the nalu type of H264E*///H264的帧的类型，i帧，p帧等
typedef enum
{
	FRAME_H264E_NALU_PSLICE = 1, /*PSLICE types*/
	FRAME_H264E_NALU_ISLICE = 5, /*ISLICE types*/
	FRAME_H264E_NALU_SEI = 6, /*SEI types*/
	FRAME_H264E_NALU_SPS = 7, /*SPS types*/
	FRAME_H264E_NALU_PPS = 8, /*PPS types*/
	FRAME_H264E_NALU_BUTT = 9,
	FRAME_H264E_NALU_UNKNOW = 0x00
} FRAME_H264E_NALU_TYPE_E;	// H264E_NALU_TYPE_E

typedef enum
{
	FRAME_H265_NALU_PSLICE = 0,       /*PSLICE types*/
	FRAME_H265_NALU_PSLICE_MAX = 18,  /*PSLICE types*/
	FRAME_H265_NALU_ISLICE = 19,      /*ISLICE types*/
	FRAME_H265_NALU_ISLICE_MAX = 20,  /*ISLICE types*/
	FRAME_H265_NALU_VPS = 32, /*VPS types*/
	FRAME_H265_NALU_SPS = 33, /*SPS types*/
	FRAME_H265_NALU_PPS = 34, /*PPS types*/
	FRAME_H265_NALU_SEI = 39, /*SEI types*/
	FRAME_H265_NALU_SEI_MAX = 40, /*SEI types*/
	FRAME_H265_NALU_MAX
} FRAME_H265_NALU_TYPE;	// H265_NALU_TYPE

// nEncodeType == E_PT_H264时, nFrameType类型为: FRAME_H264E_NALU_TYPE_E
// nEncodeType == E_PT_H265时, nFrameType类型为: FRAME_H265_NALU_TYPE_E
// nEncodeType为音频时, nAudioChs, nSamplingRate 两个值有效
typedef struct _VideoNoViskHeadFrameInfo
{
	unsigned char nEncodeType;	// 编码类型: VIDEO_ENCODE_TYPE_E
	unsigned char nFrameType;	// 帧类型: h264:FRAME_H264E_NALU_TYPE_E, h265:FRAME_H265_NALU_TYPE, 仅仅针对h264和h265
	unsigned short int nGapMs;		// 帧间距, 默认为40 ms
	unsigned short int nFrameRate;	// 平均帧率  默认25 帧/s
	// 音频
	unsigned char reserve1[1]; // 保留
	unsigned char nAudioChs; //1 单声道，2双声道
	unsigned int nSamplingRate;//采样率

	// 视频  vps, sps , I帧
	unsigned int nWidth;//视频宽 
	unsigned int nHeight;//视频高
	unsigned int nTimeStamp;	//时间戳 utc 时间，关键帧才有
	// 数据
	unsigned char* pBuf;
	unsigned int nFrameLen;		
	// 用户数据: 对应 JsonSdk_VideoTranspondStart 中的最后三个参数
	uint32 nPlayHandle;
	uint64 nUserData;
	void *pUser;
	unsigned char reserve2[16]; // 保留
}VideoNoViskHeadFrameInfo;

// 没有vsk消息头的视频帧回调
typedef void (CALL_METHOD *VideoNoViskHeadFrameCallBack)(const VideoNoViskHeadFrameInfo *pStream);

/// 异常触发结构, copy from SDK\NVRNET\trunk\NSDEVNET\DevMrg.h
/// type: EXCEPTION_TYPE
typedef struct
{
	ExceptionCallBack	ExcepCB;
	unsigned int Message;
	unsigned int Enable;
	uint64 nUserData;
	void *puser;
}EXCEPTION_OP;
typedef struct
{
	unsigned long Net_Addr;
	unsigned short Port;
	AlarmMsgCallBack alarmCB;
	void *puser;
}ALARM_ARG;
#if 0
typedef struct
{
	unsigned long Net_Addr;
	unsigned short Port;
	SubscriptionMsgCallBack alarmCB;
	void *puser;
}NETSDK_VSK_Subscription_ALARM_ARG;
#endif
typedef struct
{
	NvrDebugLogCallBack debugLogCB;
	void *puser;
}DEBUG_LOG_OP;

#endif
