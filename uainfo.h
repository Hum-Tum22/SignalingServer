#pragma once
#include <time.h>
#include "resip/stack/Uri.hxx"
#include "resip/dum/ClientRegistration.hxx"
#include <mutex>


#ifndef _WIN32
#define __stdcall 
#endif
#define CUSTORLOCKGUARD std::lock_guard<std::mutex>

using namespace resip;
using namespace std;

typedef struct  _UaSessionInfo
{
    _UaSessionInfo() : passwd(""), i_State(0),//m_DialogSet(NULL), 
        sendhearterrortimes(0) {
        lastsendheartoktime = time(NULL);
        lastSendHeartTime = 0;
        heartTimeOutCount = 0;
    };
    _UaSessionInfo(const Uri& touri, const Uri& fromUri, const Data _passwd) // m_DialogSet(NULL),
        : toUri(touri), fromUri(fromUri), passwd(_passwd), i_State(0), sendhearterrortimes(0) {
        lastsendheartoktime = time(NULL);
        lastSendHeartTime = 0;
        heartTimeOutCount = 0;
    }
    Uri toUri;
    Uri fromUri;
    Data passwd;
    int            i_State;//sip state
    ClientRegistrationHandle mh;
    time_t         lastSendHeartTime;
    time_t         lastsendheartoktime;
    int            sendhearterrortimes;
    int            heartTimeOutCount;
    Data regcallid;
}UaSessionInfo;
typedef struct _DevConfig
{
    int HeartBeatCount;
    int HeartBeatInterval;
    int Expires;
}DevConfig;

