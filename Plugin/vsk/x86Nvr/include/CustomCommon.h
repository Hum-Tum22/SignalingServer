#ifndef CUSTOMCOMMON_H
#define CUSTOMCOMMON_H

#define PACKET_HEADER_LEN    sizeof(PACKET_HEADER)
#define REPLY_HEADER_LEN     sizeof(NET_REPLY)

#define PACKET_BODY_LEN        8192 // Maximum message length definition
#define TRANVIDEODATA_LEN		512 *1024
#define PACKET_SEG_LEN          2048 //The size of the data buf received at one time
#define MAX_PLAY_BACK_NUM  4   //Maximum number of simultaneous playback per NVR
#define MAX_DISK_NUM			16 //Maximum number of hard disks

// Default intercom channel number:
#define AUDIOCHANNEL	0

#define USER_NAME_LENTH 	  21
#define USER_PWD_LENTH	  19

#define NS_NET_VERSION          0x9527

#define MAX_LOG_NUM    3000

#define EXCEPTIION_TYPE_NUM		6

#define DECODESIZE      160
#define ENCODESIZE		DECODESIZE

#define MAX_NET_USER		10

#define VT_NUMB_128         128
#define SEGMENT_OF_DAY 					6
#define DAYS_OF_WEEK                    7

#define CRUISE_BITMAP  16		//Stores the checkbox ordinal number of the split screen
#define PTZCRUISEPATH  8		//Cruise path list max 8

/*The following macro describes what each bit of link_mask in EVENT_LINK_PARA means*/
#define EVENT_LINK_ALARM_OUT		(1<<0)	/*The alarm output*/
#define EVENT_LINK_RECORD			(1<<1)	/*Linkage video*/
#define EVENT_LINK_DOME			(1<<2)	/*Linkage PTZ*/
#define EVENT_LINK_SCREEN_HINT	(1<<3)	/*The screen prompt*/
#define EVENT_LINK_NET_REPORT		(1<<4)	/*Network to upload*/
#define EVENT_LINK_EMAIL			(1<<5)	/*Send Email*/
#define EVENT_LINK_BUZZ			(1<<6)	/*Local buzz*/


#define USER_MAX_NUM				32
#define USER_GROUP_MAX_NUM 		16


#define USER_PRIVILEGE_NUM		     31 //The number of

#define ALL_PRIV					(1<<0)        //All permissions
#define	NORMAL_SET					(1<<1)	//General Settings
#define	CHANNEL_SET				(1<<2)	//Channel Settings
#define	ENCODE_SET					(1<<3)	//Encoding setting
#define	RECORD_SET					(1<<4)	//Video Settings
#define	MANUAL_RECORD				(1<<5)	//manually
#define	POOLING_SET				(1<<6)	//tour settings
#define	ABNORMAL_HANDLE			(1<<7)	//Exception handling
#define	VIDEO_DETECT			    (1<<8)	//Video detection
#define	ALARM_DETECT				(1<<9)	//Alarm set
#define	ALARM_SET_CONTROL			(1<<10)	//Alarm output setting

#define	NETWORK_SET			    (1<<11)	//Network Settings
#define	COM_SET				    	(1<<12)	//Serial port Settings
#define	PTZ_SET			      	 	(1<<13)	//PTZ setting
#define	HARD_DISK_MANAGE			(1<<14)	//Hard disk management
#define	USER_MANAGE		     	(1<<15) 	//User management
#define	SYSTEM_UPGRADE				(1<<16)	//System upgrade
#define	AUTO_MAINTENANCE    		(1<<17)	//System maintenance
#define	REC_DEFAULT				(1<<18)	//Restore the default
#define VERSION_INFO_CHECK       	(1<<19) //Version view
#define	SYSTEM_LOG_CHECK			(1<<20)	//The log view

#define NET_USER_MANAGE			(1<<21) //Network user management
#define	LOCAL_PREVIEW				(1<<22)  //Local preview
#define	LOCAL_REPLAY				(1<<23)  //Local playback
#define	REBOOT_SYS		      		(1<<24) 	//Restart the system
#define	SHUTDOWN_SYS				(1<<25)  //Shut down the system
#define	PTZ_CONTROL		       	(1<<26) 	//Yuntai control
#define	NET_PREVIEW				(1<<27)  //Network preview
#define	NET_REPLAY					(1<<28)  //Network playback
#define	SENSOR_CHECK     		 	(1<<29)  //set guard  /reset guard
#define	BACK_UP				      	(1<<30) 	//The backup


