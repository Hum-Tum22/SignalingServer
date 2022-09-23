#ifndef DEVICE_MAPPER_H_
#define DEVICE_MAPPER_H_
#include "../SqliteDb.h"
#include "DeviceInfo.h"


class IDeviceMapper 
{
    //friend class SqliteDb;
    virtual void IniTable() = 0;
protected:
    repro::SqliteDb* pDb;
public:
    IDeviceMapper();
};
class GBDeviceChannelMapper : public IDeviceMapper
{
    //friend class SqliteDb;

    //friend int SqliteDb::dbquery(const char* zSql, char*** pazResult, int* pnRow, int* pnColumn, char** pzErrMsg);
public:
    GBDeviceChannelMapper() {};
    virtual void IniTable();

    /*@Insert("INSERT INTO device_channel (channelId, deviceId, name, manufacture, model, owner, civilCode, block, " +
        "address, parental, parentId, safetyWay, registerWay, certNum, certifiable, errCode, secrecy, " +
        "ipAddress, port, password, PTZType, status, streamId, longitude, latitude, createTime, updateTime) " +
        "VALUES ('${channelId}', '${deviceId}', '${name}', '${manufacture}', '${model}', '${owner}', '${civilCode}', '${block}'," +
        "'${address}', ${parental}, '${parentId}', ${safetyWay}, ${registerWay}, '${certNum}', ${certifiable}, ${errCode}, '${secrecy}', " +
        "'${ipAddress}', ${port}, '${password}', ${PTZType}, ${status}, '${streamId}', ${longitude}, ${latitude},'${createTime}', '${updateTime}')")*/
    int add(IDeviceChannel* channel);

    /*@Update(value = { " <script>" +
            "UPDATE device_channel " +
            "SET updateTime='${updateTime}'" +
            "<if test='name != null'>, name='${name}'</if>" +
            "<if test='manufacture != null'>, manufacture='${manufacture}'</if>" +
            "<if test='model != null'>, model='${model}'</if>" +
            "<if test='owner != null'>, owner='${owner}'</if>" +
            "<if test='civilCode != null'>, civilCode='${civilCode}'</if>" +
            "<if test='block != null'>, block='${block}'</if>" +
            "<if test='address != null'>, address='${address}'</if>" +
            "<if test='parental != null'>, parental=${parental}</if>" +
            "<if test='parentId != null'>, parentId='${parentId}'</if>" +
            "<if test='safetyWay != null'>, safetyWay=${safetyWay}</if>" +
            "<if test='registerWay != null'>, registerWay=${registerWay}</if>" +
            "<if test='certNum != null'>, certNum='${certNum}'</if>" +
            "<if test='certifiable != null'>, certifiable=${certifiable}</if>" +
            "<if test='errCode != null'>, errCode=${errCode}</if>" +
            "<if test='secrecy != null'>, secrecy='${secrecy}'</if>" +
            "<if test='ipAddress != null'>, ipAddress='${ipAddress}'</if>" +
            "<if test='port != null'>, port=${port}</if>" +
            "<if test='password != null'>, password='${password}'</if>" +
            "<if test='PTZType != null'>, PTZType=${PTZType}</if>" +
            "<if test='status != null'>, status='${status}'</if>" +
            "<if test='streamId != null'>, streamId='${streamId}'</if>" +
            "<if test='hasAudio != null'>, hasAudio=${hasAudio}</if>" +
            "<if test='longitude != null'>, longitude=${longitude}</if>" +
            "<if test='latitude != null'>, latitude=${latitude}</if>" +
            "WHERE deviceId='${deviceId}' AND channelId='${channelId}'" +
            " </script>" })*/
    int update(IDeviceChannel* channel);

