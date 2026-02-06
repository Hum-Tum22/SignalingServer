#pragma once
// 定义错误码

//异常信息回调定义
typedef enum
{
	ERR_OK						= 0,	///< 没有错误

	ERR_PARSER_JSON				= 2,	///< Parser Json数据 出错
	ERR_PARSER_PARAM			= 3,	///< Parser Json数据 出错
	ERR_NO_METHOD				= 5,	///< 没有Method请求方法
	ERR_METHOD_NOT_SUPPORTED	= 6,	///< 不支持的Method请求方法
	ERR_NO_AUTHORIZATION		= 7,	///< 请求时, 没有AUTHORIZATION信息
	ERR_PARAM_ERROR				= 8,	///< 请求时, 参数错误
	ERR_NO_PRIVILEGE			= 9,	///< 没有权限!
	ERR_GET_CONFIG_FAILED		= 10,	///< 获取配置失败
	ERR_LOGIN_IPC_FAILED		= 11,	///< 登录IPC时, loginDevice失败
	ERR_NVR_ENABLE_FAILED		= 12,	///< Nvr没有启用失败: 检查硬盘序列号失败
	ERR_FIND_IPC_FAILED			= 13,	///< 查找ipc失败,没有配置该IPC
	ERR_ENABLE_AI_FACE_FAILED	= 14,	///< Nvr没有启用图片对比功能
	ERR_SET_MULTI_IP_IPC		= 15,	///< ipc添加多个ip失败
	ERR_NEED_MORE_BUFFER		= 16,	///< 提供的缓存不足
	ERR_GET_ERR_PARA			= 17,	///< 获取失败
	ERR_ADD_IPC_TOO_MANY		= 18,	///< ipc添加数量超过最大限制
	ERR_OPTION_NOT_SUPPORTED	= 19,	///< 不支持的操作

	ERR_NO_MEMORY				= 1000,	///< 内存不足
	ERR_UNKONWN_SERVER_ERROR	= 1001,	///< 服务端未知错误
	ERR_LICENCE_EXPIRED			= 1002,	///< licence授权已经过期
	ERR_LICENCE_MAX_IPC_COUNT	= 1003,	///< licence最大ipc数量限制
	ERR_LICENCE_GET_DISK_SEQ	= 1004,	///< 验证licence获取磁盘序号失败
	ERR_LICENCE_CHECK_DISK_SEQ	= 1005,	///< 验证licence 磁盘序号失败


	ERR_CONNECT_ERROR			= 2000,	///< 网络错误: 连接失败
	ERR_SEND_ERROR				= 2001,	///< 网络错误: 发送失败
	ERR_LISTEN_ERROR			= 2002,	///< 网络错误: 帧听失败
	ERR_CREATE_SOCKET_ERROR		= 2003,	///< 网络错误: 创建socket失败
	ERR_INIT_SOCKET_ERROR		= 2004,	///< 网络错误: Init socket失败
	ERR_ADD_CONNECTION_ERROR	= 2005,	///< 网络错误: 内部错误, 添加连接失败
	ERR_PARSE_FILE_ERROR		= 2006,	///< 解析文件失败
	ERR_CONNECTION_CLOSE_ERROR  = 2007,	///< 网络错误: 连接已经断开
	ERR_IN_LOGIN_PROGRESS_ERROR = 2008,	///< 正在登录.

	ERR_NO_LOGIN_ID				= 3001,	///< 操作失败,没有对应的登录ID, 或者链接已经断开
	ERR_LOCAL_ERR_PARA			= 3002,	///< 操作失败,参数不正确
	ERR_FILE_SEARCH_FINISH		= 3003,	///< 文件搜索结束
	ERR_REQ_NO_RESULT			= 3004,	///< 操作成功, 但是没有数据
	ERR_REQ_PRIVIEW_FAILED		= 3005,	///< 请求预览视频失败
	// 协议专用
	ERR_CONFIG_GET_NET_PARAM	= 5001,	///< 读取NVR_NETWORK_PARA失败
	ERR_CONFIG_SET_TIME			= 5011,	///< 设置nvr时间失败
	ERR_CONFIG_LIST_USER		= 5101,	///< 读取用户列表失败
	ERR_CONFIG_LIST_USER_GROUP			= 5102,	///< 读取用户组列表失败
	ERR_CONFIG_ADD_USER_EXISTED			= 5103,	///< 添加用户失败, 已经存在该用户
	ERR_CONFIG_MODIFY_USER_NOT_EXISTED	= 5104,	///< 修改的用户组不存在
	// ERR_CONFIG_DEL_USER_NOT_EXISTED		= 5105,	///< 删除用户组失败, 不存在该用户组
	ERR_CONFIG_ADD_USER_GROUP_EXISTED	= 5106,	///< 添加用户组失败, 已经存在该用户组
	ERR_CONFIG_MODIFY_USER_GROUP_NOT_EXISTED = 5107,	///< 修改用户组失败, 已经存在该用户
	// ERR_CONFIG_DEL_USER_GROUP_NOT_EXISTED	= 5108,	///< 删除用户组失败, 不存在该用户组
	ERR_CONFIG_DEL_USER_GROUP				= 5109,	///< 删除用户组失败, 分组存在用户
	ERR_CONFIG_ADD_USER_NO_GROUP			= 5110,	///< 添加用户失败, 没有对应的分组
	
	ERR_CONFIG_LIST_EXCEPTION				= 5120,	///< 获取异常列表失败
	ERR_CONFIG_SET_EXCEPTION_INFO			= 5121,	///< 设置异常信息失败
	ERR_CONFIG_GET_ALARM_IN_PARAM			= 5122,	///< 获取报警输入信息失败
	ERR_CONFIG_SET_ALARM_IN_PARAM			= 5123,	///< 设置报警输入信息失败

	ERR_CONFIG_GET_VIDEO_CHANNEL_PARAM		= 5130,	///< 获取视频通道信息失败
	ERR_CONFIG_SET_VIDEO_CHANNEL_PARAM		= 5131,	///< 设置视频通道信息失败
	ERR_CONFIG_GET_VIDEO_ENCODER_PARAM		= 5140,	///< 获取视频编码信息失败
	ERR_CONFIG_SET_VIDEO_ENCODER_PARAM		= 5141,	///< 设置视频编码信息失败
	ERR_CONFIG_GET_VIDEO_SCHEDULE_PARAM		= 5150,	///< 获取录像计划配置失败
	ERR_CONFIG_SET_VIDEO_SCHEDULE_PARAM		= 5151,	///< 设置录像计划配置失败
	// ----------------------------------------------------------------------
	ERR_PARSE_PARAM_ERROR					= 6000,	///< 解析请求参数失败
	ERR_PARSE_PROTOBUF_ERROR				= 6001,	///< 解析protobuf失败
	ERR_RECV_TIMEOUT						= 6002,	///< 接收超时
	ERR_CONNECT_TIMEOUT						= 6003,	///< 连接超时
	ERR_SERVER_WAIT_LOGIN_TIMEOUT			= 6004,	///< 连接成功后, 服务端等待登录消息超时
	ERR_SERVER_WAIT_HEART_BEAT_TIMEOUT		= 6005,	///< 连接成功后, 服务端等待心跳消息超时

	ERR_SERILIZE_PROTOBUF_ERROR				= 6006,	///< 保存protobuf失败
	ERR_SEND_FILE_TIMEOUT					= 6007,	///< 文件发送超时
	// ----------------------------------------------------------------------
	ERR_REVERSE_PROXY_TARGET_CMD_NOT_CONNECTED		= 6010,	///< 反向连接: 命令端口目标nvr没有连接成功.
	ERR_REVERSE_PROXY_TARGET_MEDIA_NOT_CONNECTED	= 6011,	///< 反向连接: 数据端口目标nvr没有连接成功.

	// ----------------------------------------------------------------------
	ERR_FILE_READ_OPEN_ERROR				= 6020,	///< 只读打开文件失败
	ERR_FILE_READ_OPEN_EXCEPTED				= 6021,	///< 只读打开文件异常
	ERR_FILE_SIZE_ERROR						= 6022,	///< 文件长度不正确
	ERR_FILE_WRITE_OPEN_ERROR				= 6023,	///< 写入打开文件失败
	ERR_FILE_WRITE_OPEN_EXCEPTED			= 6024,	///< 写入打开文件异常
	ERR_FILE_READ_FAILED					= 6025,	///< 读取文件失败
	ERR_FILE_WRITE_FAILED					= 6026,	///< 写入文件失败
	ERR_FILE_WRITE_EXCEPTED					= 6027,	///< 写入文件异常
	ERR_FILE_SEND_FILE_FAILED				= 6028,	///< 发送文件出错
	ERR_FILE_TRUNCATED_FAILED				= 6029,	///< 截断文件出错

	ERR_GET_THREAD_DATA_FAILED				= 6030,	///< 获取私有数据失败, 没有对应的会话

	ERR_FILE_DIR_NOT_EXISTED				= 6031,	///< 文件目录不存在
	ERR_FILE_NO_DIR_PERMISSION				= 6032,	///< 没有操作指定目录的权限
	ERR_FILE_SEEK_ERROR						= 6033,	///< 文件seek失败
	ERR_FILE_REMOVE_FAILED					= 6034,	///< 删除文件失败
	ERR_FILE_EMPTY							= 6035,	///< 文件为空

	ERR_FLIE_GET_WRITE_POS_ERROR			= 6040,	///< 接收数据时, 数据偏移量不是需要的数据


	ERR_PLAY_BACK_FAILED					= 6041, ///< 文件回放失败
	ERR_PLAY_BACK_CONTROL_FAILED			= 6042, ///< 文件回放控制失败
	ERR_PLAY_BACK_SET_POS_FAILED			= 6043, ///< 文件回放修改回放位置失败
	// 
	ERR_NETWORK_SET_IP_FAILED				= 6044,	///< 设置网络IP失败
	ERR_NETWORK_SET_ROUTE_FAILED			= 6045,	///< 设置网络路由失败

	/// windows下的errno对应的错误码
	ERR_FILE_OP_EPERM						= 6050,	///< 不允许执行该操作
	ERR_FILE_OP_ENOENT = 6051,	///< 不允许执行该操作  文件不存在
	ERR_FILE_OP_EIO = 6052,	///< I/O 错误
	ERR_FILE_OP_ENXIO = 6053,	///< 没有此设备或地址
	ERR_FILE_OP_ENOEXEC = 6054,	///< 执行格式错误
	ERR_FILE_OP_EBADF = 6055,	///< 文件编号错误
	ERR_FILE_OP_ENOMEM = 6056,	///< 没有足够内存
	ERR_FILE_OP_EACCES = 6057,	///< 权限被拒绝
	ERR_FILE_OP_EFAULT = 6058,	///< 地址错误
	ERR_FILE_OP_EBUSY = 6059,	///< 设备或资源忙碌
	ERR_FILE_OP_EFBIG = 6060,	///< 文件太大
	ERR_FILE_OP_ENOSPC = 6061,	///< 设备上没有剩余空间
	ERR_FILE_OP_EROFS = 6062,	///< 文件系统只读
	ERR_FILE_OP_EDEADLK = 6063,	///< 文件操作死锁
	ERR_FILE_OP_ENAMETOOLONG = 6064,	///< 文件名太长
	ERR_FILE_OP_EILSEQ = 6065,	///< 非法字节序列
	ERR_FILE_OP_STRUNCATE = 6066,	///< 字符串被截断
	ERR_FILE_OP_EEXIST = 6067,	///< 文件已存在
	ERR_FILE_TYPE_RROR = 6068,	///< 文件类型错误: 比如不是指定sql文件
	ERR_FILE_WITH_PATH = 6069,	///< 文件包含路径错误,不应该包含路径
	ERR_FILE_UNZIP_LIST = 6070,	///< unzip时获取文件列表失败
	ERR_FILE_UNZIP_FILE = 6071,	///< unzip时解压文件失败

	// user manager
	ERR_GET_USER_NOT_EXISTED = 6100,	///< 获取的用户不存在
	ERR_GET_USER_NAME_EXISTED = 6101,	///< 用户名已经存在
	ERR_GET_USER_GROUP_NOT_EXISTED = 6102,	///< 获取的用户组不存在
	ERR_GET_USER_GROUP_NAME_EXISTED = 6103,	///< 用户组名已经存在
	ERR_ADD_USER_ERROR_GROUP_ID = 6104,	///< 添加用户失败, 没有对应的用户组
	ERR_DELETE_USER_GROUP_IN_USE = 6105,	///< 删除用户组失败, 用户组内包含用户!
	ERR_USER_PASSWORD_LENGTH = 6106,		///< 最小密码长度不正确
	ERR_USER_PASSWORD_COMPLEX = 6107,		///< 必须包含数字和字母判断不正确
	ERR_USER_LOGIN_TIME_WAIT = 6108,		///< 登录限制, 等待1分钟后重试
	ERR_DELETE_USER_SELF = 6109,			///< 删除用户失败, 不能删除用户自己


	ERR_USER_LOGIN_PASSWORD_ERROR = 6110,	///< 用户登录失败: 密码错误
	ERR_USER_NOT_LOGIN_ERROR = 6111,	///< 用户没有登录, 没有权限, 操作失败
	ERR_USER_NO_RIGHT = 6112,	///< 用户没有权限, 操作失败
	ERR_USER_RESTORE_FAILED = 6113,	///< 恢复默认用户信息失败
	ERR_USER_LOGIN_ERROR = 6114,	///< 用户登录失败

	// uri
	ERR_CONFIG_MAX_IPC_LIMITED_ERROR = 6119,	///< 添加IPC, 达到最大数量限制
	ERR_GET_CONFIG_NOT_INIT_CONFIG = 6120,	///< 内部错误, 没有对应的配置
	ERR_GET_URI_NOT_EXISTED = 6121,	///< 指定的uri不存在
	ERR_GET_CONFIG_RESTORE_FAILED = 6122,	///< 恢复配置失败
	ERR_CONFIG_CHANNEL_ID_ERROR = 6122,	///< 使用的通道id错误.
	ERR_CONFIG_DISK_GROUP_ID = 6123,	///< 磁盘分组号错误
	ERR_CONFIG_DISK_ID_ERROR = 6124,	///< 使用的硬盘id错误.
	ERR_CONFIG_RAID_ID_ERROR = 6125,	///< 使用的Raidid错误.
	ERR_CONFIG_DISK_GROUP_NOT_RAID = 6126,	///< 磁盘分组不是raid分组
	ERR_CONFIG_ETH_ID_ERROR = 6127,	///< 使用的网卡id错误.
	ERR_CONFIG_FUNCTION_NOT_SUPPORT = 6128,	///< 不支持开启指定的功能.
	ERR_CONFIG_ALARM_ID_ERROR = 6129,	///< 使用的报警通道id错误.
	ERR_CONFIG_EXCEPTION_ID_ERROR = 6130,	///< 使用的异常id错误.
	ERR_CONFIG_REQ_ID_ERROR = 6131,	///< 使用的id错误.
	ERR_CONFIG_PARTITION_ID_ERROR = 6132,	///< 使用的分区id错误.

	// 语音对讲
	ERR_START_TALK_FAILED = 6139,	///< 开启对讲失败
	ERR_START_RECORDE_SOUND = 6140,	///< 调用开启声音采集失败 StartRecodeSound failed
	ERR_START_PLAY_SOUND = 6141,	///< 调用播放音频失败 G711_StartPlaySound failed
	// 配置
	ERR_CONFIG_SAVE_CAPABILITY = 6142,	///< 保存能力集配置失败 CNvrCapabilityConfig
	ERR_CONFIG_SAVE_NVR_MODEL = 6143,	///< 保存Nvr型号配置失败 CNvrModelInfoConfig
	ERR_CONFIG_SAVE_SERVER_URI	= 6144,	///< 保存Ipc配置失败 CNvrServerUriConfig
	ERR_CONFIG_SAVE_NVR_COMMON = 6145,	///< 保存Nvr基本配置失败 CNvrCommonParamConfig
	ERR_CONFIG_SAVE_ALARM_CONFIG = 6146,	///< 保存报警配置失败 CNvrAlarmConfig
	ERR_CONFIG_SAVE_EXCEPTION_DETECT_PARAM = 6147,	///< 保存异常检测配置失败 CNvrExceptionConfig
	ERR_CONFIG_SAVE_HMS_IPC_PARAM = 6148,	///< 保存Hms的Ipc配置失败 CNvrHmsIpcParamConfig
	ERR_CONFIG_SAVE_IPC_SERVER_URI = 6149,	///< 保存Ipc配置失败 CNvrIpcServerUriParamConfig
	ERR_CONFIG_SAVE_MAINTAIN_PARAM = 6150,	///< 保存维护信息配置失败 CNvrMaintainParamConfig
	ERR_CONFIG_SAVE_NETWORK_PARAM = 6151,	///< 保存网络配置失败 CNvrNetworkConfig
	ERR_CONFIG_SAVE_PTZ_CRUISE_PARAM = 6152,	///< 保存Ptz巡航配置失败 CNvrPtzCruiseConfig
	ERR_CONFIG_SAVE_RECORD_SCHEDULE_PARAM = 6153,	///< 保存录像计划配置失败 CNvrRecordScheduleConfig
	ERR_CONFIG_SAVE_STORAGE_PARAM = 6154,	///< 保存存储配置失败 CNvrStorageConfig
	ERR_CONFIG_SAVE_USER_PARAM = 6155,	///< 保存用户配置失败 CNvrUserConfig
	ERR_CONFIG_SAVE_VIDEO_CHANNEL_PARAM = 6156,	///< 保存视频通道配置失败 CNvrVideoChannelConfig
	ERR_CONFIG_SAVE_VIDEO_ENCODER_PARAM = 6157,	///< 保存视频编码配置失败 CNvrVideoEncoderConfig
	ERR_CONFIG_SAVE_VIDEO_DETECT_PARAM = 6158,	///< 保存视频检测配置失败 CNvrVideolDetectConfig
	ERR_CONFIG_SAVE_VERSION = 6159,	///< 保存Nvr版本配置失败 NvrVersionInfo

	// upgrade
	ERR_UPGRADE_NVR_STATE = 6200,	///< 升级nvr状态错误
	ERR_UPGRADE_NVR_VERSION = 6201,	///< 升级nvr版本错误, 目标不是符合的升级版本
	ERR_UPGRADE_NVR_BACKUP_INVALID = 6202,	///< nvr备份无效, 没有或者无法进入该目录
	ERR_UPGRADE_ROLLBACK_FAILED = 6203,	///< nvr回滚失败
	//ai模块operation
	ERR_FIND_AIUSER_FAILED = 6250,	///< AI用户不存在
	ERR_FEARTURE_EXTRACT_FAILED = 6251,	///< 特征码提取失败
	ERR_DATABASE_OPERATION_FAILED = 6252,	///< 数据库操作失败

	//{{ 为人脸数据库操作错误预留  ai_errorcode
	ERR_AI_USER_DB_ERROR_START = 7000,

	ERR_AI_USER_DB_ERROR_END = 7199,
	//}} 为人脸数据库操作错误预留  ai_errorcode
	//{{
	ERR_FORMAT_DISK_FAILED_START	= 7200,		///< 磁盘格式化失败 + FILESYSTERM_ERRORCODE

	ERR_FORMAT_DISK_FAILED_END		= 7399,		///< 磁盘格式化 保留
	//}}

	ERR_IV_DB_NOT_INIT			= 8000,		///< 数据库没有初始化
	ERR_IV_DB_NOT_CONNECTED		= 8001,		///< 数据库连接失败
	ERR_IV_DB_EXECUTE			= 8002,		///< 数据库操作失败
	ERR_IV_DB_CREATE_TABLE		= 8003,		///< 创建数据表失败
	ERR_IV_DB_INSERT			= 8004,		///< 插入数据失败
	ERR_IV_DB_CONNECT_FAILED	= 8005,		///< 数据库连接失败
	ERR_IV_DB_QUERY_EMPTY		= 8006,		///< 数据库查询结果为空
	ERR_IV_DB_QUERY_FAIL		= 8007,		///< 数据库查询结果错误
	ERR_IV_DB_RECORD_EXISTED	= 8008,		///< 插入失败, 数据已经存在
	ERR_IV_DB_REF_RECORD_NOT_EXISTED	= 8009,		///< 插入失败, 引用的外键不存在
	ERR_IV_DB_NODE_STR_ID_EXISTED		= 8010,		///< 插入失败, 设备节点表: id已经存在
	ERR_IV_DB_NODE_NAME_EXISTED			= 8011,		///< 插入失败, 记录名称已经存在
	ERR_IV_DB_NODE_FID_NOT_EXISTED		= 8012,		///< 插入失败, 设备节点的父节点不存在
	ERR_IV_DB_PARAM_EMPTY				= 8013,		///< 插入失败, 必填参数为空
	ERR_IV_DB_NODE_CHANNEL_EXISTED		= 8014,		///< 插入失败, 设备节点的父节点已经存在指定通道的ipc
	ERR_IV_DB_NODE_ID_NOT_EXISTED		= 8015,		///< 设备节点的不存在
	ERR_IV_DB_DEL_FAILED				= 8016,		///< 删除失败
	ERR_IV_DB_RESTORE_FACTORY			= 8017,		///< 恢复出厂失败
	ERR_IV_DB_DB28181_ID_EXISTED		= 8018,		///< 插入失败,国标id已存在
	ERR_IV_DB_DB28181_ID_NOTNULL		= 8019,		///< 插入失败,国标id不能为空
	ERR_IV_DB_ROOT_NODE_FID_TYPE_ERROR	= 8020,		///< 顶级节点的父节点类型不是组织结构类型错误
	ERR_IV_DB_NODE_IP_PORT_EXISTED		= 8021,		///< 插入失败, 设备节点表: ip和port都相同的节点已经存在
	ERR_IV_DB_NODE_NAME_PARAM_ERR		= 8022,		///< 插入失败, 设备名称错误
	ERR_IV_ADMIN_USER_NOT_NOTIFY		= 8023,		///< admin用户不能修改
	ERR_IV_FATHER_NODE_TYPE_ERR			= 8024,		///< 父节点类型错误
	ERR_IV_DB_DB28181_ID_LEN_ERR		= 8025,		///< 插入失败,国标id长度错误

	///raid 操作错误码
	ERR_RAID_CREATEHOTSPARE_FAILED		= 8200,		///< 创建热备盘失败
	EER_RAID_DEL_HOTSPARE_FAILED		= 8201,		///< 删除热备盘失败
	EER_RAID_START_LOCATE_FAILED		= 8202,		///< 开启locate失败
	EER_RAID_STOP_LOCATE_FAILED			= 8203,		///< 停止locate失败
	ERR_RAID_SET_ALARM_FAILED			= 8204,		///< 设置raid报警失败
	ERR_RAID_SET_CACHE_FAILED			= 8205,		///< 设置raid cache失败
	ERR_RAID_SET_IO_POLICY_FAILED		= 8206,		///< 设置io策略失败
	ERR_RAID_NO_RAID_CARD				= 8207,		///< 没有raid卡错误
	ERR_RAID_EXEC_CMD_FAILED			= 8208,		///< 执行cli命令失败
	ERR_RAID_CMD_UNSUPPORT				= 8209,		///< 暂不支持
	ERR_RAID_STATUSERROR				= 8210,		///< raid 状态错误
	ERR_RAID_CREATE_DISK_NOT_ENOUGH		= 8211,		///< raid 创建逻辑硬盘的硬盘数量不足
	ERR_RAID_TYPE_ERROR					= 8212,		///< raid 创建逻辑硬盘的raid类型错误, 暂不支持除raid5和raid6以外的类型
	ERR_RAID_DELETE_DISK_START			= 8213,		///< raid 开始删除逻辑磁盘, 结果使用报警通知:ALARM_TYPE_RAID
	ERR_RAID_DELETE_DISK_IN_PROGRESS	= 8214,		///< raid 删除逻辑磁盘失败, 正在进行删除操作

	ERR_DOWNLOAD_PROCESSED				= 8500,		///< 下载申请已处理
	// 线程
	ERR_TRHEAD_IS_RUNNING				= 8510,		///< 线程正在运行, 等待一个任务结束后才能继续执行同类任务
	ERR_SAME_TASK_IS_ADDED				= 8511,		///< 相同任务已经添加

}ERR_CODE;