typedef enum _MsgCmdType
{
    MsgCmdType_unknown,
    MsgCmdType_Catalog,
    MsgCmdType_Broadcast,
    MsgCmdType_DeviceControl,
    MsgCmdType_DeviceConfig,
    MsgCmdType_DeviceStatus,
    MsgCmdType_RecordInfo,
    MsgCmdType_Alarm,
    MsgCmdType_ConfigDownload,
    MsgCmdType_PresetQuery,
    MsgCmdType_MobilePosition,
    MsgCmdType_MediaStatus,
    MsgCmdType_DeviceInfo,

    MsgCmdType_Keepalive,

    MsgCmdType_MPAlarm,
    MsgCmdType_ECSAlarm,
    MsgCmdType_TGSAlarm,
    MsgCmdType_ConfigDefence,
    MsgCmdType_MPAlarmRecordList,
    MsgCmdType_ECSAlarmRecordList,
    MsgCmdType_TGSAlarmRecordList,
    MsgCmdType_Max
}MsgCmdType;
//expand information
class CatalogItemExpandInfo
{
public:
    int PTZType;                        //摄像机类型扩展,标识摄像机类型:1-球机;2-半球;3-固定枪机;4-遥控枪机。当目录项为摄像机时可选
    int PositionType;                   //摄像机位置类型扩展。1-省际检查站、2-党政机关、3-车站码头、4-中心广场、5 - 体育场馆、6 - 商业中心、7 - 宗教场所、8 - 校园周边、9 - 治安复杂区域、10 - 交通干线。当目录项为摄像机时可选
    int RoomType;                       //摄像机安装位置室外、室内属性。1-室外、2-室内。当目录项为摄像机时可选, 缺省为1
    int UseType;                        //摄像机用途属性。1-治安、2-交通、3-重点。当目录项为摄像机时可选
    int SupplyLightType;                //摄像机补光属性。1-无补光、2-红外补光、3-白光补光。当目录项为摄像机时可选, 缺省为1
    int DirectionType;                  //摄像机监视方位属性。1-东、2-西、3-南、4-北、5-东南、6-东北、7-西南、8-西北。当目录项为摄像机时且为固定摄像机或设置看守位摄像机时可选
    string Resolution;                  //摄像机支持的分辨率,可有多个分辨率值,各个取值间以“/”分隔。分辨率取值参见附录 F中SDPf字段规定。当目录项为摄像机时可选
    string BusinessGroupID;             //虚拟组织所属的业务分组ID,业务分组根据特定的业务需求制定,一个业务分组包含一组特定的虚拟组织
    string DownloadSpeed;               //下载倍速范围(可选),各可选参数以“/”分隔,如设备支持1,2,4倍速下载则应写为“1 / 2 / 4
    int SVCSpaceSupportMode;            //空域编码能力,取值0:不支持;1:1级增强(1个增强层);2:2级增强(2个增强层); 3:3级增强(3个增强层)(可选)
    int SVCTimeSupportMode;             //时域编码能力,取值0:不支持;1:1级增强;2:2级增强;3:3级增强(可选)
public:
    CatalogItemExpandInfo()
    {
        PTZType = 0;
        PositionType = 0;
        RoomType = 1;
        UseType = 0;
        SupplyLightType = 1;
        DirectionType = 0;
        SVCSpaceSupportMode = 0;
        SVCTimeSupportMode = 0;
    }
};
class CatalogItem
{
public:
    string DeviceID;                    //设备/区域/系统编码(必选)
    string Name;                        //设备/区域/系统名称(必选)
    string Manufacturer;                //当为设备时,设备厂商(必选)
    string Model;                       //当为设备时,设备型号(必选)
    string Owner;                       //当为设备时,设备归属(必选)
    string CivilCode;                   //行政区域(必选)
    string Block;                       //警区(可选)        //若设备属于某组织机构下,应在Block字段中填写相应组织机构代码,组织机构代码应符合 GA / T380—2011规定
    string Address;                     //当为设备时,安装地址(必选)
    int Parental;                       //当为设备时,是否有子设备(必选)1有,0没有
    string ParentID;                    //父设备/区域/系统ID(必选)   //若上传目录中有此设备的父设备则应填写父设备ID,若无父设备则应填写系统ID;若设备属于某虚拟组织下, 则应同时填写虚拟组织ID; 各个ID之间用“ / ”分隔
    int SafetyWay;                      //信令安全模式(可选)缺省为0; 0:不采用;2:S/MIME 签名方式;3:S/MIME加密签名同时采用方式; 4:数字摘要方式
    int RegisterWay;                    //注册方式(必选)缺省为1;1:符合IETFRFC3261标准的认证注册模式; 2:基于口令的双向认证注册模式; 3:基于数字证书的双向认证注册模式
    int CertNum;                        //证书序列号(有证书的设备必选)
    int Certifiable;                    //证书有效标识(有证书的设备必选)缺省为0;证书有效标识:0:无效 1:有效
    int ErrCode;                        //无效原因码(有证书且证书无效的设备必选)
    string EndTime;                     //证书终止有效期(有证书的设备必选)
    int Secrecy;                        //保密属性(必选)缺省为0;0:不涉密,1:涉密
    string IPAddress;                   //设备/区域/系统IP地址(可选)
    int Port;                           //设备/区域/系统端口(可选)
    string Password;                    //设备口令(可选)
    string Status;                      //设备状态(必选)
    double Longitude;                   //经度(可选)
    double Latitude;                    //纬度(可选)
//ipc 可选项
    CatalogItemExpandInfo info;
public:
    CatalogItem()
    {
        SafetyWay = 0;
        RegisterWay = 1;
        CertNum = 0;
        Certifiable = 0;
        ErrCode = 0;
        Secrecy = 0;
        Port = 0;
        Longitude = 0;
        Latitude = 0;
    };
    ~CatalogItem()
    {
    };
};
typedef struct
{
    Data m_content;
    Data m_event;
    ServerSubscriptionHandle m_sh;
}ServerSubscriptionInfo;
typedef struct
{
    vector<ServerSubscriptionInfo> Subscrips;
}ServerSubscriptionInfos;


typedef struct
{
    string DeviceName;
    string Result;
    string Manufacturer;
    string Model;
    string Firmware;
    int Channel;
}DeviceInfoMsg;