    /*@Select(value = { " <script>" +
            "SELECT " +
            "dc.* " +
            "from " +
            "device_channel dc " +
            "WHERE " +
            "dc.deviceId = #{deviceId} " +
            " <if test='query != null'> AND (dc.channelId LIKE '%${query}%' OR dc.name LIKE '%${query}%' OR dc.name LIKE '%${query}%')</if> " +
            " <if test='parentChannelId != null'> AND dc.parentId=#{parentChannelId} </if> " +
            " <if test='online == true' > AND dc.status=1</if>" +
            " <if test='online == false' > AND dc.status=0</if>" +
            " <if test='hasSubChannel == true' >  AND dc.subCount > 0 </if>" +
            " <if test='hasSubChannel == false' >  AND dc.subCount = 0 </if>" +
            "ORDER BY dc.channelId " +
            " </script>" })*/
    list<GBDeviceChannel> queryChannels(string deviceId, string parentChannelId, string query, bool hasSubChannel, bool online);

    //@Select("SELECT * FROM device_channel WHERE deviceId=#{deviceId} AND channelId=#{channelId}")
    GBDeviceChannel queryChannel(string deviceId, string channelId);

    //@Delete("DELETE FROM device_channel WHERE deviceId=#{deviceId}")
    int cleanChannelsByDeviceId(string deviceId);

    //@Delete("DELETE FROM device_channel WHERE deviceId=#{deviceId} AND channelId=#{channelId}")
    int del(string deviceId, string channelId);

    //@Update(value = { "UPDATE device_channel SET streamId=null WHERE deviceId=#{deviceId} AND channelId=#{channelId}" })
    void stopPlay(string deviceId, string channelId);

    //@Update(value = { "UPDATE device_channel SET streamId=#{streamId} WHERE deviceId=#{deviceId} AND channelId=#{channelId}" })
    void startPlay(string deviceId, string channelId, string streamId);

    /*@Select(value = { " <script>" +
            "SELECT " +
            "    dc.id,\n" +
            "    dc.channelId,\n" +
            "    dc.deviceId,\n" +
            "    dc.name,\n" +
            "    de.manufacturer,\n" +
            "    de.hostAddress,\n" +
            "    dc.subCount,\n" +
            "    pgc.platformId as platformId,\n" +
            "    pgc.catalogId as catalogId " +
            " FROM device_channel dc " +
            " LEFT JOIN device de ON dc.deviceId = de.deviceId " +
            " LEFT JOIN platform_gb_channel pgc on pgc.deviceChannelId = dc.id " +
            " WHERE 1=1 " +
            " <if test='query != null'> AND (dc.channelId LIKE '%${query}%' OR dc.name LIKE '%${query}%' OR dc.name LIKE '%${query}%')</if> " +
            " <if test='online == true' > AND dc.status=1</if> " +
            " <if test='online == false' > AND dc.status=0</if> " +
            " <if test='hasSubChannel!= null and hasSubChannel == true' >  AND dc.subCount > 0</if> " +
            " <if test='hasSubChannel!= null and hasSubChannel == false' >  AND dc.subCount = 0</if> " +
            " <if test='catalogId == null ' >  AND dc.id not in (select deviceChannelId from platform_gb_channel where platformId=#{platformId} ) </if> " +
            " <if test='catalogId != null ' >  AND pgc.platformId = #{platformId} and pgc.catalogId=#{catalogId} </if> " +
            " ORDER BY dc.deviceId, dc.channelId ASC" +
            " </script>" })*/
            //list<ChannelReduce> queryChannelListInAll(string query, bool online, bool hasSubChannel, string platformId, string catalogId);

            /*@Select(value = { " <script>" +
                    "SELECT " +
                    "    dc.*,\n" +
                    "    pgc.platformId as platformId,\n" +
                    "    pgc.catalogId as catalogId " +
                    " FROM device_channel dc " +
                    " LEFT JOIN platform_gb_channel pgc on pgc.deviceChannelId = dc.id " +
                    " WHERE pgc.platformId = #{platformId} " +
                    " ORDER BY dc.deviceId, dc.channelId ASC" +
                    " </script>" })*/
                    //list<IDeviceChannelInPlatform> queryChannelByPlatformId(string platformId);


                    //@Select("SELECT * FROM device_channel WHERE channelId=#{channelId}")
    list<GBDeviceChannel> queryChannelByChannelId(string channelId);

    //@Update(value = { "UPDATE device_channel SET status=0 WHERE deviceId=#{deviceId} AND channelId=#{channelId}" })
    void offline(string deviceId, string channelId);

