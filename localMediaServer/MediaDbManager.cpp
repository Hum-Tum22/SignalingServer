
#include "MediaDbManager.h"

void MediaServerDbManagr::IniTable()
{
    if (!pDb)
    {
        return;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "CREATE TABLE `media_server` ("
        << "`id` varchar(255) NOT NULL,"
        << "`ip` varchar(50) NOT NULL,"
        << "`hookIp` varchar(50) NOT NULL,"
        << "`sdpIp` varchar(50) NOT NULL,"
        << "`streamIp` varchar(50) NOT NULL,"
        << "`httpPort` int(11) NOT NULL,"
        << "`httpSSlPort` int(11) NOT NULL,"
        << "`rtmpPort` int(11) NOT NULL,"
        << "`rtmpSSlPort` int(11) NOT NULL,"
        << "`rtpProxyPort` int(11) NOT NULL,"
        << "`rtspPort` int(11) NOT NULL,"
        << "`rtspSSLPort` int(11) NOT NULL,"
        << "`autoConfig` int(11) NOT NULL,"
        << "`secret` varchar(50) NOT NULL,"
        << "`streamNoneReaderDelayMS` int(11) NOT NULL,"
        << "`rtpEnable` int(11) NOT NULL,"
        << "`rtpPortRange` varchar(50) NOT NULL,"
        << "`sendRtpPortRange` varchar(50) NOT NULL,"
        << "`recordAssistPort` int(11) NOT NULL,"
        << "`defaultServer` int(11) NOT NULL,"
        << "`createTime` varchar(50) NOT NULL,"
        << "`updateTime` varchar(50) NOT NULL,"
        << "`hookAliveInterval` int(11) NOT NULL,"
        << "PRIMARY KEY(`id`) USING BTREE,"
        << "UNIQUE KEY `media_server_i` (`ip`,`httpPort`) USING BTREE"
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
        //´´½¨Ë÷Òý
        rc = pDb->Sqlite_exec("CREATE UNIQUE INDEX IF NOT EXISTS `media_server_i` on media_server(`ip`,`httpPort`)", &szErrMsg);
        if (rc != SQLITE_OK)
        {
            if (szErrMsg)
            {
                sqlite3_free(szErrMsg);
            }
        }
    }
    return;
}
//Insert("INSERT INTO media_server (" +
      /*  "id, " +
        "ip, " +
        "hookIp, " +
        "sdpIp, " +
        "streamIp, " +
        "httpPort, " +
        "httpSSlPort, " +
        "rtmpPort, " +
        "rtmpSSlPort, " +
        "rtpProxyPort, " +
        "rtspPort, " +
        "rtspSSLPort, " +
        "autoConfig, " +
        "secret, " +
        "streamNoneReaderDelayMS, " +
        "rtpEnable, " +
        "rtpPortRange, " +
        "sendRtpPortRange, " +
        "recordAssistPort, " +
        "defaultServer, " +
        "createTime, " +
        "updateTime, " +
        "hookAliveInterval" +
        ") VALUES " +
        "(" +
        "'${id}', " +
        "'${ip}', " +
        "'${hookIp}', " +
        "'${sdpIp}', " +
        "'${streamIp}', " +
        "${httpPort}, " +
        "${httpSSlPort}, " +
        "${rtmpPort}, " +
        "${rtmpSSlPort}, " +
        "${rtpProxyPort}, " +
        "${rtspPort}, " +
        "${rtspSSLPort}, " +
        "${autoConfig}, " +
        "'${secret}', " +
        "${streamNoneReaderDelayMS}, " +
        "${rtpEnable}, " +
        "'${rtpPortRange}', " +
        "'${sendRtpPortRange}', " +
        "${recordAssistPort}, " +
        "${defaultServer}, " +
        "'${createTime}', " +
        "'${updateTime}', " +
        "${hookAliveInterval})")*/
