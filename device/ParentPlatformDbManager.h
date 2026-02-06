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
     * 是否启用
     */
    bool enable;

    /**
     * 名称
     */
    std::string name;

    /**
     * SIP服务国标编码
     */
    std::string serverGBId;

    /**
     * SIP服务国标域
     */
    std::string serverGBDomain;

    /**
     * SIP服务IP
     */
    std::string serverIP;

    /**
     * SIP服务端口
     */
    int serverPort;

    /**
     * 设备国标编号
     */
    std::string deviceGBId;

    /**
     * 设备ip
     */
    std::string deviceIp;

    /**
     * 设备端口
     */
    std::string devicePort;

    /**
     * SIP认证用户名(默认使用设备国标编号)
     */
    std::string username;

    /**
     * SIP认证密码
     */
    std::string password;

    /**
     * 注册周期 (秒)
     */
    std::string expires;

    /**
     * 心跳周期(秒)
     */
    std::string keepTimeout;

    /**
     * 传输协议
     * UDP/TCP
     */
    std::string transport;

    /**
     * 字符集
     */
    std::string characterSet;

    /**
     * 允许云台控制
     */
    bool ptz;

    /**
     * RTCP流保活
     * TODO 预留, 暂不实现
     */
    bool rtcp;

    /**
     * 在线状态
     */
    bool status;

    /**
     * 在线状态
     */
    int channelCount;

    /**
     * 共享所有的直播流
     */
    bool shareAllLiveStream;

    /**
     * 默认目录Id,自动添加的通道多放在这个目录下
     */
    std::string catalogId;

    /**
     * 已被订阅目录信息
     */
    bool catalogSubscribe;

    /**
     * 已被订阅报警信息
     */
    bool alarmSubscribe;

    /**
     * 已被订阅移动位置信息
     */
    bool mobilePositionSubscribe;

    /**
     * 点播未推流的设备时是否使用redis通知拉起
     */
    bool startOfflinePush;

    /**
     * 目录分组-每次向上级发送通道信息时单个包携带的通道数量，取值1,2,4,8
     */
    int catalogGroup;

    /**
     * 行政区划
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