    //@Update(value = { "UPDATE device_channel SET status=1 WHERE deviceId=#{deviceId} AND channelId=#{channelId}" })
    void online(string deviceId, string channelId);

    /*@Insert("<script> " +
        "insert into device_channel " +
        "(channelId, deviceId, name, manufacture, model, owner, civilCode, block, subCount, " +
        "  address, parental, parentId, safetyWay, registerWay, certNum, certifiable, errCode, secrecy, " +
        "  ipAddress, port, password, PTZType, status, streamId, longitude, latitude, createTime, updateTime) " +
        "values " +
        "<foreach collection='addChannels' index='index' item='item' separator=','> " +
        "('${item.channelId}', '${item.deviceId}', '${item.name}', '${item.manufacture}', '${item.model}', " +
        "'${item.owner}', '${item.civilCode}', '${item.block}',${item.subCount}," +
        "'${item.address}', ${item.parental}, '${item.parentId}', ${item.safetyWay}, ${item.registerWay}, " +
        "'${item.certNum}', ${item.certifiable}, ${item.errCode}, '${item.secrecy}', " +
        "'${item.ipAddress}', ${item.port}, '${item.password}', ${item.PTZType}, ${item.status}, " +
        "'${item.streamId}', ${item.longitude}, ${item.latitude},'${item.createTime}', '${item.updateTime}')" +
        "</foreach> " +
        "ON DUPLICATE KEY UPDATE " +
        "updateTime=VALUES(updateTime), " +
        "name=VALUES(name), " +
        "manufacture=VALUES(manufacture), " +
        "model=VALUES(model), " +
        "owner=VALUES(owner), " +
        "civilCode=VALUES(civilCode), " +
        "block=VALUES(block), " +
        "subCount=VALUES(subCount), " +
        "address=VALUES(address), " +
        "parental=VALUES(parental), " +
        "parentId=VALUES(parentId), " +
        "safetyWay=VALUES(safetyWay), " +
        "registerWay=VALUES(registerWay), " +
        "certNum=VALUES(certNum), " +
        "certifiable=VALUES(certifiable), " +
        "errCode=VALUES(errCode), " +
        "secrecy=VALUES(secrecy), " +
        "ipAddress=VALUES(ipAddress), " +
        "port=VALUES(port), " +
        "password=VALUES(password), " +
        "PTZType=VALUES(PTZType), " +
        "status=VALUES(status), " +
        "streamId=VALUES(streamId), " +
        "longitude=VALUES(longitude), " +
        "latitude=VALUES(latitude)" +
        "</script>")*/
    int batchAdd(list<IDeviceChannel*> addChannels);

    /*@Update({ "<script>" +
            "<foreach collection='updateChannels' item='item' separator=';'>" +
            " UPDATE" +
            " device_channel" +
            " SET updateTime='${item.updateTime}'" +
            "<if test='item.name != null'>, name='${item.name}'</if>" +
            "<if test='item.manufacture != null'>, manufacture='${item.manufacture}'</if>" +
            "<if test='item.model != null'>, model='${item.model}'</if>" +
            "<if test='item.owner != null'>, owner='${item.owner}'</if>" +
            "<if test='item.civilCode != null'>, civilCode='${item.civilCode}'</if>" +
            "<if test='item.block != null'>, block='${item.block}'</if>" +
            "<if test='item.subCount != null'>, block=${item.subCount}</if>" +
            "<if test='item.address != null'>, address='${item.address}'</if>" +
            "<if test='item.parental != null'>, parental=${item.parental}</if>" +
            "<if test='item.parentId != null'>, parentId='${item.parentId}'</if>" +
            "<if test='item.safetyWay != null'>, safetyWay=${item.safetyWay}</if>" +
            "<if test='item.registerWay != null'>, registerWay=${item.registerWay}</if>" +
            "<if test='item.certNum != null'>, certNum='${item.certNum}'</if>" +
            "<if test='item.certifiable != null'>, certifiable=${item.certifiable}</if>" +
            "<if test='item.errCode != null'>, errCode=${item.errCode}</if>" +
            "<if test='item.secrecy != null'>, secrecy='${item.secrecy}'</if>" +
            "<if test='item.ipAddress != null'>, ipAddress='${item.ipAddress}'</if>" +
            "<if test='item.port != null'>, port=${item.port}</if>" +
            "<if test='item.password != null'>, password='${item.password}'</if>" +
            "<if test='item.PTZType != null'>, PTZType=${item.PTZType}</if>" +
            "<if test='item.status != null'>, status='${item.status}'</if>" +
            "<if test='item.streamId != null'>, streamId='${item.streamId}'</if>" +
            "<if test='item.hasAudio != null'>, hasAudio=${item.hasAudio}</if>" +
            "<if test='item.longitude != null'>, longitude=${item.longitude}</if>" +
            "<if test='item.latitude != null'>, latitude=${item.latitude}</if>" +
            "WHERE deviceId=#{item.deviceId} AND channelId=#{item.channelId}" +
            "</foreach>" +
            "</script>" })*/
    int batchUpdate(list<IDeviceChannel*> updateChannels);