#define VIDEO_CH_NAME_LEN		64
#define VIDEO_CH_MASK_NUM		4


#define OC_MAX_X 	4095
#define OC_MAX_Y 	4095

//Used to be 8192 *32 -4 ; 
#define ONEPLAYBACKSIZE (8192 * 8 - 1024)

#define MAX_FTP_PATH			128


#define MCAST_DEVICE_FINDER_PORT	8053
#define MCAST_DEVICE_FINDER_ADDR	"224.0.0.53"
#define VK_FINDER_VERSION_ID 0x9527

#define MCAST_DEVICE_RESPONSE	0x12345
#define MCAST_DEV_GB_RESPONSE   0x12355
#define MCAST_NETWORK_RESPONSE  0x12365

#define MCAST_DEVICE_REQUESTE	0x45123
#define MCAST_DEV_GB_REQUESTE   0x55123
#define MCAST_NETWORK_REQUESTE  0x65123

typedef enum
{
	NS_QUERYINFO_SUCCESS,
	NS_QUERYINFO_NOFIND,
	NS_QUERYINFO_FINDING,
	NS_QUERYINFO_FINISH,
	NS_QUERYINFO_EXCEPTION,
	NS_QUERYINFO_MORE_BUFFER,
}QUERY_REPLY;

//Exception information callback definition
typedef enum
{
	NS_EXCEPTION_EXCHANGE = 1,
	NS_EXCEPTION_VOICETALK,
	NS_EXCEPTION_ALARM,
	NS_EXCEPTION_PLAYBACK,
	NS_EXCEPTION_DOWNLOAD,
	NS_EXCEPTION_DISKFMT,
	NS_EXCEPTION_UPGRADE,
	NS_EXCEPTION_TRANVIDO,

	NS_EXCEPTION_TYPE_COUNT,
}EXCEPTION_TYPE;

// Video detects the type
enum VIDEO_DETECT_TYPE
{
	VIDEO_DETECT_TYPE_MOTION = 0,	// Motion detecting
	VIDEO_DETECT_TYPE_LOSS = 1,		// Video loss
	VIDEO_DETECT_TYPE_BLIND = 2,	// Video shade
};

typedef enum DATATYPE {
	H264,
	G711A,
	G711U,
	PS,
	VISK_H264,
	H265,		// HEVC,	
	AAC,
	MP3,
	_OTHER_STREAM
}DATATYPE;

/// 客户端的登录权限定义 from widget_header.h  MENU_CTL 
enum EM_AUTHORITY_TYPE
{
	// 预览: 节点管理, 通道配置, 报警联动配置, 用户ipc, 设备分组, 系统版本信息, 操作日志
	em_authority_preview = 0,
	// 回放: 文件搜索,
	em_authority_playback = 1,
	em_authority_picback = 2,
	em_authority_aifaceselect = 3,
	em_authority_aiface = 4,
	em_authority_download = 5,
	em_authority_ptz = 6,
	em_authority_capture = 7,
	em_authority_recoder = 8,
	em_authority_group = 9,
	// ipc管理
	em_authority_device = 10,
	em_authority_tvwall = 11,
	em_authority_alarm = 12,
	em_authority_system_config = 13,
	em_authority_emap = 14,
	em_authority_voice_talk = 15,
	em_authority_system_lock = 16,
	em_authority_network = 17,
	//em_authority_port = 18,
	em_authority_restore_settings = 19,
	em_authority_reboot = 20,
	em_authority_shutdown = 21,
	// 远程配置:ipc管理通道搜索, 获取nvr时间,ntp时间管理,设置nvr基本配置, 电话配置,gb28181,监管平台,系统日志和调试信息,代理配置,人脸配置
	// 视频通道配置,编码设置,录像设置,预览设置,异常设置,报警检测,报警输出设置,硬盘管理,日志查看,用户组和用户管理
	em_authority_config = 22,
	em_authority_system_quit = 23,	//退出
	em_authority_networkcard_config
};