int MediaServerDbManagr::add(MediaServerItem mediaServerItem)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "INSERT INTO media_server ("
        << "id, "
        << "ip, "
        << "hookIp, "
        << "sdpIp, "
        << "streamIp, "
        << "httpPort, "
        << "httpSSlPort, "
        << "rtmpPort, "
        << "rtmpSSlPort, "
        << "rtpProxyPort, "
        << "rtspPort, "
        << "rtspSSLPort, "
        << "autoConfig, "
        << "secret, "
        << "streamNoneReaderDelayMS, "
        << "rtpEnable, "
        << "rtpPortRange, "
        << "sendRtpPortRange, "
        << "recordAssistPort, "
        << "defaultServer, "
        << "createTime, "
        << "updateTime, "
        << "hookAliveInterval"
        << ") VALUES "
        << "(NULL,"
        << "'" << mediaServerItem.getIp()<< "', "
        << "'" << mediaServerItem.getHookIp() << "', "
        << "'" << mediaServerItem.getSdpIp() << "', "
        << "'" << mediaServerItem.getStreamIp() << "', "
        << "'" << mediaServerItem.getHttpPort() << ", "
        << "'" << mediaServerItem.getHttpSSlPort() << ", "
        << "'" << mediaServerItem.getRtmpPort() << ", "
        << "'" << mediaServerItem.getRtmpSSlPort() << ", "
        << "'" << mediaServerItem.getRtpProxyPort() << ", "
        << "'" << mediaServerItem.getRtspPort() << ", "
        << "'" << mediaServerItem.getRtspSSLPort() << ", "
        << "'" << mediaServerItem.isAutoConfig() << ", "
        << "'" << mediaServerItem.getSecret() << "', "
        << "'" << mediaServerItem.getStreamNoneReaderDelayMS() << ", "
        << "'" << mediaServerItem.isRtpEnable() << ", "
        << "'" << mediaServerItem.getRtpPortRange() << "', "
        << "'" << mediaServerItem.getSendRtpPortRange() << "', "
        << "'" << mediaServerItem.getRecordAssistPort() << ", "
        << "'" << mediaServerItem.isDefaultServer() << ", "
        << "'" << mediaServerItem.getCreateTime() << "', "
        << "'" << mediaServerItem.getUpdateTime() << "', "
        << "'" << mediaServerItem.getHookAliveInterval() << ")";
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

//Update(value = { " <script>" +
       /* "UPDATE media_server " +
        "SET updateTime='${updateTime}'" +
        "<if test=\"ip != null\">, ip='${ip}'</if>" +
        "<if test=\"hookIp != null\">, hookIp='${hookIp}'</if>" +
        "<if test=\"sdpIp != null\">, sdpIp='${sdpIp}'</if>" +
        "<if test=\"streamIp != null\">, streamIp='${streamIp}'</if>" +
        "<if test=\"httpPort != null\">, httpPort=${httpPort}</if>" +
        "<if test=\"httpSSlPort != null\">, httpSSlPort=${httpSSlPort}</if>" +
        "<if test=\"rtmpPort != null\">, rtmpPort=${rtmpPort}</if>" +
        "<if test=\"rtmpSSlPort != null\">, rtmpSSlPort=${rtmpSSlPort}</if>" +
        "<if test=\"rtpProxyPort != null\">, rtpProxyPort=${rtpProxyPort}</if>" +
        "<if test=\"rtspPort != null\">, rtspPort=${rtspPort}</if>" +
        "<if test=\"rtspSSLPort != null\">, rtspSSLPort=${rtspSSLPort}</if>" +
        "<if test=\"autoConfig != null\">, autoConfig=${autoConfig}</if>" +
        "<if test=\"streamNoneReaderDelayMS != null\">, streamNoneReaderDelayMS=${streamNoneReaderDelayMS}</if>" +
        "<if test=\"rtpEnable != null\">, rtpEnable=${rtpEnable}</if>" +
        "<if test=\"rtpPortRange != null\">, rtpPortRange='${rtpPortRange}'</if>" +
        "<if test=\"sendRtpPortRange != null\">, sendRtpPortRange='${sendRtpPortRange}'</if>" +
        "<if test=\"secret != null\">, secret='${secret}'</if>" +
        "<if test=\"recordAssistPort != null\">, recordAssistPort=${recordAssistPort}</if>" +
        "<if test=\"hookAliveInterval != null\">, hookAliveInterval=${hookAliveInterval}</if>" +
        "WHERE id='${id}'" +
        " </script>" })*/