    /*@Select(value = { " <script>" +
            "SELECT " +
            "dc1.* " +
            "from " +
            "device_channel dc1 " +
            "WHERE " +
            "dc1.deviceId = #{deviceId} " +
            " <if test='query != null'> AND (dc1.channelId LIKE '%${query}%' OR dc1.name LIKE '%${query}%' OR dc1.name LIKE '%${query}%')</if> " +
            " <if test='parentChannelId != null'> AND dc1.parentId=#{parentChannelId} </if> " +
            " <if test='online == true' > AND dc1.status=1</if>" +
            " <if test='online == false' > AND dc1.status=0</if>" +
            " <if test='hasSubChannel == true' >  AND dc1.subCount >0</if>" +
            " <if test='hasSubChannel == false' >  AND dc1.subCount=0</if>" +
            "ORDER BY dc1.channelId ASC " +
            "Limit #{limit} OFFSET #{start}" +
            " </script>" })*/
    list<GBDeviceChannel> queryChannelsByDeviceIdWithStartAndLimit(string deviceId, string parentChannelId, string query,
        bool hasSubChannel, bool online, int start, int limit);

    //@Select("SELECT * FROM device_channel WHERE deviceId=#{deviceId} AND status=1")
    list<GBDeviceChannel> queryOnlineChannelsByDeviceId(string deviceId);

    /*@Delete(value = { " <script>" +
            "DELETE " +
            "from " +
            "device_channel " +
            "WHERE " +
            "deviceId = #{deviceId} " +
            " AND channelId NOT IN " +
            "<foreach collection='channels'  item='item'  open='(' separator=',' close=')' > #{item.channelId}</foreach>" +
            " </script>" })*/
    int cleanChannelsNotInList(string deviceId, const list<IDeviceChannel>& channels);

    /*@Update(" update device_channel" +
        " set subCount = (select *" +
        "                from (select count(0)" +
        "                      from device_channel" +
        "                      where deviceId = #{deviceId} and parentId = #{channelId}) as temp)" +
        " where deviceId = #{deviceId} " +
        " and channelId = #{channelId}")*/
    int updateChannelSubCount(string deviceId, string channelId);

    //@Update(value = { "UPDATE device_channel SET latitude=${latitude}, longitude=${longitude} WHERE deviceId=#{deviceId} AND channelId=#{channelId}" })
    void updatePotion(string deviceId, string channelId, double longitude, double latitude);

    //@Select("SELECT * FROM device_channel WHERE length(trim(streamId)) > 0")
    list<IDeviceChannel> getAllChannelInPlay();
    int getAllChannelCount();
};
class GBDeviceMapper : public IDeviceMapper
{
    //friend class SqliteDb;

    //friend int SqliteDb::dbquery(const char* zSql, char*** pazResult, int* pnRow, int* pnColumn, char** pzErrMsg);
    GBDeviceChannelMapper mGBDeviceChannleMapper;
public:
    GBDeviceMapper();


    virtual void IniTable();

    GBDeviceChannelMapper& GetGBDeviceChannelMapper();
    //@Select("SELECT * FROM device WHERE deviceId = #{deviceId}")
    virtual SipServerDeviceInfo getDeviceByDeviceId(string deviceId);

