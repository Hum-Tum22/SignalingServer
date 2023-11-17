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
    _UaSessionInfo() :i_State(0), passwd(""),//m_DialogSet(NULL), 
        sendhearterrortimes(0) {
        lastsendheartoktime = time(NULL);
        lastSendHeartTime = 0;
        heartTimeOutCount = 0;
    };
    _UaSessionInfo(const Uri& touri, const Uri& fromUri, const Data _passwd) :i_State(0),// m_DialogSet(NULL),
        toUri(touri), fromUri(fromUri), passwd(_passwd), sendhearterrortimes(0) {
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
    int PTZType;                        //�����������չ,��ʶ���������:1-���;2-����;3-�̶�ǹ��;4-ң��ǹ������Ŀ¼��Ϊ�����ʱ��ѡ
    int PositionType;                   //�����λ��������չ��1-ʡ�ʼ��վ��2-�������ء�3-��վ��ͷ��4-���Ĺ㳡��5 - �������ݡ�6 - ��ҵ���ġ�7 - �ڽ̳�����8 - У԰�ܱߡ�9 - �ΰ���������10 - ��ͨ���ߡ���Ŀ¼��Ϊ�����ʱ��ѡ
    int RoomType;                       //�������װλ�����⡢�������ԡ�1-���⡢2-���ڡ���Ŀ¼��Ϊ�����ʱ��ѡ, ȱʡΪ1
    int UseType;                        //�������;���ԡ�1-�ΰ���2-��ͨ��3-�ص㡣��Ŀ¼��Ϊ�����ʱ��ѡ
    int SupplyLightType;                //������������ԡ�1-�޲��⡢2-���ⲹ�⡢3-�׹ⲹ�⡣��Ŀ¼��Ϊ�����ʱ��ѡ, ȱʡΪ1
    int DirectionType;                  //��������ӷ�λ���ԡ�1-����2-����3-�ϡ�4-����5-���ϡ�6-������7-���ϡ�8-��������Ŀ¼��Ϊ�����ʱ��Ϊ�̶�����������ÿ���λ�����ʱ��ѡ
    string Resolution;                  //�����֧�ֵķֱ���,���ж���ֱ���ֵ,����ȡֵ���ԡ�/���ָ����ֱ���ȡֵ�μ���¼ F��SDPf�ֶι涨����Ŀ¼��Ϊ�����ʱ��ѡ
    string BusinessGroupID;             //������֯������ҵ�����ID,ҵ���������ض���ҵ�������ƶ�,һ��ҵ��������һ���ض���������֯
    string DownloadSpeed;               //���ر��ٷ�Χ(��ѡ),����ѡ�����ԡ�/���ָ�,���豸֧��1,2,4����������ӦдΪ��1 / 2 / 4
    int SVCSpaceSupportMode;            //�����������,ȡֵ0:��֧��;1:1����ǿ(1����ǿ��);2:2����ǿ(2����ǿ��); 3:3����ǿ(3����ǿ��)(��ѡ)
    int SVCTimeSupportMode;             //ʱ���������,ȡֵ0:��֧��;1:1����ǿ;2:2����ǿ;3:3����ǿ(��ѡ)
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
    string DeviceID;                    //�豸/����/ϵͳ����(��ѡ)
    string Name;                        //�豸/����/ϵͳ����(��ѡ)
    string Manufacturer;                //��Ϊ�豸ʱ,�豸����(��ѡ)
    string Model;                       //��Ϊ�豸ʱ,�豸�ͺ�(��ѡ)
    string Owner;                       //��Ϊ�豸ʱ,�豸����(��ѡ)
    string CivilCode;                   //��������(��ѡ)
    string Block;                       //����(��ѡ)        //���豸����ĳ��֯������,Ӧ��Block�ֶ�����д��Ӧ��֯��������,��֯��������Ӧ���� GA / T380��2011�涨
    string Address;                     //��Ϊ�豸ʱ,��װ��ַ(��ѡ)
    int Parental;                       //��Ϊ�豸ʱ,�Ƿ������豸(��ѡ)1��,0û��
    string ParentID;                    //���豸/����/ϵͳID(��ѡ)   //���ϴ�Ŀ¼���д��豸�ĸ��豸��Ӧ��д���豸ID,���޸��豸��Ӧ��дϵͳID;���豸����ĳ������֯��, ��Ӧͬʱ��д������֯ID; ����ID֮���á� / ���ָ�
    int SafetyWay;                      //���ȫģʽ(��ѡ)ȱʡΪ0; 0:������;2:S/MIME ǩ����ʽ;3:S/MIME����ǩ��ͬʱ���÷�ʽ; 4:����ժҪ��ʽ
    int RegisterWay;                    //ע�᷽ʽ(��ѡ)ȱʡΪ1;1:����IETFRFC3261��׼����֤ע��ģʽ; 2:���ڿ����˫����֤ע��ģʽ; 3:��������֤���˫����֤ע��ģʽ
    int CertNum;                        //֤�����к�(��֤����豸��ѡ)
    int Certifiable;                    //֤����Ч��ʶ(��֤����豸��ѡ)ȱʡΪ0;֤����Ч��ʶ:0:��Ч 1:��Ч
    int ErrCode;                        //��Чԭ����(��֤����֤����Ч���豸��ѡ)
    string EndTime;                     //֤����ֹ��Ч��(��֤����豸��ѡ)
    int Secrecy;                        //��������(��ѡ)ȱʡΪ0;0:������,1:����
    string IPAddress;                   //�豸/����/ϵͳIP��ַ(��ѡ)
    int Port;                           //�豸/����/ϵͳ�˿�(��ѡ)
    string Password;                    //�豸����(��ѡ)
    string Status;                      //�豸״̬(��ѡ)
    double Longitude;                   //����(��ѡ)
    double Latitude;                    //γ��(��ѡ)
//ipc ��ѡ��
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