enum EM_Node_Type
{
	em_node_unknown = 0,//未知类型, 暂不使用	
	em_node_unit = 1,//组织机构	
	em_node_nvr = 2,//NVR	
	em_node_dec = 3,//DEC
	em_node_dvr = 4,//DVR
	em_node_ipc = 5,//IPC			独立ipc
	em_node_child_ipc = 6,//IPC		subIPC
	em_node_camera = 7,//摄像机		没有使用到
	em_node_probe = 8,//探头
	em_node_ms = 9,//硬媒体
	em_node_enc = 10,//编码器
	em_node_group = 11,//分组
	em_node_alarmhost = 12,//报警主机
	em_node_alarmhost_subsystem = 13,//报警主机子系统
	em_node_alarmhost_zone = 14,//报警主机防区
	em_node_hirsch_door = 15,//赫氏门禁
	em_node_cwg_zone = 16,//丛文防区
	em_node_anson_controller = 17,//安森门禁控制器
	em_node_anson_door = 18,//安森门禁门卡
	em_node_patrol_device = 19,//巡更棒
	em_node_honeywell_alarm_zone = 20,//霍尼韦尔报警防区 --> 协议: em_dev_honeywell_victrix, em_dev_honeywelll_ip2000
	em_node_compressor = 21,// 编码压缩仪
	em_node_saat_door = 22,//国密门禁
	em_up_node_cms = 23,//cms上级节点
	em_node_bx_hostalarm_zone = 24,//宝学报警主机  --> 协议: em_dev_bx_alarmhost
	em_node_gato_alarmhost = 25,//广拓报警主机
	em_node_gato_alarmhost_defence_area = 26,//广拓报警主机防区
	em_node_honeywell_victrix_zone = 27,//霍尼韦尔victrix报警防区  --> 协议: em_dev_honeywell_victrix
	em_down_node_cms = 28,//下级cms节点
	em_node_hls = 29,     //hls
	em_node_cwg_platform = 30,	//从文报警平台
	em_node_infinova_keyboard_dec_screen = 31, //英飞拓键盘序号和解码器屏号的对应

	em_node_qianfan = 34,		//千帆门禁系统
	em_node_qianfan_controller = 35,	//千帆门禁控制器
	em_node_qianfan_door = 36,	//千帆门禁的门
	em_node_honeywell_vista = 37,	//霍尼韦尔Vista报警主机


	// 客户端需要增加宏: CLIENT_TYPE_PELCO
#if defined(CMS_TYPE_PELCO) || defined(NVR_TYPE_PELCO) || defined(CLIENT_TYPE_PELCO)
	em_node_boshi_alarmhost = 32,	//博世报警主机
	em_node_honeywell_vista_zone = 33, //霍尼韦尔Vista报警防区

	//em_node_dahua_dss = 38,      // 大华dss平台 : 先认为是一种不同协议的nvr
	//em_node_dahua_dss_nvr = 39,      // 大华dss平台下的各种nvr, 不登录!
#else
	em_node_dahua_dss  = 32,      // 大华dss平台 : 先认为是一种不同协议的nvr
	em_node_dahua_dss_nvr  = 33,      // 大华dss平台下的各种nvr, 不登录!

	em_node_boshi_alarmhost = 38,	//博世报警主机
	em_node_honeywell_vista_zone = 39, //霍尼韦尔Vista报警防区
#endif
	
	// 后面从40开始



	em_node_type_max,	// 节点类型数量
};

enum EM_Device_Protocol_Type
{
	em_dev_protocol_type_unknown = 0,

	em_dev_rtsp = 2,
	em_dev_vsk = 3,	//VMS
	em_dev_onvif = 4,
	em_dev_ip2000 = 5,
	em_dev_dahua = 6,
	em_dev_hik = 7,			// 海康
	//em_dev_localfile = 8,
	em_dev_yushi = 9,
	em_dev_yushi_ex = 10,
	em_dev_hh = 11,
	em_dev_xhy = 12,
	em_dev_samsung = 13,
	em_dev_anson = 14,
	em_dev_patrol = 15,
	em_dev_honeywelll_ip2000 = 16,
	em_dev_vsk_json = 17,			// 增加json类型的nvr协议
	em_dev_cms_json = 18,			// 增加json类型的cms协议
	em_dev_honeywell_victrix = 19,
	em_dev_bx_alarmhost = 20,
	em_dev_guangtuo = 21,                 //广拓报警主机
	em_dev_sip28181 = 22,				// sip协议
	em_dev_hls = 23,
	em_dev_boshi_alarmhost = 24,		// 博世报警主机, 暂未用到!

	em_dev_pelco = 38,					// 派尔高协议 用的是 英飞拓协议
	em_dev_infinovia_thermal = 39,			// 英飞拓 热成像测温相机
	em_dev_infinovia = 40,					// 英飞拓协议
	em_dev_hanhui = 41,						// 瀚辉协议
	em_dev_hms = 42,						// HMS协议
	em_dev_jvlong = 43,					   // 巨龙新协议 UNS_DEVICE_JVLONG
	em_dev_gateway = 44,					// 网关使用
	em_dev_langtao = 45,					// 浪涛
	em_dev_jiguang  = 46,                   // 集光nvr
	em_dev_dahua_dss  = 47,                 // 大华dss平台
	em_dev_tiandy = 48,                    // 天地伟业nvr
	em_qianfan_door = 49,			        //千帆门禁
};