int MediaServerDbManagr::update(MediaServerItem mediaServerItem)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "UPDATE media_server "
        << "SET updateTime='" << mediaServerItem.getUpdateTime() << "'";
    if (!mediaServerItem.getIp().empty())
        ds << ", ip='" << mediaServerItem.getIp() << "'";
    if (!mediaServerItem.getHookIp().empty())
        ds << ", hookIp='" << mediaServerItem.getHookIp() << "'";
    if (!mediaServerItem.getSdpIp().empty())
        ds << ", sdpIp='" << mediaServerItem.getSdpIp() << "'";
    if (!mediaServerItem.getStreamIp().empty())
        ds << ", streamIp='" << mediaServerItem.getStreamIp() << "'";

    ds << ", httpPort='" << (mediaServerItem.getHttpPort() > 0 ? mediaServerItem.getHttpPort() : 0) << "'";
    ds << ", httpSSlPort='" << (mediaServerItem.getHttpSSlPort() > 0 ? mediaServerItem.getHttpSSlPort() : 0) << "'";
    ds << ", rtmpPort='" << (mediaServerItem.getRtmpPort() > 0 ? mediaServerItem.getRtmpPort() : 0) << "'";
    ds << ", rtmpSSlPort='" << (mediaServerItem.getRtmpSSlPort() > 0 ? mediaServerItem.getRtmpSSlPort() : 0) << "'";
    ds << ", rtpProxyPort='" << (mediaServerItem.getRtpProxyPort() > 0 ? mediaServerItem.getRtpProxyPort() : 0) << "'";
    ds << ", rtspPort='" << (mediaServerItem.getRtspPort() > 0 ? mediaServerItem.getRtspPort() : 0) << "'";
    ds << ", rtspSSLPort='" << (mediaServerItem.getRtspSSLPort() > 0 ? mediaServerItem.getRtspSSLPort() : 0) << "'";
    ds << ", autoConfig='" << mediaServerItem.isAutoConfig() << "'";
    ds << ", streamNoneReaderDelayMS='" << (mediaServerItem.getStreamNoneReaderDelayMS() > 0 ? mediaServerItem.getStreamNoneReaderDelayMS() : 0) << "'";
    ds << ", rtpEnable='" << mediaServerItem.isRtpEnable() << "'";
    if (!mediaServerItem.getRtpPortRange().empty())
        ds << ", rtpPortRange='" << mediaServerItem.getRtpPortRange() << "'";
    if (!mediaServerItem.getSendRtpPortRange().empty())
        ds << ", sendRtpPortRange='" << mediaServerItem.getSendRtpPortRange() << "'";
    if (!mediaServerItem.getSecret().empty())
        ds << ", secret='" << mediaServerItem.getSecret() << "'";
    ds << ", recordAssistPort='" << (mediaServerItem.getRecordAssistPort() > 0 ? mediaServerItem.getRecordAssistPort() : 0) << "'";
    ds << ", hookAliveInterval='" << (mediaServerItem.getHookAliveInterval() > 0 ? mediaServerItem.getHookAliveInterval() : 0) << "'";
    ds << "WHERE id='" << mediaServerItem.getId() << "'";
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

//Update(value = { " <script>" +
        /*"UPDATE media_server " +
        "SET updateTime='${updateTime}'" +
        "<if test=\"id != null\">, id='${id}'</if>" +
        "<if test=\"hookIp != null\">, hookIp='${hookIp}'</if>" +
        "<if test=\"sdpIp != null\">, sdpIp='${sdpIp}'</if>" +
        "<if test=\"streamIp != null\">, streamIp='${streamIp}'</if>" +
        "<if test=\"httpSSlPort != null\">, httpSSlPort=${httpSSlPort}</if>" +
        "<if test=\"rtmpPort != null\">, rtmpPort=${rtmpPort}</if>" +
        "<if test=\"rtmpSSlPort != null\">, rtmpSSlPort=${rtmpSSlPort}</if>" +
        "<if test=\"rtpProxyPort != null\">, rtpProxyPort=${rtpProxyPort}</if>" +
        "<if test=\"rtspPort != null\">, rtspPort=${rtspPort}</if>" +
        "<if test=\"rtspSSLPort != null\">, rtspSSLPort=${rtspSSLPort}</if>" +
        "<if test=\"autoConfig != null\">, autoConfig=${autoConfig}</if>" +
        "<if test=\"streamNoneReaderDelayMS != null\">, streamNoneReaderDelayMS=${streamNoneReaderDelayMS}</if>" +
        "<if test=\"rtpEnable != null\">, rtpEnable=${rtpEnable}</if>" +
        "<if test=\"rtpPortRange != null\">, rtpPortRange='${rtpPortRange}'</if>" +
        "<if test=\"sendRtpPortRange != null\">, sendRtpPortRange='${sendRtpPortRange}'</if>" +
        "<if test=\"secret != null\">, secret='${secret}'</if>" +
        "<if test=\"recordAssistPort != null\">, recordAssistPort=${recordAssistPort}</if>" +
        "<if test=\"hookAliveInterval != null\">, hookAliveInterval=${hookAliveInterval}</if>" +
        "WHERE ip='${ip}' and httpPort=${httpPort}" +
        " </script>" })*/