typedef enum
{
	WE_OK					= 0,	// 没有错误
	WE_NO_DATA				= 1,	// 没有post Json数据
	WE_PARSER_JSON			= 2,	// Parser Json数据 出错
	WE_PARSER_PARAM			= 3,	// Parser Json数据 出错
	WE_NO_LOGIN_ID			= 4,	// 没有登录
	WE_NO_METHOD			= 5,	// 没有Method请求方法
	WE_METHOD_NOT_SUPPORTED	= 6,	// 不支持的Method请求方法
	WE_NO_AUTHORIZATION		= 7,	// 请求时, 没有AUTHORIZATION信息
	WE_PARAM_ERROR			= 8,	// 请求时, 参数错误
	WE_NO_PRIVILEGE			= 9,	// 没有权限!
	WE_GET_CONFIG_FAILED	= 10,	// 获取配置失败

	// 协议专用
	WE_CONFIG_GET_NET_PARAM = 5001,	// 读取NVR_NETWORK_PARA失败
	WE_CONFIG_SET_TIME		= 5011,	// 设置nvr时间失败
	WE_CONFIG_LIST_USER		= 5101,	// 读取用户列表失败
	WE_CONFIG_LIST_USER_GROUP			= 5102,	// 读取用户组列表失败
	WE_CONFIG_ADD_USER_EXISTED			= 5103,	// 添加用户失败, 已经存在该用户
	WE_CONFIG_MODIFY_USER_NOT_EXISTED	= 5104,	// 修改用户失败, 已经存在该用户
	//WE_CONFIG_DEL_USER_NOT_EXISTED		= 5105,	// 删除用户失败, 不存在该用户
	WE_CONFIG_ADD_USER_GROUP_EXISTED	= 5106,	// 添加用户组失败, 已经存在该用户
	WE_CONFIG_MODIFY_USER_GROUP_NOT_EXISTED = 5107,	// 修改用户组失败, 已经存在该用户
	//WE_CONFIG_DEL_USER_GROUP_NOT_EXISTED	= 5108,	// 删除用户组失败, 不存在该用户
	WE_CONFIG_DEL_USER_GROUP				= 5109,	// 删除用户组失败, 分组存在用户
	WE_CONFIG_ADD_USER_NO_GROUP				= 5110,	// 添加用户失败, 没有对应的分组
	
	WE_CONFIG_LIST_EXCEPTION				= 5120,	// 获取异常列表失败
	WE_CONFIG_SET_EXCEPTION_INFO			= 5121,	// 设置异常信息失败
	WE_CONFIG_GET_ALARM_IN_PARAM			= 5122,	// 获取报警输入信息失败
	WE_CONFIG_SET_ALARM_IN_PARAM			= 5123,	// 设置报警输入信息失败

	WE_CONFIG_GET_VIDEO_CHANNEL_PARAM		= 5130,	// 获取视频通道信息失败
	WE_CONFIG_SET_VIDEO_CHANNEL_PARAM		= 5131,	// 设置视频通道信息失败
	WE_CONFIG_GET_VIDEO_ENCODER_PARAM		= 5140,	// 获取视频编码信息失败
	WE_CONFIG_SET_VIDEO_ENCODER_PARAM		= 5141,	// 设置视频编码信息失败
	WE_CONFIG_GET_VIDEO_SCHEDULE_PARAM		= 5150,	// 获取录像计划配置失败
	WE_CONFIG_SET_VIDEO_SCHEDULE_PARAM		= 5151,	// 设置录像计划配置失败
	WE_CONFIG_IMG_UPLOAD_FAILD				= 5152,	// 图片下载失败

}WEB_ERROR_STATUS;

