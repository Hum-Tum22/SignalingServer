
#include "ParentPlatformDbManager.h"
#include "../SipServerConfig.h"

#include <string>
#include <sstream>
using namespace repro;


ParentPlatform::ParentPlatform() :id(0), enable(false), serverPort(0), ptz(false), rtcp(false), status(false)
, channelCount(0), shareAllLiveStream(false), catalogSubscribe(false), alarmSubscribe(false)
, mobilePositionSubscribe(false), startOfflinePush(false), catalogGroup(1)
{
}
ParentPlatformDbManager::ParentPlatformDbManager() :pDb(0)
{
    pDb = dynamic_cast<SqliteDb*>(getCurDatabase());
}
void ParentPlatformDbManager::IniTable()
{
    if (!pDb)
    {
        return;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "CREATE TABLE IF NOT EXISTS `parent_platform` ("
        << "`id` int(11) NOT NULL AUTO_INCREMENT,"
        << "`enable` int(11) DEFAULT NULL,"
        << "`name` varchar(255) DEFAULT NULL,"
        << "`serverGBId` varchar(50) NOT NULL,"
        << "`serverGBDomain` varchar(50) DEFAULT NULL,"
        << "`serverIP` varchar(50) DEFAULT NULL,"
        << "`serverPort` int(11) DEFAULT NULL,"
        << "`deviceGBId` varchar(50) NOT NULL,"
        << "`deviceIp` varchar(50) DEFAULT NULL,"
        << "`devicePort` varchar(50) DEFAULT NULL,"
        << "`username` varchar(255) DEFAULT NULL,"
        << "`password` varchar(50) DEFAULT NULL,"
        << "`expires` varchar(50) DEFAULT NULL,"
        << "`keepTimeout` varchar(50) DEFAULT NULL,"
        << "`transport` varchar(50) DEFAULT NULL,"
        << "`characterSet` varchar(50) DEFAULT NULL,"
        << "`catalogId` varchar(50) NOT NULL,"
        << "`ptz` int(11) DEFAULT NULL,"
        << "`rtcp` int(11) DEFAULT NULL,"
        << "`status` bit(1) DEFAULT NULL,"
        << "`shareAllLiveStream` int(11) DEFAULT NULL,"
        << "`startOfflinePush` int(11) DEFAULT '0',"
        << "`administrativeDivision` varchar(50) NOT NULL,"
        << "`catalogGroup` int(11) DEFAULT '1',"
        << "PRIMARY KEY(`id`)"
        << ");";
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
        rc = pDb->Sqlite_exec("CREATE UNIQUE INDEX IF NOT EXISTS `parent_platform_id_uindex` on parent_platform(`id`)", &szErrMsg);
        if (rc != SQLITE_OK)
        {
            if (szErrMsg)
            {
                sqlite3_free(szErrMsg);
                szErrMsg = NULL;
            }
        }
        rc = pDb->Sqlite_exec("CREATE UNIQUE INDEX IF NOT EXISTS `parent_platform_pk` on gb_device_channel(`serverGBId`)", &szErrMsg);
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
////Insert("INSERT INTO parent_platform (enable, name, serverGBId, serverGBDomain, serverIP, serverPort, deviceGBId, deviceIp,  " +
    //    "        devicePort, username, password, expires, keepTimeout, transport, characterSet, ptz, rtcp, " +
    //    "        status, shareAllLiveStream, startOfflinePush, catalogId, administrativeDivision, catalogGroup) " +
    //    "        VALUES (${enable}, '${name}', '${serverGBId}', '${serverGBDomain}', '${serverIP}', ${serverPort}, '${deviceGBId}', '${deviceIp}', " +
    //    "        '${devicePort}', '${username}', '${password}', '${expires}', '${keepTimeout}', '${transport}', '${characterSet}', ${ptz}, ${rtcp}, " +
    //    "        ${status}, ${shareAllLiveStream},  ${startOfflinePush}, #{catalogId}, #{administrativeDivision}, #{catalogGroup})")
int ParentPlatformDbManager::addParentPlatform(ParentPlatform parentPlatform)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "INSERT INTO parent_platform ("
        << "enable,"
        << "name,"
        << "serverGBId,"
        << "serverGBDomain,"
        << "serverIP,"
        << "serverPort,"
        << "deviceGBId,"
        << "deviceIp,"
        << "devicePort,"
        << "username,"
        << "password,"
        << "expires,"
        << "keepTimeout,"
        << "transport,"
        << "characterSet,"
        << "ptz,"
        << "rtcp,"
        << "status,"
        << "shareAllLiveStream,"
        << "startOfflinePush,"
        << "catalogId,"
        << "administrativeDivision,"
        << "catalogGroup)"
        << "VALUES ("
        << "'" << parentPlatform.enable << "',"
        << "'" << parentPlatform.name << "'," 
        << "'" << parentPlatform.serverGBId << "',"
        << "'" << parentPlatform.serverGBDomain << "',"
        << "'" << parentPlatform.serverIP << "',"
        << "'" << parentPlatform.serverPort << "',"
        << "'" << parentPlatform.deviceGBId << "',"
        << "'" << parentPlatform.deviceIp << "',"
        << "'" << parentPlatform.devicePort << "',"
        << "'" << parentPlatform.username << "',"
        << "'" << parentPlatform.password << "',"
        << "'" << parentPlatform.expires << "',"
        << "'" << parentPlatform.keepTimeout << "',"
        << "'" << parentPlatform.transport << "',"
        << "'" << parentPlatform.characterSet << "',"
        << "'" << parentPlatform.ptz << "',"
        << "'" << parentPlatform.rtcp << "',"
        << "'" << parentPlatform.status << "',"
        << "'" << parentPlatform.shareAllLiveStream << "',"
        << "'" << parentPlatform.startOfflinePush << "',"
        << "'" << parentPlatform.catalogId << "',"
        << "'" << parentPlatform.administrativeDivision << "',"
        << "'" << parentPlatform.catalogGroup << "'"
        << ");";
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
int ParentPlatformDbManager::updateParentPlatform(ParentPlatform parentPlatform)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "UPDATE parent_platform "
        << "SET enable='" << parentPlatform.enable << "',"
        << "name='" << parentPlatform.name << "',"
        << "serverGBId='" << parentPlatform.serverGBId << "',"
        << "serverGBDomain='" << parentPlatform.serverGBDomain << "',"
        << "serverIP='" << parentPlatform.serverIP << "',"
        << "serverPort='" << parentPlatform.serverPort << "',"
        << "deviceGBId='" << parentPlatform.deviceGBId << "',"
        << "deviceIp='" << parentPlatform.deviceIp << "',"
        << "devicePort='" << parentPlatform.devicePort << "',"
        << "username='" << parentPlatform.username << "',"
        << "password='" << parentPlatform.password << "',"
        << "expires='" << parentPlatform.expires << "',"
        << "keepTimeout='" << parentPlatform.keepTimeout << "',"
        << "transport='" << parentPlatform.transport << "',"
        << "characterSet='" << parentPlatform.characterSet << "',"
        << "ptz='" << parentPlatform.ptz << "',"
        << "rtcp='" << parentPlatform.rtcp << "',"
        << "status='" << parentPlatform.status << "',"
        << "shareAllLiveStream='" << parentPlatform.shareAllLiveStream << "',"
        << "startOfflinePush='" << parentPlatform.startOfflinePush << "',"
        << "catalogId='" << parentPlatform.catalogId << "',"
        << "administrativeDivision='" << parentPlatform.administrativeDivision << "',"
        << "catalogGroup='" << parentPlatform.catalogGroup << "' "
        << "WHERE id='" << parentPlatform.id << "';";
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

//Delete("DELETE FROM parent_platform WHERE serverGBId=#{serverGBId}")
int ParentPlatformDbManager::delParentPlatform(ParentPlatform parentPlatform)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "DELETE FROM parent_platform WHERE serverGBId='" << parentPlatform.serverGBId << "'";
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
std::list<ParentPlatform> ParentPlatformDbManager::getParentPlatformList()
{
    std::list<ParentPlatform> parentPlatformList;

    ostringstream ds;
    ds << "SELECT *, ((SELECT count(0) "
        << "FROM platform_gb_channel pc "
        << "WHERE pc.platformId = pp.serverGBId) "
        << "(SELECT count(0) "
        << "FROM platform_gb_stream pgs "
        << "WHERE pgs.platformId = pp.serverGBId) "
        << "(SELECT count(0) "
        << "FROM platform_catalog pgc "
        << "WHERE pgc.platformId = pp.serverGBId)) as channelCount "
        << "FROM parent_platform pp ";
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
            ParentPlatform parentPlatform;
            parentPlatform.id = std::atoi(azResult[countcol++]);
            /*parentPlatform.setChannelId(azResult[countcol++]);
            parentPlatform.setName(azResult[countcol++]);
            parentPlatform.setManufacture(azResult[countcol++]);
            parentPlatform.setModel(azResult[countcol++]);
            parentPlatform.setOwner(azResult[countcol++]);
            parentPlatform.setCivilCode(azResult[countcol++]);
            parentPlatform.setBlock(azResult[countcol++]);
            parentPlatform.setAddress(azResult[countcol++]);
            parentPlatform.setParentId(azResult[countcol++]);
            parentPlatform.setSafetyWay(atoi(azResult[countcol++]));
            parentPlatform.setRegisterWay(atoi(azResult[countcol++]));
            parentPlatform.setCertNum(azResult[countcol++]);
            parentPlatform.setCertifiable(atoi(azResult[countcol++]));
            parentPlatform.setErrCode(atoi(azResult[countcol++]));
            parentPlatform.setEndTime(azResult[countcol++]);
            parentPlatform.setSecrecy(azResult[countcol++]);
            parentPlatform.setIpAddress(azResult[countcol++]);
            parentPlatform.setPort(atoi(azResult[countcol++]));
            parentPlatform.setPassword(azResult[countcol++]);
            parentPlatform.setPTZType(atoi(azResult[countcol++]));
            parentPlatform.setStatus(atoi(azResult[countcol++]));
            parentPlatform.setLongitude(atof(azResult[countcol++]));
            parentPlatform.setLatitude(atof(azResult[countcol++]));
            parentPlatform.setStreamId(azResult[countcol++]);
            parentPlatform.setDeviceId(azResult[countcol++]);
            parentPlatform.setParental(atoi(azResult[countcol++]));
            parentPlatform.setHasAudio(atoi(azResult[countcol++]));
            parentPlatform.setCreateTime(azResult[countcol++]);
            parentPlatform.setUpdateTime(azResult[countcol++]);
            parentPlatform.setSubCount(atoi(azResult[countcol++]));*/
            parentPlatformList.push_back(parentPlatform);
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
    return parentPlatformList;
}

//Select("SELECT * FROM parent_platform WHERE enable=#{enable}")
std::list<ParentPlatform> ParentPlatformDbManager::getEnableParentPlatformList(bool enable)
{
    std::list<ParentPlatform> parentPlatformList;

    ostringstream ds;
    ds << "SELECT * FROM parent_platform WHERE enable='" << enable << "'";
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
            ParentPlatform parentPlatform;
            parentPlatform.id = std::atoi(azResult[countcol++]);
            parentPlatform.enable = !!std::atoi(azResult[countcol++]);
            parentPlatform.name = azResult[countcol++];
            parentPlatform.serverGBId = azResult[countcol++];
            parentPlatform.serverGBDomain = azResult[countcol++];
            parentPlatform.serverIP = azResult[countcol++];
            parentPlatform.serverPort = std::atoi(azResult[countcol++]);
            parentPlatform.deviceGBId = azResult[countcol++];
            parentPlatform.deviceIp = azResult[countcol++];
            parentPlatform.devicePort = azResult[countcol++];
            parentPlatform.username = azResult[countcol++];
            parentPlatform.password = azResult[countcol++];
            parentPlatform.expires = std::atoi(azResult[countcol++]);
            parentPlatform.keepTimeout = atoi(azResult[countcol++]);
            parentPlatform.transport = azResult[countcol++];
            parentPlatform.characterSet = azResult[countcol++];
            parentPlatform.catalogId = azResult[countcol++];
            parentPlatform.ptz = std::atoi(azResult[countcol++]);
            parentPlatform.rtcp = std::atoi(azResult[countcol++]);
            parentPlatform.status = std::atoi(azResult[countcol++]);
            parentPlatform.shareAllLiveStream = std::atoi(azResult[countcol++]);
            parentPlatform.startOfflinePush = std::atoi(azResult[countcol++]);
            parentPlatform.administrativeDivision = std::atoi(azResult[countcol++]);
            parentPlatform.catalogGroup = std::atoi(azResult[countcol++]);
            parentPlatformList.push_back(parentPlatform);
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
    return parentPlatformList;
}

//Select("SELECT * FROM parent_platform WHERE serverGBId=#{platformGbId}")
ParentPlatform ParentPlatformDbManager::getParentPlatByServerGBId(std::string platformGbId)
{
    ParentPlatform parentPlatform;

    ostringstream ds;
    ds << "SELECT * FROM parent_platform WHERE serverGBId='" << platformGbId << "'";
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
            parentPlatform.id = std::atoi(azResult[countcol++]);
            parentPlatform.enable = !!std::atoi(azResult[countcol++]);
            parentPlatform.name = azResult[countcol++];
            parentPlatform.serverGBId = azResult[countcol++];
            parentPlatform.serverGBDomain = azResult[countcol++];
            parentPlatform.serverIP = azResult[countcol++];
            parentPlatform.serverPort = std::atoi(azResult[countcol++]);
            parentPlatform.deviceGBId = azResult[countcol++];
            parentPlatform.deviceIp = azResult[countcol++];
            parentPlatform.devicePort = azResult[countcol++];
            parentPlatform.username = azResult[countcol++];
            parentPlatform.password = azResult[countcol++];
            parentPlatform.expires = std::atoi(azResult[countcol++]);
            parentPlatform.keepTimeout = atoi(azResult[countcol++]);
            parentPlatform.transport = azResult[countcol++];
            parentPlatform.characterSet = azResult[countcol++];
            parentPlatform.catalogId = azResult[countcol++];
            parentPlatform.ptz = std::atoi(azResult[countcol++]);
            parentPlatform.rtcp = std::atoi(azResult[countcol++]);
            parentPlatform.status = std::atoi(azResult[countcol++]);
            parentPlatform.shareAllLiveStream = std::atoi(azResult[countcol++]);
            parentPlatform.startOfflinePush = std::atoi(azResult[countcol++]);
            parentPlatform.administrativeDivision = std::atoi(azResult[countcol++]);
            parentPlatform.catalogGroup = std::atoi(azResult[countcol++]);
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
    return parentPlatform;
}

//Select("SELECT * FROM parent_platform WHERE id=#{id}")
ParentPlatform ParentPlatformDbManager::getParentPlatById(int id)
{
    ParentPlatform parentPlatform;

    ostringstream ds;
    ds << "SELECT * FROM parent_platform WHERE id='" << id << "'";
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
            parentPlatform.id = std::atoi(azResult[countcol++]);
            parentPlatform.enable = !!std::atoi(azResult[countcol++]);
            parentPlatform.name = azResult[countcol++];
            parentPlatform.serverGBId = azResult[countcol++];
            parentPlatform.serverGBDomain = azResult[countcol++];
            parentPlatform.serverIP = azResult[countcol++];
            parentPlatform.serverPort = std::atoi(azResult[countcol++]);
            parentPlatform.deviceGBId = azResult[countcol++];
            parentPlatform.deviceIp = azResult[countcol++];
            parentPlatform.devicePort = azResult[countcol++];
            parentPlatform.username = azResult[countcol++];
            parentPlatform.password = azResult[countcol++];
            parentPlatform.expires = std::atoi(azResult[countcol++]);
            parentPlatform.keepTimeout = atoi(azResult[countcol++]);
            parentPlatform.transport = azResult[countcol++];
            parentPlatform.characterSet = azResult[countcol++];
            parentPlatform.catalogId = azResult[countcol++];
            parentPlatform.ptz = std::atoi(azResult[countcol++]);
            parentPlatform.rtcp = std::atoi(azResult[countcol++]);
            parentPlatform.status = std::atoi(azResult[countcol++]);
            parentPlatform.shareAllLiveStream = std::atoi(azResult[countcol++]);
            parentPlatform.startOfflinePush = std::atoi(azResult[countcol++]);
            parentPlatform.administrativeDivision = std::atoi(azResult[countcol++]);
            parentPlatform.catalogGroup = std::atoi(azResult[countcol++]);
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
    return parentPlatform;
}

//Update("UPDATE parent_platform SET status=false")
int ParentPlatformDbManager::outlineForAllParentPlatform()
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "UPDATE parent_platform SET status='0'";
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

//Update("UPDATE parent_platform SET status=#{online} WHERE serverGBId=#{platformGbID}")
int ParentPlatformDbManager::updateParentPlatformStatus(std::string platformGbID, bool online)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "UPDATE parent_platform SET status='" << online << "' WHERE serverGBId='" << platformGbID << "'";
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

//Select("SELECT * FROM parent_platform WHERE shareAllLiveStream=true")
std::list<ParentPlatform> ParentPlatformDbManager::selectAllAhareAllLiveStream()
{
    std::list<ParentPlatform> parentPlatformList;

    ostringstream ds;
    ds << "SELECT * FROM parent_platform WHERE shareAllLiveStream=true";
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
            ParentPlatform parentPlatform;
            parentPlatform.id = std::atoi(azResult[countcol++]);
            parentPlatform.enable = !!std::atoi(azResult[countcol++]);
            parentPlatform.name = azResult[countcol++];
            parentPlatform.serverGBId = azResult[countcol++];
            parentPlatform.serverGBDomain = azResult[countcol++];
            parentPlatform.serverIP = azResult[countcol++];
            parentPlatform.serverPort = std::atoi(azResult[countcol++]);
            parentPlatform.deviceGBId = azResult[countcol++];
            parentPlatform.deviceIp = azResult[countcol++];
            parentPlatform.devicePort = azResult[countcol++];
            parentPlatform.username = azResult[countcol++];
            parentPlatform.password = azResult[countcol++];
            parentPlatform.expires = std::atoi(azResult[countcol++]);
            parentPlatform.keepTimeout = atoi(azResult[countcol++]);
            parentPlatform.transport = azResult[countcol++];
            parentPlatform.characterSet = azResult[countcol++];
            parentPlatform.catalogId = azResult[countcol++];
            parentPlatform.ptz = std::atoi(azResult[countcol++]);
            parentPlatform.rtcp = std::atoi(azResult[countcol++]);
            parentPlatform.status = std::atoi(azResult[countcol++]);
            parentPlatform.shareAllLiveStream = std::atoi(azResult[countcol++]);
            parentPlatform.startOfflinePush = std::atoi(azResult[countcol++]);
            parentPlatform.administrativeDivision = std::atoi(azResult[countcol++]);
            parentPlatform.catalogGroup = std::atoi(azResult[countcol++]);
            parentPlatformList.push_back(parentPlatform);
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
    return parentPlatformList;
}

////Update(value = { " <script>" +
//    "UPDATE parent_platform " +
//    "SET catalogId=#{catalogId}" +
//    "WHERE serverGBId=#{platformId}" +
//    "</script>" })
int ParentPlatformDbManager::setDefaultCatalog(std::string platformId, std::string catalogId)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "UPDATE parent_platform "
        << "SET catalogId='" << catalogId << "' "
        << "WHERE serverGBId='" << platformId << "'";
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

////Select("select 'channel' as name, count(pgc.platformId) count from platform_gb_channel pgc left join device_channel dc on dc.id = pgc.deviceChannelId where  pgc.platformId=#{platformId} and dc.channelId =#{gbId} " +
//"union " +
//"select 'stream' as name, count(pgs.platformId) count from platform_gb_stream pgs left join gb_stream gs on pgs.gbStreamId = gs.gbStreamId where  pgs.platformId=#{platformId} and gs.gbId = #{gbId}")
std::list<ChannelSourceInfo> ParentPlatformDbManager::getChannelSource(std::string platformId, std::string gbId)
{
    std::list<ChannelSourceInfo> channelSourceList;

    ostringstream ds;
    ds << "select 'channel' as name, count(pgc.platformId) count from platform_gb_channel pgc "
        << "left join device_channel dc on dc.id = pgc.deviceChannelId where  "
        << "pgc.platformId='" << platformId << "' and dc.channelId ='" << gbId << "' "
        << "union select 'stream' as name, count(pgs.platformId) count from platform_gb_stream pgs "
        << "left join gb_stream gs on pgs.gbStreamId = gs.gbStreamId where  "
        << "pgs.platformId='" << platformId << "' and gs.gbId = '" << gbId << "'";
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
            ChannelSourceInfo channelSourceInfo;
            channelSourceInfo.name = azResult[countcol++];
            channelSourceInfo.count = std::atoi(azResult[countcol++]);
            channelSourceList.push_back(channelSourceInfo);
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
    return channelSourceList;
}