int MediaServerDbManagr::updateByHostAndPort(MediaServerItem mediaServerItem)
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "UPDATE media_server "
        << "SET updateTime='" << mediaServerItem.getUpdateTime() << "'";
    if (!mediaServerItem.getHookIp().empty())
        ds << ", hookIp='" << mediaServerItem.getHookIp() << "'";
    if (!mediaServerItem.getSdpIp().empty())
        ds << ", sdpIp='" << mediaServerItem.getSdpIp() << "'";
    if (!mediaServerItem.getStreamIp().empty())
        ds << ", streamIp='" << mediaServerItem.getStreamIp() << "'";

    ds << ", httpSSlPort='" << (mediaServerItem.getHttpSSlPort() > 0 ? mediaServerItem.getHttpSSlPort() : 0) << "'";
    ds << ", rtmpPort='" << (mediaServerItem.getRtmpPort() > 0 ? mediaServerItem.getRtmpPort() : 0) << "'";
    ds << ", rtmpSSlPort='" << (mediaServerItem.getRtmpSSlPort() > 0 ? mediaServerItem.getRtmpSSlPort() : 0) << "'";
    ds << ", rtpProxyPort='" << (mediaServerItem.getRtpProxyPort() > 0 ? mediaServerItem.getRtpProxyPort() : 0) << "'";
    ds << ", rtspPort='" << (mediaServerItem.getRtspPort() > 0 ? mediaServerItem.getRtspPort() : 0) << "'";
    ds << ", rtspSSLPort='" << (mediaServerItem.getRtspSSLPort() > 0 ? mediaServerItem.getRtspSSLPort() : 0) << "'";
    ds << ", autoConfig='" << mediaServerItem.isAutoConfig() << "'";
    ds << ", streamNoneReaderDelayMS='" << (mediaServerItem.getStreamNoneReaderDelayMS() > 0 ? mediaServerItem.getStreamNoneReaderDelayMS() : 0) << "'";
    ds << ", rtpEnable='" << mediaServerItem.isRtpEnable() << "'";
    if (!mediaServerItem.getRtpPortRange().empty())
        ds << ", rtpPortRange='" << mediaServerItem.getRtpPortRange() << "'";
    if (!mediaServerItem.getSendRtpPortRange().empty())
        ds << ", sendRtpPortRange='" << mediaServerItem.getSendRtpPortRange() << "'";
    if (!mediaServerItem.getSecret().empty())
        ds << ", secret='" << mediaServerItem.getSecret() << "'";
    ds << ", recordAssistPort='" << (mediaServerItem.getRecordAssistPort() > 0 ? mediaServerItem.getRecordAssistPort() : 0) << "'";
    ds << ", hookAliveInterval='" << (mediaServerItem.getHookAliveInterval() > 0 ? mediaServerItem.getHookAliveInterval() : 0) << "'";
    ds << "WHERE ip='" << mediaServerItem.getIp() << "' and httpPort='" << mediaServerItem.getHttpPort() << "'";
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