enum ai_errorcode
{
	ai_ok,
	ai_inerterror,
	ai_conectdberror,
	ai_execerror,
	ai_CreateAiFaceError,
	ai_InitAiFaceError,
	ai_LoadDbDataError,
	ai_LoadFileDataError,
	ai_NoUserError,
	ai_ParamError,
	ai_NoMatchFaceError,
	ai_AiUserExistedError,		// 添加用户失败, 当前身份证号的用户已经存在
	ai_NoMatchAlarmLogError,

};

// 文件传输状态
typedef enum
{
	FILE_TRANSFER_NONE = 0,		///< 文件传输没有开始, 或者出错
	FILE_TRANSFER_ON = 1,		///< 文件正在传输
	FILE_TRANSFER_DONE = 2,		///< 文件传输成功
	FILE_TRANSFER_FAIL = 3,		///< 文件传输失败

}FILE_TRANSFER_STATUS;

// 可以下载的文件目录类型
typedef enum
{
	EDIR_TYPE_UPLOAD_DIR = 0,	///< 上传文件目录, 默认为: /tmp目录, 
	EDIR_TYPE_SQL_DIR = 1,		///< 数据库备份文件目录,  /var/log/sql
	EDIR_TYPE_VAR_LOG = 2,		///< /var/log 目录

	EDIR_TYPE_IV_DIR = 3,		///< iv_pic目录, ai图片目录
	EDIR_TYPE_PIC_DIR = 4,		///< pic_dir目录, ai图片目录

	EDIR_TYPE_MAX,
}EDIR_TYPE;

// 目录后面没有/
#ifdef _WIN32
#define UPLOAD_FILE_PATH "./pic_user"
#define DATABASE_SQL_FILE_PATH "./pic_user"
#define VAR_LOG_FILE_PATH "./pic_user"
#define IV_FILE_PATH "./pic_user"
#define PIC_FILE_PATH "./pic_user"
#else
#define UPLOAD_FILE_PATH "/tmp"
#define DATABASE_SQL_FILE_PATH "/var/log/sql"
#define VAR_LOG_FILE_PATH "/var/log"
#define IV_FILE_PATH "/iv_dir"
#define PIC_FILE_PATH "/pic_dir"
#endif