    /*@Insert("INSERT INTO device (" +
                "deviceId, " +
                "name, " +
                "manufacturer, " +
                "model, " +
                "firmware, " +
                "transport," +
                "streamMode," +
                "ip," +
                "port," +
                "hostAddress," +
                "expires," +
                "registerTime," +
                "keepaliveTime," +
                "createTime," +
                "updateTime," +
                "charset," +
                "subscribeCycleForCatalog," +
                "subscribeCycleForMobilePosition," +
                "mobilePositionSubmissionInterval," +
                "subscribeCycleForAlarm," +
                "ssrcCheck," +
                "online" +
            ") VALUES (" +
                "#{deviceId}," +
                "#{name}," +
                "#{manufacturer}," +
                "#{model}," +
                "#{firmware}," +
                "#{transport}," +
                "#{streamMode}," +
                "#{ip}," +
                "#{port}," +
                "#{hostAddress}," +
                "#{expires}," +
                "#{registerTime}," +
                "#{keepaliveTime}," +
                "#{createTime}," +
                "#{updateTime}," +
                "#{charset}," +
                "#{subscribeCycleForCatalog}," +
                "#{subscribeCycleForMobilePosition}," +
                "#{mobilePositionSubmissionInterval}," +
                "#{subscribeCycleForAlarm}," +
                "#{ssrcCheck}," +
                "#{online}" +
            ")")*/
    virtual int add(Device* device);

    /*@Update(value = {" <script>" +
                "UPDATE device " +
                "SET updateTime='${updateTime}'" +
                "<if test=\"name != null\">, name='${name}'</if>" +
                "<if test=\"manufacturer != null\">, manufacturer='${manufacturer}'</if>" +
                "<if test=\"model != null\">, model='${model}'</if>" +
                "<if test=\"firmware != null\">, firmware='${firmware}'</if>" +
                "<if test=\"transport != null\">, transport='${transport}'</if>" +
                "<if test=\"streamMode != null\">, streamMode='${streamMode}'</if>" +
                "<if test=\"ip != null\">, ip='${ip}'</if>" +
                "<if test=\"port != null\">, port=${port}</if>" +
                "<if test=\"hostAddress != null\">, hostAddress='${hostAddress}'</if>" +
                "<if test=\"online != null\">, online=${online}</if>" +
                "<if test=\"registerTime != null\">, registerTime='${registerTime}'</if>" +
                "<if test=\"keepaliveTime != null\">, keepaliveTime='${keepaliveTime}'</if>" +
                "<if test=\"expires != null\">, expires=${expires}</if>" +
                "<if test=\"charset != null\">, charset='${charset}'</if>" +
                "<if test=\"subscribeCycleForCatalog != null\">, subscribeCycleForCatalog=${subscribeCycleForCatalog}</if>" +
                "<if test=\"subscribeCycleForMobilePosition != null\">, subscribeCycleForMobilePosition=${subscribeCycleForMobilePosition}</if>" +
                "<if test=\"mobilePositionSubmissionInterval != null\">, mobilePositionSubmissionInterval=${mobilePositionSubmissionInterval}</if>" +
                "<if test=\"subscribeCycleForAlarm != null\">, subscribeCycleForAlarm=${subscribeCycleForAlarm}</if>" +
                "<if test=\"ssrcCheck != null\">, ssrcCheck=${ssrcCheck}</if>" +
                "WHERE deviceId='${deviceId}'" +
            " </script>"})*/
    virtual int update(Device* device);

    //@Select("SELECT *, (SELECT count(0) FROM device_channel WHERE deviceId=de.deviceId) as channelCount  FROM device de")
    virtual list<SipServerDeviceInfo> getDevices();

    //@Delete("DELETE FROM device WHERE deviceId=#{deviceId}")
    virtual int del(string deviceId);

    //@Update("UPDATE device SET online=0")
    virtual int outlineForAll();

    //@Select("SELECT * FROM device WHERE online = 1")
    virtual list<SipServerDeviceInfo> getOnlineDevices();
    //@Select("SELECT * FROM device WHERE ip = #{host} AND port=${port}")
    virtual SipServerDeviceInfo getDeviceByHostAndPort(string host, int port);
};

#endif