//Select("SELECT * FROM media_server WHERE id='${id}'")
MediaServerItem MediaServerDbManagr::queryOne(std::string id)
{
    MediaServerItem mediaItem;
    ostringstream ds;
    ds << "SELECT * FROM media_server WHERE id='" << id << "'";
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

            //countcol++;//skip id column
            mediaItem.setId(azResult[countcol++]);
            mediaItem.setIp(azResult[countcol++]);
            mediaItem.setHookIp(azResult[countcol++]);
            mediaItem.setSdpIp(azResult[countcol++]);
            mediaItem.setStreamIp(azResult[countcol++]);
            mediaItem.setHttpPort(atoi(azResult[countcol++]));
            mediaItem.setHttpSSlPort(atoi(azResult[countcol++]));
            mediaItem.setRtmpPort(atoi(azResult[countcol++]));
            mediaItem.setRtmpSSlPort(atoi(azResult[countcol++]));
            mediaItem.setRtpProxyPort(atoi(azResult[countcol++]));
            mediaItem.setRtspPort(atoi(azResult[countcol++]));
            mediaItem.setRtspSSLPort(atoi(azResult[countcol++]));
            mediaItem.setAutoConfig(atoi(azResult[countcol++]));
            mediaItem.setSecret(azResult[countcol++]);
            mediaItem.setStreamNoneReaderDelayMS(atoi(azResult[countcol++]));
            mediaItem.setRtpEnable(atoi(azResult[countcol++]));
            mediaItem.setRtpPortRange(azResult[countcol++]);
            mediaItem.setSendRtpPortRange(azResult[countcol++]);
            mediaItem.setRecordAssistPort(atoi(azResult[countcol++]));
            mediaItem.setDefaultServer(atoi(azResult[countcol++]));
            mediaItem.setCreateTime(azResult[countcol++]);
            mediaItem.setUpdateTime(azResult[countcol++]);
            mediaItem.setHookAliveInterval(atoi(azResult[countcol++]));
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
    return mediaItem;
}

//Select("SELECT * FROM media_server")
std::list<MediaServerItem> MediaServerDbManagr::queryAll()
{
    std::list<MediaServerItem> mediaList;
    ostringstream ds;
    ds << "SELECT * FROM media_server";
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
            MediaServerItem mediaItem;
            //countcol++;//skip id column
            mediaItem.setId(azResult[countcol++]);
            mediaItem.setIp(azResult[countcol++]);
            mediaItem.setHookIp(azResult[countcol++]);
            mediaItem.setSdpIp(azResult[countcol++]);
            mediaItem.setStreamIp(azResult[countcol++]);
            mediaItem.setHttpPort(atoi(azResult[countcol++]));
            mediaItem.setHttpSSlPort(atoi(azResult[countcol++]));
            mediaItem.setRtmpPort(atoi(azResult[countcol++]));
            mediaItem.setRtmpSSlPort(atoi(azResult[countcol++]));
            mediaItem.setRtpProxyPort(atoi(azResult[countcol++]));
            mediaItem.setRtspPort(atoi(azResult[countcol++]));
            mediaItem.setRtspSSLPort(atoi(azResult[countcol++]));
            mediaItem.setAutoConfig(atoi(azResult[countcol++]));
            mediaItem.setSecret(azResult[countcol++]);
            mediaItem.setStreamNoneReaderDelayMS(atoi(azResult[countcol++]));
            mediaItem.setRtpEnable(atoi(azResult[countcol++]));
            mediaItem.setRtpPortRange(azResult[countcol++]);
            mediaItem.setSendRtpPortRange(azResult[countcol++]);
            mediaItem.setRecordAssistPort(atoi(azResult[countcol++]));
            mediaItem.setDefaultServer(atoi(azResult[countcol++]));
            mediaItem.setCreateTime(azResult[countcol++]);
            mediaItem.setUpdateTime(azResult[countcol++]);
            mediaItem.setHookAliveInterval(atoi(azResult[countcol++]));
            mediaList.push_back(mediaItem);
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
    return mediaList;
}

