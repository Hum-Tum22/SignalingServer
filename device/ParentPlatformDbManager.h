#pragma once
#include <iostream>
#include <list>

#include "../SqliteDb.h"


class ParentPlatform
{
public:
    ParentPlatform();
    /**
     * id
     */
    int id;

    /**
     * �Ƿ�����
     */
    bool enable;

    /**
     * ����
     */
    std::string name;

    /**
     * SIP����������
     */
    std::string serverGBId;

    /**
     * SIP���������
     */
    std::string serverGBDomain;

    /**
     * SIP����IP
     */
    std::string serverIP;

    /**
     * SIP����˿�
     */
    int serverPort;

    /**
     * �豸������
     */
    std::string deviceGBId;

    /**
     * �豸ip
     */
    std::string deviceIp;

    /**
     * �豸�˿�
     */
    std::string devicePort;

    /**
     * SIP��֤�û���(Ĭ��ʹ���豸������)
     */
    std::string username;

    /**
     * SIP��֤����
     */
    std::string password;

    /**
     * ע������ (��)
     */
    std::string expires;

    /**
     * ��������(��)
     */
    std::string keepTimeout;

    /**
     * ����Э��
     * UDP/TCP
     */
    std::string transport;

    /**
     * �ַ���
     */
    std::string characterSet;

    /**
     * ������̨����
     */
    bool ptz;

    /**
     * RTCP������
     * TODO Ԥ��, �ݲ�ʵ��
     */
    bool rtcp;

    /**
     * ����״̬
     */
    bool status;

    /**
     * ����״̬
     */
    int channelCount;

    /**
     * �������е�ֱ����
     */
    bool shareAllLiveStream;

    /**
     * Ĭ��Ŀ¼Id,�Զ���ӵ�ͨ����������Ŀ¼��
     */
    std::string catalogId;

    /**
     * �ѱ�����Ŀ¼��Ϣ
     */
    bool catalogSubscribe;

    /**
     * �ѱ����ı�����Ϣ
     */
    bool alarmSubscribe;

    /**
     * �ѱ������ƶ�λ����Ϣ
     */
    bool mobilePositionSubscribe;

    /**
     * �㲥δ�������豸ʱ�Ƿ�ʹ��redis֪ͨ����
     */
    bool startOfflinePush;

    /**
     * Ŀ¼����-ÿ�����ϼ�����ͨ����Ϣʱ������Я����ͨ��������ȡֵ1,2,4,8
     */
    int catalogGroup;

    /**
     * ��������
     */
    std::string administrativeDivision;

};
class ChannelSourceInfo
{
public:
    std::string name;
    int count;
};

class ParentPlatformDbManager
{
protected:
    repro::SqliteDb* pDb;
public:
    ParentPlatformDbManager();
    virtual void IniTable();

    ////Insert("INSERT INTO parent_platform (enable, name, serverGBId, serverGBDomain, serverIP, serverPort, deviceGBId, deviceIp,  " +
    //    "        devicePort, username, password, expires, keepTimeout, transport, characterSet, ptz, rtcp, " +
    //    "        status, shareAllLiveStream, startOfflinePush, catalogId, administrativeDivision, catalogGroup) " +
    //    "        VALUES (${enable}, '${name}', '${serverGBId}', '${serverGBDomain}', '${serverIP}', ${serverPort}, '${deviceGBId}', '${deviceIp}', " +
    //    "        '${devicePort}', '${username}', '${password}', '${expires}', '${keepTimeout}', '${transport}', '${characterSet}', ${ptz}, ${rtcp}, " +
    //    "        ${status}, ${shareAllLiveStream},  ${startOfflinePush}, #{catalogId}, #{administrativeDivision}, #{catalogGroup})")
    int addParentPlatform(ParentPlatform parentPlatform);

    ////Update("UPDATE parent_platform " +
    //"SET enable=#{enable}, " +
    //"name=#{name}," +
    //"deviceGBId=#{deviceGBId}," +
    //"serverGBId=#{serverGBId}, " +
    //"serverGBDomain=#{serverGBDomain}, " +
    //"serverIP=#{serverIP}," +
    //"serverPort=#{serverPort}, " +
    //"deviceIp=#{deviceIp}, " +
    //"devicePort=#{devicePort}, " +
    //"username=#{username}, " +
    //"password=#{password}, " +
    //"expires=#{expires}, " +
    //"keepTimeout=#{keepTimeout}, " +
    //"transport=#{transport}, " +
    //"characterSet=#{characterSet}, " +
    //"ptz=#{ptz}, " +
    //"rtcp=#{rtcp}, " +
    //"status=#{status}, " +
    //"shareAllLiveStream=#{shareAllLiveStream}, " +
    //"startOfflinePush=${startOfflinePush}, " +
    //"catalogGroup=#{catalogGroup}, " +
    //"administrativeDivision=#{administrativeDivision}, " +
    //"catalogId=#{catalogId} " +
    //"WHERE id=#{id}")
    int updateParentPlatform(ParentPlatform parentPlatform);

    //Delete("DELETE FROM parent_platform WHERE serverGBId=#{serverGBId}")
    int delParentPlatform(ParentPlatform parentPlatform);

    ////Select("SELECT *, ((SELECT count(0)\n" +
    //"          FROM platform_gb_channel pc\n" +
    //"          WHERE pc.platformId = pp.serverGBId)\n" +
    //"          +\n" +
    //"          (SELECT count(0)\n" +
    //"          FROM platform_gb_stream pgs\n" +
    //"          WHERE pgs.platformId = pp.serverGBId)\n" +
    //"          +\n" +
    //"          (SELECT count(0)\n" +
    //"          FROM platform_catalog pgc\n" +
    //"          WHERE pgc.platformId = pp.serverGBId)) as channelCount\n" +
    //"FROM parent_platform pp ")
    std::list<ParentPlatform> getParentPlatformList();

    //Select("SELECT * FROM parent_platform WHERE enable=#{enable}")
    std::list<ParentPlatform> getEnableParentPlatformList(bool enable);

    //Select("SELECT * FROM parent_platform WHERE serverGBId=#{platformGbId}")
    ParentPlatform getParentPlatByServerGBId(std::string platformGbId);

    //Select("SELECT * FROM parent_platform WHERE id=#{id}")
    ParentPlatform getParentPlatById(int id);

    //Update("UPDATE parent_platform SET status=false")
    int outlineForAllParentPlatform();

    //Update("UPDATE parent_platform SET status=#{online} WHERE serverGBId=#{platformGbID}")
    int updateParentPlatformStatus(std::string platformGbID, bool online);

    //Select("SELECT * FROM parent_platform WHERE shareAllLiveStream=true")
    std::list<ParentPlatform> selectAllAhareAllLiveStream();

    ////Update(value = { " <script>" +
    //    "UPDATE parent_platform " +
    //    "SET catalogId=#{catalogId}" +
    //    "WHERE serverGBId=#{platformId}" +
    //    "</script>" })
    int setDefaultCatalog(std::string platformId, std::string catalogId);

    ////Select("select 'channel' as name, count(pgc.platformId) count from platform_gb_channel pgc left join device_channel dc on dc.id = pgc.deviceChannelId where  pgc.platformId=#{platformId} and dc.channelId =#{gbId} " +
    //"union " +
    //"select 'stream' as name, count(pgs.platformId) count from platform_gb_stream pgs left join gb_stream gs on pgs.gbStreamId = gs.gbStreamId where  pgs.platformId=#{platformId} and gs.gbId = #{gbId}")
    std::list<ChannelSourceInfo> getChannelSource(std::string platformId, std::string gbId);
};