enum EM_CLIENT_ALARM_TYPE
{
	em_nvr_alarm = 0,
	em_ipc_alarm = 1,
	em_ip2000_alarm = 2,
	em_cwg_alarm = 3,
	em_hirsch_alarm = 4,
	em_hik_alarm = 5,
	em_yushi_alarm = 6,
	em_yushi_imos_alarm = 7,
	em_anson_alarm = 8,
	em_patrol_system_alarm = 9,
	em_ai_face_alarm = 10,
	em_log_bx_alarm = 11,
	em_gato_alarm = 12,
	em_cw_platform_alarm = 13,				// 丛文平台报警: 报警消息strid为 ipc的device node的str_id
	em_cw_platform_alarm_alarm_node = 14,	// 丛文平台报警: 报警消息strid为报警主机配置对象的device node的str_id
	em_devicemrg_log = 15,
	em_tvwall_log = 16,
	em_sytemmrg_log = 17,
	em_log_smart_box_alarm = 18,
	em_boshi_alarm = 19,
	em_qianfan_alarm = 20,
	em_honeywell_vista_alarm = 21,
	em_preview_log = 22,
	em_playback_log = 23,
};

/*
licence请求串格式为: {"type":1,"expire_time":1629200280,"licence_key":"006c92c11043f4281a0015d9f2a0a38c","enable":1}
type不存在或者0: 使用老的证书对.
1: 智慧商超webserver
2. arm nvr
3. x86 nvr		制作nvr启动盘时, mpclient调用, server端直接返回 新的证书对加密的lic文件, 最好在lic中添加 type=3
4. x86 cms使用
5. qt客户端
*/

// licence文件类型
typedef enum _LicenceFileType {
	LicenceFileType_Old = 0,			// type字段不存在或者值为0:	
	LicenceFileType_PlaceWebServer = 1,	// 智慧商超
	LicenceFileType_ArmNvr = 2,			// arm nvr
	LicenceFileType_X86Nvr = 3,			// x86 nvr
	LicenceFileType_X86Cms = 4,			// x86 cms
	LicenceFileType_QtClient = 5,		// 客户端vms7000单机有licence版本
}LicenceFileType;


/* search NVR */
/* start */

// typedef struct tip_mreq {
// 	IN_ADDR imr_multiaddr;  // IP multicast address of group.
// 	IN_ADDR imr_interface;  // Local IP address of interface.
// } T_IP_MREQ, *PT_IP_MREQ;

typedef struct
{
	unsigned char video_chns;			/*IP Video channels*/
	unsigned char audio_chns;			/*Audio channel number*/
	unsigned char alarmin_chns;		/*Alarm input channel number*/
	unsigned char alarmout_chns;		/*Alarm output channel number*/
	unsigned char serial_num;			/*Number of external serial ports (excluding debugging serial ports and front panel serial ports)*/
	unsigned char dec_pic_size;		/*Maximum decoding resolution supported by the device DECODE_SIZE_D1...*/
	unsigned short vga_resolution; 		/*VGA supported resolution，bitmap bit0: PIXEL_720P60  bit1: PIXEL_1080P50  ...*/
	unsigned int version;			/*Software version number*/
	unsigned char reserved[12];
	unsigned char release_date[32];	/*The release date*/
} FNVR_SYS_INFO;

typedef struct
{
	unsigned int dspversion;         /*DSP Driver software version number*/
}FNVR_SYS_INFO_DSP;

typedef struct
{
	unsigned int version;
	unsigned int cmd;
	unsigned int session;
	unsigned int deviceip;
	unsigned int  deviceport;
	unsigned int	devicemask;
	unsigned int  devicegate;
	unsigned  char macaddr[8];
	unsigned int  machtype;
	FNVR_SYS_INFO	systeminfo;
	FNVR_SYS_INFO_DSP dspinfo;
	unsigned int   httpport;
	unsigned char	hardwaremodel[16];
	unsigned int   bdevicemodel;
	unsigned char 	resev[40];
	unsigned char 	devcemodelstr[64];
}MCAST_DEVICE_PACKET_EX;

#ifndef	_WIN32
typedef long                BOOL;
typedef unsigned short      WORD;
//typedef unsigned long		DWORD;
//typedef unsigned long		ULONG;
//typedef unsigned long long	UINT64;

typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
#endif

#endif