//Delete("DELETE FROM media_server WHERE id='${id}'")
void MediaServerDbManagr::delOne(std::string id)
{
    if (!pDb)
    {
        return;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "DELETE FROM media_server WHERE id='" << id << "'";
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

//Select("DELETE FROM media_server WHERE ip='${host}' and httpPort=${port}")
void MediaServerDbManagr::delOneByIPAndPort(std::string host, int port)
{
    if (!pDb)
    {
        return;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "DELETE FROM media_server WHERE ip='" << host << "' and httpPort='" << port << "'";
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

//Delete("DELETE FROM media_server WHERE defaultServer=1")
int MediaServerDbManagr::delDefault()
{
    if (!pDb)
    {
        return -1;
    }
    char* szErrMsg = 0;
    //gb device channel table
    ostringstream ds;
    ds << "DELETE FROM media_server WHERE defaultServer='1'";
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

//Select("SELECT * FROM media_server WHERE ip='${host}' and httpPort=${port}")
MediaServerItem MediaServerDbManagr::queryOneByHostAndPort(std::string host, int port)
{
    MediaServerItem mediaItem;
    ostringstream ds;
    ds << "SELECT * FROM media_server WHERE ip='" << host << "' and httpPort='" << port << "'";
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

            //countcol++;//skip id column
            mediaItem.setId(azResult[countcol++]);
            mediaItem.setIp(azResult[countcol++]);
            mediaItem.setHookIp(azResult[countcol++]);
            mediaItem.setSdpIp(azResult[countcol++]);
            mediaItem.setStreamIp(azResult[countcol++]);
            mediaItem.setHttpPort(atoi(azResult[countcol++]));
            mediaItem.setHttpSSlPort(atoi(azResult[countcol++]));
            mediaItem.setRtmpPort(atoi(azResult[countcol++]));
            mediaItem.setRtmpSSlPort(atoi(azResult[countcol++]));
            mediaItem.setRtpProxyPort(atoi(azResult[countcol++]));
            mediaItem.setRtspPort(atoi(azResult[countcol++]));
            mediaItem.setRtspSSLPort(atoi(azResult[countcol++]));
            mediaItem.setAutoConfig(atoi(azResult[countcol++]));
            mediaItem.setSecret(azResult[countcol++]);
            mediaItem.setStreamNoneReaderDelayMS(atoi(azResult[countcol++]));
            mediaItem.setRtpEnable(atoi(azResult[countcol++]));
            mediaItem.setRtpPortRange(azResult[countcol++]);
            mediaItem.setSendRtpPortRange(azResult[countcol++]);
            mediaItem.setRecordAssistPort(atoi(azResult[countcol++]));
            mediaItem.setDefaultServer(atoi(azResult[countcol++]));
            mediaItem.setCreateTime(azResult[countcol++]);
            mediaItem.setUpdateTime(azResult[countcol++]);
            mediaItem.setHookAliveInterval(atoi(azResult[countcol++]));
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
    return mediaItem;
}

//Select("SELECT * FROM media_server WHERE defaultServer=1")
MediaServerItem MediaServerDbManagr::queryDefault()
{
    MediaServerItem mediaItem;
    ostringstream ds;
    ds << "SELECT * FROM media_server WHERE defaultServer='1'";
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

            //countcol++;//skip id column
            mediaItem.setId(azResult[countcol++]);
            mediaItem.setIp(azResult[countcol++]);
            mediaItem.setHookIp(azResult[countcol++]);
            mediaItem.setSdpIp(azResult[countcol++]);
            mediaItem.setStreamIp(azResult[countcol++]);
            mediaItem.setHttpPort(atoi(azResult[countcol++]));
            mediaItem.setHttpSSlPort(atoi(azResult[countcol++]));
            mediaItem.setRtmpPort(atoi(azResult[countcol++]));
            mediaItem.setRtmpSSlPort(atoi(azResult[countcol++]));
            mediaItem.setRtpProxyPort(atoi(azResult[countcol++]));
            mediaItem.setRtspPort(atoi(azResult[countcol++]));
            mediaItem.setRtspSSLPort(atoi(azResult[countcol++]));
            mediaItem.setAutoConfig(atoi(azResult[countcol++]));
            mediaItem.setSecret(azResult[countcol++]);
            mediaItem.setStreamNoneReaderDelayMS(atoi(azResult[countcol++]));
            mediaItem.setRtpEnable(atoi(azResult[countcol++]));
            mediaItem.setRtpPortRange(azResult[countcol++]);
            mediaItem.setSendRtpPortRange(azResult[countcol++]);
            mediaItem.setRecordAssistPort(atoi(azResult[countcol++]));
            mediaItem.setDefaultServer(atoi(azResult[countcol++]));
            mediaItem.setCreateTime(azResult[countcol++]);
            mediaItem.setUpdateTime(azResult[countcol++]);
            mediaItem.setHookAliveInterval(atoi(azResult[countcol++]));
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
    return mediaItem;
}