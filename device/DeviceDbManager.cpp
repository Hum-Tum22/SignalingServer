
#include "DeviceDbManager.h"
#include "../SipServerConfig.h"
#include <iostream>
#include <sstream>

using namespace repro;
#ifndef USE_MYSQL
IDeviceMapper::IDeviceMapper() :pDb(0)
{
    pDb = dynamic_cast<SqliteDb*>(getCurDatabase());
}
void GBDeviceChannelMapper::IniTable()
{
    if (!pDb)
    {
        return;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "CREATE TABLE IF NOT EXISTS gb_device_channel("
        << "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        << "uuid VARCHAR(64) NOT NULL,"
        << "channelId VARCHAR(20) NOT NULL,"
        << "name VARCHAR(255) NOT NULL,"
        << "manufacturer VARCHAR(50) NOT NULL,"
        << "model VARCHAR(50) NOT NULL,"
        << "owner VARCHAR(50) NOT NULL,"
        << "civilCode VARCHAR(10) NOT NULL,"
        << "block VARCHAR(50) NOT NULL,"
        << "address VARCHAR(255) NOT NULL,"
        << "parentId VARCHAR(20) NOT NULL,"
        << "safetyWay INT DEFAULT 0,"
        << "registerWay INT DEFAULT 0,"
        << "certNum VARCHAR(50) NOT NULL,"
        << "certifiable INT DEFAULT 0,"
        << "errCode INT DEFAULT 0,"
        << "endTime VARCHAR(25) NOT NULL,"
        << "secrecy VARCHAR(25) NOT NULL,"
        << "ipAddress VARCHAR(25) NOT NULL,"
        << "port INT DEFAULT 0,"
        << "password VARCHAR(128) NOT NULL,"
        << "PTZType INT DEFAULT 0,"
        << "status INT DEFAULT 0,"
        << "longitude double DEFAULT 0,"
        << "latitude double DEFAULT 0,"
        << "streamId VARCHAR(50) NOT NULL,"
        << "deviceId VARCHAR(20) NOT NULL,"
        << "parental VARCHAR(50) NOT NULL,"
        << "hasAudio bit(1) DEFAULT 0,"
        << "createTime VARCHAR(25) NOT NULL,"
        << "updateTime VARCHAR(25) NOT NULL,"
        << "subCount VARCHAR(20) NOT NULL"
        << ")";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    else
    {
        //创建索引
        rc = pDb->Sqlite_exec("CREATE UNIQUE INDEX IF NOT EXISTS `gb_device_channel_id_uindex` on gb_device_channel(`id`)", &szErrMsg);
        if (rc != SQLITE_OK)
        {
            if (szErrMsg)
            {
                sqlite3_free(szErrMsg);
                szErrMsg = NULL;
            }
        }
        rc = pDb->Sqlite_exec("CREATE UNIQUE INDEX IF NOT EXISTS `gb_device_channel_id_pk` on gb_device_channel(`channelId`)", &szErrMsg);
        if (rc != SQLITE_OK)
        {
            if (szErrMsg)
            {
                sqlite3_free(szErrMsg);
                szErrMsg = NULL;
            }
        }
        rc = pDb->Sqlite_exec("CREATE UNIQUE INDEX IF NOT EXISTS `gb_device_channel_pk` on gb_device_channel(`channelId`,`deviceId`)", &szErrMsg);
        if (rc != SQLITE_OK)
        {
            if (szErrMsg)
            {
                sqlite3_free(szErrMsg);
                szErrMsg = NULL;
            }
        }
        rc = pDb->Sqlite_exec("CREATE UNIQUE INDEX IF NOT EXISTS `gb_device_channel_uuid` on gb_device_channel(`uuid`)", &szErrMsg);
        if (rc != SQLITE_OK)
        {
            if (szErrMsg)
            {
                sqlite3_free(szErrMsg);
                szErrMsg = NULL;
            }
        }
    }
    return;
}
int GBDeviceChannelMapper::add(IDeviceChannel* channel)
{
    if (!pDb)
    {
        return -1;
    }
    GBDeviceChannel *pDevChl = dynamic_cast<GBDeviceChannel*>(channel);
    if (!pDevChl)
        return -2;
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "INSERT INTO gb_device_channel("
        << "id,"
        << "uuid,"
        << "channelId,"
        << "name,"
        << "manufacturer,"
        << "model, "
        << "owner,"
        << "civilCode,"
        << "block,"
        << "address,"
        << "parentId,"
        << "safetyWay,"
        << "registerWay,"
        << "certNum,"
        << "certifiable,"
        << "errCode,"
        << "endTime,"
        << "secrecy,"
        << "ipAddress,"
        << "port,"
        << "password,"
        << "PTZType,"
        << "status,"
        << "longitude,"
        << "latitude,"
        << "streamId,"
        << "deviceId,"
        << "parental,"
        << "hasAudio,"
        << "createTime,"
        << "updateTime,"
        << "subCount"
        << ") VALUES("
        << "NULL,"
        << "'" << pDevChl->getUuid() << "',"
        << "'" << pDevChl->getChannelId() << "',"
        << "'" << pDevChl->getName() << "',"
        << "'" << pDevChl->getManufacture() << "',"
        << "'" << pDevChl->getModel() << "',"
        << "'" << pDevChl->getOwner() << "',"
        << "'" << pDevChl->getCivilCode() << "',"
        << "'" << pDevChl->getBlock() << "',"
        << "'" << pDevChl->getAddress() << "',"
        << "'" << pDevChl->getParentId() << "',"
        << "'" << pDevChl->getSafetyWay() << "',"
        << "'" << pDevChl->getRegisterWay() << "',"
        << "'" << pDevChl->getCertNum() << "',"
        << "'" << pDevChl->getCertifiable() << "',"
        << "'" << pDevChl->getErrCode() << "',"
        << "'" << pDevChl->getEndTime() << "',"
        << "'" << pDevChl->getSecrecy() << "',"
        << "'" << pDevChl->getIpAddress() << "',"
        << "'" << pDevChl->getPort() << "',"
        << "'" << pDevChl->getPassword() << "',"
        << "'" << pDevChl->getPTZType() << "',"
        << "'" << pDevChl->getStatus() << "',"
        << "'" << pDevChl->getLongitude() << "',"
        << "'" << pDevChl->getLatitude() << "',"
        << "'" << pDevChl->getStreamId() << "',"
        << "'" << pDevChl->getDeviceId() << "',"
        << "'" << pDevChl->getParental() << "',"
        << "'" << pDevChl->isHasAudio() << "',"
        << "'" << pDevChl->getCreateTime() << "',"
        << "'" << pDevChl->getUpdateTime() << "',"
        << "'" << pDevChl->getSubCount() << "'"
        << ");";
    ds.flush();
    if (pDevChl->getChannelId() == "34020000001320000017")
    {
        cout << "" << endl;
    }
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return 0;
}
int GBDeviceChannelMapper::update(IDeviceChannel* channel)
{
    if (!pDb)
    {
        return -1;
    }
    GBDeviceChannel* pDevChl = dynamic_cast<GBDeviceChannel*>(channel);
    if (!pDevChl)
        return -2;
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "UPDATE gb_device_channel SET ";
        if (!pDevChl->getUpdateTime().empty())
        {
            ds << "updateTime='" << pDevChl->getUpdateTime() << "'";
        }
        if (!pDevChl->getName().empty())
        {
            ds << "name='" << pDevChl->getName() << "'";
        }
        if (!pDevChl->getManufacture().empty())
        {
            ds << "manufacture='" << pDevChl->getManufacture() << "'";
        }
        if (!pDevChl->getModel().empty())
        {
            ds << "model='" << pDevChl->getModel() << "'";
        }
        if (!pDevChl->getOwner().empty())
        {
            ds << "owner='" << pDevChl->getOwner() << "'";
        }
        if (!pDevChl->getOwner().empty())
        {
            ds << "civilCode='" << pDevChl->getCivilCode() << "'";
        }
        if (!pDevChl->getBlock().empty())
        {
            ds << "block='" << pDevChl->getBlock() << "'";
        }
        if (!pDevChl->getAddress().empty())
        {
            ds << "address='" << pDevChl->getAddress() << "'";
        }
        if (!pDevChl->getParentId().empty())
        {
            ds << "parentId='" << pDevChl->getParentId() << "'";
        }
        ds << "safetyWay='" << pDevChl->getSafetyWay() << "'";
        ds << "registerWay='" << pDevChl->getRegisterWay() << "'";
        if (pDevChl->getCertNum().empty())
        {
            ds << "certNum='" << pDevChl->getCertNum() << "'";
        }
        ds << "certifiable='" << pDevChl->getCertifiable() << "'";
        ds << "errCode='" << pDevChl->getErrCode() << "'";
        if (pDevChl->getEndTime().empty())
        {
            ds << "endTime='" << pDevChl->getEndTime() << "'";
        }
        if (pDevChl->getSecrecy().empty())
        {
            ds << "secrecy='" << pDevChl->getSecrecy() << "'";
        }
        if (pDevChl->getIpAddress().empty())
        {
            ds << "ipAddress='" << pDevChl->getIpAddress() << "'";
        }
        if (pDevChl->getPort() != 0)
        {
            ds << "port='" << pDevChl->getPort() << "'";
        }
        if (pDevChl->getPassword().empty())
        {
            ds << "password='" << pDevChl->getPassword() << "'";
        }
        ds << "PTZType='" << pDevChl->getPTZType() << "'";
        ds << "status='" << pDevChl->getStatus() << "'";
        ds << "longitude='" << pDevChl->getLongitude() << "'";
        ds << "latitude='" << pDevChl->getLatitude() << "'";
        if (pDevChl->getStreamId().empty())
        {
            ds << "streamId='" << pDevChl->getStreamId() << "'";
        }
        if (pDevChl->getDeviceId().empty())
        {
            ds << "deviceId='" << pDevChl->getDeviceId() << "'";
        }
        if (pDevChl->getParental() != 0)
        {
            ds << "parental='" << pDevChl->getParental() << "'";
        }
        ds << "hasAudio='" << pDevChl->getParental() << "'";
        if (pDevChl->getCreateTime().empty())
        {
            ds << "createTime='" << pDevChl->getCreateTime() << "'";
        }
        if (pDevChl->getUpdateTime().empty())
        {
            ds << "updateTime='" << pDevChl->getUpdateTime() << "'";
        }
        ds << "subCount='" << pDevChl->getSubCount() << "'";
        ds << " WHERE deviceId='" << pDevChl->getDeviceId() << "' AND channelId='" << pDevChl->getChannelId() << "'";
    ds.flush();
    if (pDevChl->getChannelId() == "34020000001320000017")
    {
        cout << "" << endl;
    }
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return 0;
}
list<GBDeviceChannel> GBDeviceChannelMapper::queryChannels(string deviceId, string parentChannelId, string query, bool hasSubChannel, bool online)
{
    list<GBDeviceChannel> chllist;
    if (!pDb)
    {
        return chllist;
    }
    ostringstream ds;
    ds << "SELECT dc.* FROM gb_device_channel dc WHERE dc.deviceId='" << deviceId << "' ";
    if (!query.empty())
    {
        ds << " AND (dc.channelId LIKE '%" << query << "%' OR dc.name LIKE '%" << query << "%' OR dc.name LIKE '%" << query << "') ";
    }
    if (!parentChannelId.empty())
    {
        ds << " AND dc.parentId='" << parentChannelId << "' ";
    }
    ds << " AND dc.status='" << (online ? 1 : 0) << "' ";
    if (hasSubChannel)
    {
        ds << " AND dc.subCount > '0' ";
    }
    else
    {
        ds << " AND dc.subCount = '0' ";
    }
    ds << " ORDER BY dc.channelId ";
    ds.flush();
    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);

            countcol++;//skip id column
            GBDeviceChannel devchl;
            devchl.setUuid(azResult[countcol++]);
            devchl.setChannelId(azResult[countcol++]);
            devchl.setName(azResult[countcol++]);
            devchl.setManufacture(azResult[countcol++]);
            devchl.setModel(azResult[countcol++]);
            devchl.setOwner(azResult[countcol++]);
            devchl.setCivilCode(azResult[countcol++]);
            devchl.setBlock(azResult[countcol++]);
            devchl.setAddress(azResult[countcol++]);
            devchl.setParentId(azResult[countcol++]);
            devchl.setSafetyWay(atoi(azResult[countcol++]));
            devchl.setRegisterWay(atoi(azResult[countcol++]));
            devchl.setCertNum(azResult[countcol++]);
            devchl.setCertifiable(atoi(azResult[countcol++]));
            devchl.setErrCode(atoi(azResult[countcol++]));
            devchl.setEndTime(azResult[countcol++]);
            devchl.setSecrecy(azResult[countcol++]);
            devchl.setIpAddress(azResult[countcol++]);
            devchl.setPort(atoi(azResult[countcol++]));
            devchl.setPassword(azResult[countcol++]);
            devchl.setPTZType(atoi(azResult[countcol++]));
            devchl.setStatus(atoi(azResult[countcol++]));
            devchl.setLongitude(atof(azResult[countcol++]));
            devchl.setLatitude(atof(azResult[countcol++]));
            devchl.setStreamId(azResult[countcol++]);
            devchl.setDeviceId(azResult[countcol++]);
            devchl.setParental(atoi(azResult[countcol++]));
            devchl.setHasAudio(atoi(azResult[countcol++]));
            devchl.setCreateTime(azResult[countcol++]);
            devchl.setUpdateTime(azResult[countcol++]);
            devchl.setSubCount(atoi(azResult[countcol++]));
            chllist.push_back(devchl);
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return chllist;
}
GBDeviceChannel GBDeviceChannelMapper::queryChannel(string deviceId, string channelId)
{
    GBDeviceChannel devchl;
    if (!pDb)
    {
        return devchl;
    }
    ostringstream ds;
    ds << "SELECT * FROM gb_device_channel WHERE deviceId='" << deviceId << "' AND channelId='" << channelId << "'";
    ds.flush();

    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);

            countcol++;//skip id column
            devchl.setUuid(azResult[countcol++]);
            devchl.setChannelId(azResult[countcol++]);
            devchl.setName(azResult[countcol++]);
            devchl.setManufacture(azResult[countcol++]);
            devchl.setModel(azResult[countcol++]);
            devchl.setOwner(azResult[countcol++]);
            devchl.setCivilCode(azResult[countcol++]);
            devchl.setBlock(azResult[countcol++]);
            devchl.setAddress(azResult[countcol++]);
            devchl.setParentId(azResult[countcol++]);
            devchl.setSafetyWay(atoi(azResult[countcol++]));
            devchl.setRegisterWay(atoi(azResult[countcol++]));
            devchl.setCertNum(azResult[countcol++]);
            devchl.setCertifiable(atoi(azResult[countcol++]));
            devchl.setErrCode(atoi(azResult[countcol++]));
            devchl.setEndTime(azResult[countcol++]);
            devchl.setSecrecy(azResult[countcol++]);
            devchl.setIpAddress(azResult[countcol++]);
            devchl.setPort(atoi(azResult[countcol++]));
            devchl.setPassword(azResult[countcol++]);
            devchl.setPTZType(atoi(azResult[countcol++]));
            devchl.setStatus(atoi(azResult[countcol++]));
            devchl.setLongitude(atof(azResult[countcol++]));
            devchl.setLatitude(atof(azResult[countcol++]));
            devchl.setStreamId(azResult[countcol++]);
            devchl.setDeviceId(azResult[countcol++]);
            devchl.setParental(atoi(azResult[countcol++]));
            devchl.setHasAudio(atoi(azResult[countcol++]));
            devchl.setCreateTime(azResult[countcol++]);
            devchl.setUpdateTime(azResult[countcol++]);
            devchl.setSubCount(atoi(azResult[countcol++]));
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return devchl;
}
int GBDeviceChannelMapper::cleanChannelsByDeviceId(string deviceId)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "DELETE FROM gb_device_channel WHERE deviceId='"
        << deviceId << "'";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return 0;
}
int GBDeviceChannelMapper::del(string deviceId, string channelId)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table deviceId=#{deviceId} AND channelId=#{channelId}
    ostringstream ds;
    ds << "DELETE FROM gb_device_channel WHERE deviceId='"
        << deviceId << "' AND channelId='" << channelId << "'";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return 0;
}
void GBDeviceChannelMapper::stopPlay(string deviceId, string channelId)
{
    if (!pDb)
    {
        return;
    }
    char* szErrMsg = 0;
    //gb device channel table deviceId=#{deviceId} AND channelId=#{channelId}
    ostringstream ds;
    ds << "UPDATE gb_device_channel SET streamId="" WHERE deviceId='"
        << deviceId << "' AND channelId='" << channelId << "'";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return ;
}
void GBDeviceChannelMapper::startPlay(string deviceId, string channelId, string streamId)
{
    if (!pDb)
    {
        return;
    }
    char* szErrMsg = 0;
    //gb device channel table deviceId=#{deviceId} AND channelId=#{channelId}
    ostringstream ds;
    ds << "UPDATE gb_device_channel SET streamId='" << streamId << "' WHERE deviceId='"
        << deviceId << "' AND channelId='" << channelId << "'";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return;
}
//list<ChannelReduce> GBDeviceChannelMapper::queryChannelListInAll(string query, bool online, bool hasSubChannel, string platformId, string catalogId)
//{
//}
//list<IDeviceChannelInPlatform> GBDeviceChannelMapper::queryChannelByPlatformId(string platformId)
//{
//}
list<GBDeviceChannel> GBDeviceChannelMapper::queryChannelByChannelId(string channelId)
{
    list<GBDeviceChannel> chllist;
    if (!pDb)
    {
        return chllist;
    }
    ostringstream ds;
    ds << "SELECT * FROM gb_device_channel WHERE channelId='" << channelId << "'";
    ds.flush();
    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);

            GBDeviceChannel devchl;
            devchl.setId(std::stoi(azResult[countcol++]));
            devchl.setUuid(azResult[countcol++]);
            devchl.setChannelId(azResult[countcol++]);
            devchl.setName(azResult[countcol++]);
            devchl.setManufacture(azResult[countcol++]);
            devchl.setModel(azResult[countcol++]);
            devchl.setOwner(azResult[countcol++]);
            devchl.setCivilCode(azResult[countcol++]);
            devchl.setBlock(azResult[countcol++]);
            devchl.setAddress(azResult[countcol++]);
            devchl.setParentId(azResult[countcol++]);
            devchl.setSafetyWay(atoi(azResult[countcol++]));
            devchl.setRegisterWay(atoi(azResult[countcol++]));
            devchl.setCertNum(azResult[countcol++]);
            devchl.setCertifiable(atoi(azResult[countcol++]));
            devchl.setErrCode(atoi(azResult[countcol++]));
            devchl.setEndTime(azResult[countcol++]);
            devchl.setSecrecy(azResult[countcol++]);
            devchl.setIpAddress(azResult[countcol++]);
            devchl.setPort(atoi(azResult[countcol++]));
            devchl.setPassword(azResult[countcol++]);
            devchl.setPTZType(atoi(azResult[countcol++]));
            devchl.setStatus(atoi(azResult[countcol++]));
            devchl.setLongitude(atof(azResult[countcol++]));
            devchl.setLatitude(atof(azResult[countcol++]));
            devchl.setStreamId(azResult[countcol++]);
            devchl.setDeviceId(azResult[countcol++]);
            devchl.setParental(atoi(azResult[countcol++]));
            devchl.setHasAudio(atoi(azResult[countcol++]));
            devchl.setCreateTime(azResult[countcol++]);
            devchl.setUpdateTime(azResult[countcol++]);
            devchl.setSubCount(atoi(azResult[countcol++]));
            chllist.push_back(devchl);
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return chllist;
}
void GBDeviceChannelMapper::offline(string deviceId, string channelId)
{
    if (!pDb)
    {
        return;
    }
    char* szErrMsg = 0;
    //gb device channel table deviceId=#{deviceId} AND channelId=#{channelId}
    ostringstream ds;
    ds << "UPDATE gb_device_channel SET status='0' WHERE deviceId='"
        << deviceId << "' AND channelId='" << channelId << "'";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return;
}
void GBDeviceChannelMapper::online(string deviceId, string channelId)
{
    if (!pDb)
    {
        return;
    }
    char* szErrMsg = 0;
    //gb device channel table deviceId=#{deviceId} AND channelId=#{channelId}
    ostringstream ds;
    ds << "UPDATE gb_device_channel SET status='1' WHERE deviceId='"
        << deviceId << "' AND channelId='" << channelId << "'";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return;
}
int GBDeviceChannelMapper::batchAdd(list<IDeviceChannel*> addChannels)
{
    if (!pDb)
    {
        return -1;
    }
    int rc = 0;
    if (pDb->BeginTransaction())
    {
        for (auto& it : addChannels)
        {
            rc = add(it);
            if (rc != 0)
                break;
        }
        if (rc == 0)
        {
            pDb->CommitTransaction();
        }
        else
        {
            pDb->RollbackTransaction();
        }
    }
    return rc;
}
int GBDeviceChannelMapper::batchUpdate(list<IDeviceChannel*> updateChannels)
{
    if (!pDb)
    {
        return -1;
    }
    int rc = 0;
    if (pDb->BeginTransaction())
    {
        for (auto& it : updateChannels)
        {
            rc = update(it);
            if (rc != 0)
                break;
        }
        if (rc == 0)
        {
            pDb->CommitTransaction();
        }
        else
        {
            pDb->RollbackTransaction();
        }
    }
    return rc;
}
list<GBDeviceChannel> GBDeviceChannelMapper::queryChannelsByDeviceIdWithStartAndLimit(string deviceId, string parentChannelId, string query,
    bool hasSubChannel, bool online, int start, int limit)
{
    list<GBDeviceChannel> chllist;
    if (!pDb)
    {
        return chllist;
    }
    ostringstream ds;
    ds << "SELECT dc1.* FROM gb_device_channel dc1 WHERE dc1.deviceId ='" << deviceId << "'";
    if (!query.empty())
    {
        ds << " AND (dc1.channelId LIKE '" << query << "%' OR dc1.name LIKE '%" << query << "%' OR dc1.name LIKE '%" << query << "%') ";
    }
    if (!parentChannelId.empty())
    {
        ds << " AND dc1.parentId='" << parentChannelId << "' ";
    }
    ds << " AND dc1.status='" << (online ? 1 : 0) << "' ";
    if (hasSubChannel)
    {
        ds << " AND dc1.subCount >'0' ";
    }
    else
    {
        ds << " AND dc1.subCount ='0' ";
    }
    ds << " ORDER BY dc1.channelId ASC Limit '" << limit << "' OFFSET '" << start << "'";
    ds.flush();
    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);

            countcol++;//skip id column
            GBDeviceChannel devchl;
            devchl.setUuid(azResult[countcol++]);
            devchl.setChannelId(azResult[countcol++]);
            devchl.setName(azResult[countcol++]);
            devchl.setManufacture(azResult[countcol++]);
            devchl.setModel(azResult[countcol++]);
            devchl.setOwner(azResult[countcol++]);
            devchl.setCivilCode(azResult[countcol++]);
            devchl.setBlock(azResult[countcol++]);
            devchl.setAddress(azResult[countcol++]);
            devchl.setParentId(azResult[countcol++]);
            devchl.setSafetyWay(atoi(azResult[countcol++]));
            devchl.setRegisterWay(atoi(azResult[countcol++]));
            devchl.setCertNum(azResult[countcol++]);
            devchl.setCertifiable(atoi(azResult[countcol++]));
            devchl.setErrCode(atoi(azResult[countcol++]));
            devchl.setEndTime(azResult[countcol++]);
            devchl.setSecrecy(azResult[countcol++]);
            devchl.setIpAddress(azResult[countcol++]);
            devchl.setPort(atoi(azResult[countcol++]));
            devchl.setPassword(azResult[countcol++]);
            devchl.setPTZType(atoi(azResult[countcol++]));
            devchl.setStatus(atoi(azResult[countcol++]));
            devchl.setLongitude(atof(azResult[countcol++]));
            devchl.setLatitude(atof(azResult[countcol++]));
            devchl.setStreamId(azResult[countcol++]);
            devchl.setDeviceId(azResult[countcol++]);
            devchl.setParental(atoi(azResult[countcol++]));
            devchl.setHasAudio(atoi(azResult[countcol++]));
            devchl.setCreateTime(azResult[countcol++]);
            devchl.setUpdateTime(azResult[countcol++]);
            devchl.setSubCount(atoi(azResult[countcol++]));
            chllist.push_back(devchl);
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return chllist;
}
list<GBDeviceChannel> GBDeviceChannelMapper::queryOnlineChannelsByDeviceId(string deviceId)
{
    list<GBDeviceChannel> chllist;
    if (!pDb)
    {
        return chllist;
    }
    ostringstream ds;
    ds << "SELECT * FROM gb_device_channel WHERE deviceId='" << deviceId << "' AND status='1'";
    ds.flush();
    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);

            countcol++;//skip id column
            GBDeviceChannel devchl;
            devchl.setUuid(azResult[countcol++]);
            devchl.setChannelId(azResult[countcol++]);
            devchl.setName(azResult[countcol++]);
            devchl.setManufacture(azResult[countcol++]);
            devchl.setModel(azResult[countcol++]);
            devchl.setOwner(azResult[countcol++]);
            devchl.setCivilCode(azResult[countcol++]);
            devchl.setBlock(azResult[countcol++]);
            devchl.setAddress(azResult[countcol++]);
            devchl.setParentId(azResult[countcol++]);
            devchl.setSafetyWay(atoi(azResult[countcol++]));
            devchl.setRegisterWay(atoi(azResult[countcol++]));
            devchl.setCertNum(azResult[countcol++]);
            devchl.setCertifiable(atoi(azResult[countcol++]));
            devchl.setErrCode(atoi(azResult[countcol++]));
            devchl.setEndTime(azResult[countcol++]);
            devchl.setSecrecy(azResult[countcol++]);
            devchl.setIpAddress(azResult[countcol++]);
            devchl.setPort(atoi(azResult[countcol++]));
            devchl.setPassword(azResult[countcol++]);
            devchl.setPTZType(atoi(azResult[countcol++]));
            devchl.setStatus(atoi(azResult[countcol++]));
            devchl.setLongitude(atof(azResult[countcol++]));
            devchl.setLatitude(atof(azResult[countcol++]));
            devchl.setStreamId(azResult[countcol++]);
            devchl.setDeviceId(azResult[countcol++]);
            devchl.setParental(atoi(azResult[countcol++]));
            devchl.setHasAudio(atoi(azResult[countcol++]));
            devchl.setCreateTime(azResult[countcol++]);
            devchl.setUpdateTime(azResult[countcol++]);
            devchl.setSubCount(atoi(azResult[countcol++]));
            chllist.push_back(devchl);
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return chllist;
}
list<std::shared_ptr<IDeviceChannel>> GBDeviceChannelMapper::queryChannelByDeviceId(string deviceId)
{
    list<std::shared_ptr<IDeviceChannel>> chllist;
    if (!pDb)
    {
        return chllist;
    }
    ostringstream ds;
    ds << "SELECT * FROM gb_device_channel WHERE deviceId='" << deviceId << "'";
    ds.flush();
    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);

            countcol++;//skip id column
            std::shared_ptr<GBDeviceChannel> devchl = std::make_shared<GBDeviceChannel>();
            devchl->setUuid(azResult[countcol++]);
            devchl->setChannelId(azResult[countcol++]);
            devchl->setName(azResult[countcol++]);
            devchl->setManufacture(azResult[countcol++]);
            devchl->setModel(azResult[countcol++]);
            devchl->setOwner(azResult[countcol++]);
            devchl->setCivilCode(azResult[countcol++]);
            devchl->setBlock(azResult[countcol++]);
            devchl->setAddress(azResult[countcol++]);
            devchl->setParentId(azResult[countcol++]);
            devchl->setSafetyWay(atoi(azResult[countcol++]));
            devchl->setRegisterWay(atoi(azResult[countcol++]));
            devchl->setCertNum(azResult[countcol++]);
            devchl->setCertifiable(atoi(azResult[countcol++]));
            devchl->setErrCode(atoi(azResult[countcol++]));
            devchl->setEndTime(azResult[countcol++]);
            devchl->setSecrecy(azResult[countcol++]);
            devchl->setIpAddress(azResult[countcol++]);
            devchl->setPort(atoi(azResult[countcol++]));
            devchl->setPassword(azResult[countcol++]);
            devchl->setPTZType(atoi(azResult[countcol++]));
            devchl->setStatus(atoi(azResult[countcol++]));
            devchl->setLongitude(atof(azResult[countcol++]));
            devchl->setLatitude(atof(azResult[countcol++]));
            devchl->setStreamId(azResult[countcol++]);
            devchl->setDeviceId(azResult[countcol++]);
            devchl->setParental(atoi(azResult[countcol++]));
            devchl->setHasAudio(atoi(azResult[countcol++]));
            devchl->setCreateTime(azResult[countcol++]);
            devchl->setUpdateTime(azResult[countcol++]);
            devchl->setSubCount(atoi(azResult[countcol++]));
            chllist.push_back(devchl);
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return chllist;
}
int GBDeviceChannelMapper::cleanChannelsNotInList(string deviceId, const list<IDeviceChannel>& channels)
{
    return 0;
}
int GBDeviceChannelMapper::updateChannelSubCount(string deviceId, string channelId)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "UPDATE gb_device_channel SET set subCount = (select * from (select count(0) from gb_device_channel"
        << " where deviceId ='" << deviceId << "' and parentId ='" << channelId << "') as temp) "
        << " where deviceId ='" << deviceId << "' and channelId ='" << channelId << "'";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return 0;
}
void GBDeviceChannelMapper::updatePotion(string deviceId, string channelId, double longitude, double latitude)
{
    if (!pDb)
    {
        return ;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "UPDATE gb_device_channel SET latitude='" << latitude << "', longitude='" << longitude << "' WHERE deviceId='" << deviceId << "' AND channelId='" << channelId << "'";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return ;
}
list<IDeviceChannel> GBDeviceChannelMapper::getAllChannelInPlay()
{
    list<IDeviceChannel> chllist;
    if (!pDb)
    {
        return chllist;
    }
    ostringstream ds;
    ds << "SELECT * FROM gb_device_channel WHERE length(trim(streamId)) > '0'";
    ds.flush();
    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);

            countcol++;//skip id column
            GBDeviceChannel devchl;
            devchl.setUuid(azResult[countcol++]);
            devchl.setChannelId(azResult[countcol++]);
            devchl.setName(azResult[countcol++]);
            devchl.setManufacture(azResult[countcol++]);
            devchl.setModel(azResult[countcol++]);
            devchl.setOwner(azResult[countcol++]);
            devchl.setCivilCode(azResult[countcol++]);
            devchl.setBlock(azResult[countcol++]);
            devchl.setAddress(azResult[countcol++]);
            devchl.setParentId(azResult[countcol++]);
            devchl.setSafetyWay(atoi(azResult[countcol++]));
            devchl.setRegisterWay(atoi(azResult[countcol++]));
            devchl.setCertNum(azResult[countcol++]);
            devchl.setCertifiable(atoi(azResult[countcol++]));
            devchl.setErrCode(atoi(azResult[countcol++]));
            devchl.setEndTime(azResult[countcol++]);
            devchl.setSecrecy(azResult[countcol++]);
            devchl.setIpAddress(azResult[countcol++]);
            devchl.setPort(atoi(azResult[countcol++]));
            devchl.setPassword(azResult[countcol++]);
            devchl.setPTZType(atoi(azResult[countcol++]));
            devchl.setStatus(atoi(azResult[countcol++]));
            devchl.setLongitude(atof(azResult[countcol++]));
            devchl.setLatitude(atof(azResult[countcol++]));
            devchl.setStreamId(azResult[countcol++]);
            devchl.setDeviceId(azResult[countcol++]);
            devchl.setParental(atoi(azResult[countcol++]));
            devchl.setHasAudio(atoi(azResult[countcol++]));
            devchl.setCreateTime(azResult[countcol++]);
            devchl.setUpdateTime(azResult[countcol++]);
            devchl.setSubCount(atoi(azResult[countcol++]));
            chllist.push_back(devchl);
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return chllist;
}
int GBDeviceChannelMapper::getAllChannelCount()
{
    if (!pDb)
    {
        return -1;
    }
    ostringstream ds;
    ds << "SELECT COUNT(*) FROM gb_device_channel ";
    ds.flush();
    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int nCount = 0;
        if (ncolumn >= 1 && azResult[ncolumn] != NULL)
        {
            nCount = atoi(azResult[ncolumn]);
        }
        sqlite3_free_table(azResult);
        return nCount;
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return 0;
}
GBDeviceMapper::GBDeviceMapper()
{
}
void GBDeviceMapper::IniTable()
{
    if (!pDb)
    {
        return;
    }
    char* szErrMsg = 0;
    //gb device table
    ostringstream ds;
    ds << "CREATE TABLE IF NOT EXISTS gb_device("
        << "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        << "uuid VARCHAR(64) NOT NULL,"
        << "deviceId VARCHAR(20) NOT NULL,"
        << "name VARCHAR(255) NOT NULL,"
        << "manufacturer VARCHAR(255) NOT NULL,"
        << "model VARCHAR(255) NOT NULL,"
        << "firmware VARCHAR(255) NOT NULL,"
        << "transport VARCHAR(20) NOT NULL,"
        << "streamMode VARCHAR(20) NOT NULL,"
        << "online INT DEFAULT 0,"
        << "registerTime VARCHAR(25) NOT NULL,"
        << "keepaliveTime VARCHAR(25) NOT NULL,"
        << "ip VARCHAR(50) NOT NULL,"
        << "createTime VARCHAR(25) NOT NULL,"
        << "updateTime VARCHAR(25) NOT NULL,"
        << "port INT DEFAULT 0,"
        << "expires INT DEFAULT 0,"
        << "subscribeCycleForCatalog INT DEFAULT 0,"
        << "subscribeCycleForMobilePosition INT DEFAULT 0,"
        << "mobilePositionSubmissionInterval INT DEFAULT 0,"
        << "subscribeCycleForAlarm INT DEFAULT 0,"
        << "hostAddress VARCHAR(50) NOT NULL,"
        << "charset VARCHAR(20) NOT NULL,"
        << "ssrcCheck VARCHAR(20) NOT NULL"
        << ")";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
        return;
    }
    else
    {
        //创建索引
        rc = pDb->Sqlite_exec("CREATE UNIQUE INDEX IF NOT EXISTS `gb_device_deviceId_uindex` on gb_device(`deviceId`)", &szErrMsg);
        if (rc != SQLITE_OK)
        {
            if (szErrMsg)
            {
                sqlite3_free(szErrMsg);
                szErrMsg = NULL;
            }
        }
        rc = pDb->Sqlite_exec("CREATE UNIQUE INDEX IF NOT EXISTS `gb_device_uuid` on gb_device(`uuid`)", &szErrMsg);
        if (rc != SQLITE_OK)
        {
            if (szErrMsg)
            {
                sqlite3_free(szErrMsg);
                szErrMsg = NULL;
            }
        }
    }
    mGBDeviceChannleMapper.IniTable();
}
GBDeviceChannelMapper& GBDeviceMapper::GetGBDeviceChannelMapper()
{
    return mGBDeviceChannleMapper;
}
SipServerDeviceInfo GBDeviceMapper::getDeviceByDeviceId(string deviceId)
{
    SipServerDeviceInfo devinfo;
    if (!pDb)
    {
        return devinfo;
    }
    //query gb device 
    //"SELECT * FROM device WHERE deviceId ='"
    ostringstream ds;
    ds << "SELECT * FROM gb_device WHERE deviceId ='"
        << deviceId
        << "'";
    ds.flush();

    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    //int rc = sqlite3_get_table(mConn, command.c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);
            countcol++; //skip id column
            devinfo.setUuId(azResult[countcol++]);
            devinfo.setDeviceId(azResult[countcol++]);
            devinfo.setName(azResult[countcol++]);
            devinfo.setManufacturer(azResult[countcol++]);
            devinfo.setModel(azResult[countcol++]);
            devinfo.setFirmware(azResult[countcol++]);
            devinfo.setTransport(azResult[countcol++]);
            devinfo.setStreamMode(azResult[countcol++]);
            devinfo.setOnline(atoi(azResult[countcol++]));
            devinfo.setRegisterTime(azResult[countcol++]);
            devinfo.setKeepaliveTime(azResult[countcol++]);
            devinfo.setIp(azResult[countcol++]);
            devinfo.setCreateTime(azResult[countcol++]);
            devinfo.setUpdateTime(azResult[countcol++]);
            devinfo.setPort(atoi(azResult[countcol++]));
            devinfo.setExpires(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForCatalog(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForMobilePosition(atoi(azResult[countcol++]));
            devinfo.setMobilePositionSubmissionInterval(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForAlarm(atoi(azResult[countcol++]));
            devinfo.setHostAddress(azResult[countcol++]);
            devinfo.setCharset(azResult[countcol++]);
            devinfo.setSsrcCheck(azResult[countcol++]);
            devinfo.setDevAccessProtocal(Device::DEV_ACCESS_GB28181);
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return devinfo;
}
int GBDeviceMapper::add(Device* device)
{
    SipServerDeviceInfo* GbDev = dynamic_cast<SipServerDeviceInfo*>(device);
    if (!GbDev)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //add gb device
    ostringstream ds; 
    ds << "INSERT INTO gb_device("
        << "id,"
        << "uuid,"
        << "deviceId,"
        << "name,"
        << "manufacturer,"
        << "model,"
        << "firmware,"
        << "transport,"
        << "streamMode,"
        << "online,"
        << "registerTime,"
        << "keepaliveTime,"
        << "ip,"
        << "createTime,"
        << "updateTime,"
        << "port,"
        << "expires,"
        << "subscribeCycleForCatalog,"
        << "subscribeCycleForMobilePosition,"
        << "mobilePositionSubmissionInterval,"
        << "subscribeCycleForAlarm,"
        << "hostAddress,"
        << "charset,"
        << "ssrcCheck"
        << ") VALUES("
        << "NULL,"
        << "'" << GbDev->getUuId() << "',"
        << "'" << GbDev->getDeviceId() << "',"
        << "'" << GbDev->getName() << "',"
        << "'" << GbDev->getManufacturer() << "',"
        << "'" << GbDev->getModel() << "',"
        << "'" << GbDev->getFirmware() << "',"
        << "'" << GbDev->getTransport() << "',"
        << "'" << GbDev->getStreamMode() << "',"
        << "'" << GbDev->getOnline() << "',"
        << "'" << GbDev->getRegisterTime() << "',"
        << "'" << GbDev->getKeepaliveTime() << "',"
        << "'" << GbDev->getIp() << "',"
        << "'" << GbDev->getCreateTime() << "',"
        << "'" << GbDev->getUpdateTime() << "',"
        << "'" << GbDev->getPort() << "',"
        << "'" << GbDev->getExpires() << "',"
        << "'" << GbDev->getSubscribeCycleForCatalog() << "',"
        << "'" << GbDev->getSubscribeCycleForMobilePosition() << "',"
        << "'" << GbDev->getMobilePositionSubmissionInterval() << "',"
        << "'" << GbDev->getSubscribeCycleForAlarm() << "',"
        << "'" << GbDev->getHostAddress() << "',"
        << "'" << GbDev->getCharset() << "',"
        << "'" << GbDev->isSsrcCheck() << "'"
        << ")";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return rc;
}
int GBDeviceMapper::update(Device* device)
{
    SipServerDeviceInfo* GbDev = dynamic_cast<SipServerDeviceInfo*>(device);
    if (!GbDev)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //add gb device
    ostringstream ds;
    ds << "UPDATE gb_device SET "
        << " updateTime='" << GbDev->getUpdateTime() << "',"
        << " name='" << GbDev->getName() << "',"
        << " manufacturer='" << GbDev->getManufacturer() << "',"
        << " model='" << GbDev->getModel() << "',"
        << " firmware='" << GbDev->getFirmware() << "',"
        << " transport='" << GbDev->getTransport() << "',"
        << " streamMode='" << GbDev->getStreamMode() << "',"
        << " ip='" << GbDev->getIp() << "',"
        << " port='" << GbDev->getPort() << "',"
        << " hostAddress='" << GbDev->getHostAddress() << "',"
        << " online='" << GbDev->getOnline() << "',"
        << " registerTime='" << GbDev->getRegisterTime() << "',"
        << " keepaliveTime='" << GbDev->getKeepaliveTime() << "',"
        << " expires='" << GbDev->getExpires() << "',"
        << " charset='" << GbDev->getCharset() << "',"
        << " subscribeCycleForCatalog='" << GbDev->getSubscribeCycleForCatalog() << "',"
        << " subscribeCycleForMobilePosition='" << GbDev->getSubscribeCycleForMobilePosition() << "',"
        << " mobilePositionSubmissionInterval='" << GbDev->getMobilePositionSubmissionInterval() << "',"
        << " subscribeCycleForAlarm='" << GbDev->getSubscribeCycleForAlarm() << "',"
        << " ssrcCheck='" << GbDev->isSsrcCheck() << "'"
        << " WHERE deviceId='" << GbDev->getDeviceId() << "'";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return rc;
}
list<SipServerDeviceInfo> GBDeviceMapper::getDevices()
{
    list<SipServerDeviceInfo> devicelist;
    if (!pDb)
    {
        return devicelist;
    }
    //query gb device 
    //"SELECT * FROM device WHERE deviceId ='"
    ostringstream ds;
    ds << "SELECT *,(SELECT count(*) FROM gb_device_channel WHERE gb_device_channel.deviceId=de.deviceId) as channelCount  FROM gb_device de";
    ds.flush();

    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);

            SipServerDeviceInfo devinfo;
            countcol++;//skip id column
            devinfo.setUuId(azResult[countcol++]);
            devinfo.setDeviceId(azResult[countcol++]);
            devinfo.setName(azResult[countcol++]);
            devinfo.setManufacturer(azResult[countcol++]);
            devinfo.setModel(azResult[countcol++]);
            devinfo.setFirmware(azResult[countcol++]);
            devinfo.setTransport(azResult[countcol++]);
            devinfo.setStreamMode(azResult[countcol++]);
            devinfo.setOnline(atoi(azResult[countcol++]));
            devinfo.setRegisterTime(azResult[countcol++]);
            devinfo.setKeepaliveTime(azResult[countcol++]);
            devinfo.setIp(azResult[countcol++]);
            devinfo.setCreateTime(azResult[countcol++]);
            devinfo.setUpdateTime(azResult[countcol++]);
            devinfo.setPort(atoi(azResult[countcol++]));
            devinfo.setExpires(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForCatalog(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForMobilePosition(atoi(azResult[countcol++]));
            devinfo.setMobilePositionSubmissionInterval(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForAlarm(atoi(azResult[countcol++]));
            devinfo.setHostAddress(azResult[countcol++]);
            devinfo.setCharset(azResult[countcol++]);
            devinfo.setSsrcCheck(azResult[countcol++]);
            devinfo.setChannelCount(atoi(azResult[countcol++]));
            devinfo.setDevAccessProtocal(Device::DEV_ACCESS_GB28181);
            devicelist.push_back(devinfo);
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    for (auto &iter : devicelist)
    {

        list<std::shared_ptr<IDeviceChannel>> chlList = mGBDeviceChannleMapper.queryChannelByDeviceId(iter.getDeviceId());
        iter.setChannelList(chlList);
    }
    return devicelist;
}
int GBDeviceMapper::del(string deviceId)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //add gb device
    ostringstream ds;
    ds << "DELETE FROM gb_device WHERE deviceId = '" << deviceId << "'";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return rc;
}
int GBDeviceMapper::outlineForAll()
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //add gb device
    ostringstream ds;
    ds << "UPDATE device SET online=0";
    ds.flush();
    int rc = pDb->Sqlite_exec(ds.str().c_str(), &szErrMsg);
    if (rc != SQLITE_OK)
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return rc;
}
list<SipServerDeviceInfo> GBDeviceMapper::getOnlineDevices()
{
    list<SipServerDeviceInfo> devicelist;
    if (!pDb)
    {
        return devicelist;
    }
    //query gb device 
    //"SELECT * FROM device WHERE deviceId ='"
    ostringstream ds;
    ds << "SELECT * FROM device WHERE online = 1";
    ds.flush();

    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);

            SipServerDeviceInfo devinfo;
            devinfo.setUuId(azResult[countcol++]);
            devinfo.setDeviceId(azResult[countcol++]);
            devinfo.setName(azResult[countcol++]);
            devinfo.setManufacturer(azResult[countcol++]);
            devinfo.setModel(azResult[countcol++]);
            devinfo.setFirmware(azResult[countcol++]);
            devinfo.setTransport(azResult[countcol++]);
            devinfo.setStreamMode(azResult[countcol++]);
            devinfo.setOnline(atoi(azResult[countcol++]));
            devinfo.setRegisterTime(azResult[countcol++]);
            devinfo.setKeepaliveTime(azResult[countcol++]);
            devinfo.setIp(azResult[countcol++]);
            devinfo.setCreateTime(azResult[countcol++]);
            devinfo.setUpdateTime(azResult[countcol++]);
            devinfo.setPort(atoi(azResult[countcol++]));
            devinfo.setExpires(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForCatalog(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForMobilePosition(atoi(azResult[countcol++]));
            devinfo.setMobilePositionSubmissionInterval(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForAlarm(atoi(azResult[countcol++]));
            devinfo.setHostAddress(azResult[countcol++]);
            devinfo.setCharset(azResult[countcol++]);
            devinfo.setSsrcCheck(azResult[countcol++]);
            devinfo.setDevAccessProtocal(Device::DEV_ACCESS_GB28181);
            //devinfo.setChannelCount(atoi(azResult[countcol++]));
            devicelist.push_back(devinfo);
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return devicelist;
}
SipServerDeviceInfo GBDeviceMapper::getDeviceByHostAndPort(string host, int port)
{
    SipServerDeviceInfo devinfo;
    if (!pDb)
    {
        return devinfo;
    }
    //query gb device 
    //"SELECT * FROM device WHERE deviceId ='"
    ostringstream ds;
    ds << "SELECT * FROM device WHERE ip = '"
        << host
        << "' AND port='"
        << port
        << "'";
    ds.flush();

    char* szErrMsg = 0;
    char** azResult = NULL;
    int nrow = 0, ncolumn = 0;
    int rc = pDb->Sqlite_query(ds.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        int countcol = 0;
        for (int i = 0; i < nrow; i++)
        {
            countcol = ncolumn * (i + 1);
            devinfo.setUuId(azResult[countcol++]);
            devinfo.setDeviceId(azResult[countcol++]);
            devinfo.setName(azResult[countcol++]);
            devinfo.setManufacturer(azResult[countcol++]);
            devinfo.setModel(azResult[countcol++]);
            devinfo.setFirmware(azResult[countcol++]);
            devinfo.setTransport(azResult[countcol++]);
            devinfo.setStreamMode(azResult[countcol++]);
            devinfo.setOnline(atoi(azResult[countcol++]));
            devinfo.setRegisterTime(azResult[countcol++]);
            devinfo.setKeepaliveTime(azResult[countcol++]);
            devinfo.setIp(azResult[countcol++]);
            devinfo.setCreateTime(azResult[countcol++]);
            devinfo.setUpdateTime(azResult[countcol++]);
            devinfo.setPort(atoi(azResult[countcol++]));
            devinfo.setExpires(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForCatalog(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForMobilePosition(atoi(azResult[countcol++]));
            devinfo.setMobilePositionSubmissionInterval(atoi(azResult[countcol++]));
            devinfo.setSubscribeCycleForAlarm(atoi(azResult[countcol++]));
            devinfo.setHostAddress(azResult[countcol++]);
            devinfo.setCharset(azResult[countcol++]);
            devinfo.setSsrcCheck(azResult[countcol++]);
            devinfo.setDevAccessProtocal(Device::DEV_ACCESS_GB28181);
            break;
        }
        sqlite3_free_table(azResult);
    }
    else
    {
        if (szErrMsg)
        {
            sqlite3_free(szErrMsg);
        }
    }
    return devinfo;
}

#else

#endif