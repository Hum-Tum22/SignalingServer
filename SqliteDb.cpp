#include <fcntl.h>
#include "rutil/ResipAssert.h"
#include <cstdlib>

#include "rutil/Data.hxx"
#include "rutil/DataStream.hxx"
#include "rutil/Logger.hxx"
#include "rutil/MD5Stream.hxx"
#include "resip/stack/Symbols.hxx"

//#include "repro/AbstractDb.hxx"
#include "SqliteDb.h"
#include "rutil/WinLeakCheck.hxx"
#include "repro/UserStore.hxx"
#include "repro/TlsPeerIdentityStore.hxx"

#include <sstream>

#include "SipServerConfig.h"

using namespace resip;
using namespace repro;
using namespace std;

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST
//#define USE_DBENV   // Required for transaction support

SqliteDb::SqliteDb(const resip::ConfigParse& config,
    const Data& path,
    const Data& dbName) :
    SqlDb(config),
    dbFilePath(path),
    mConn(0),
    bDbPassword(false)
{
    if (!dbFilePath.empty())
    {
#ifdef WIN32
        dbFilePath += '\\';
#else
        dbFilePath += '/';
#endif
    }

    if (dbName.empty())
    {
        DebugLog(<< "No BerkeleyDb prefix specified - using default");
        dbFilePath += "repro";
    }
    else
    {
        dbFilePath += dbName;
    }
    dbFilePath += ".db";

    InfoLog(<< "Using BerkeleyDb prefixed with " << dbFilePath);
    initialize();

    MyServerConfig& svrCfgi = GetSipServerConfig();
    bDbPassword = svrCfgi.getConfigBool("DbPassword", false);
}


SqliteDb::~SqliteDb()
{
    disconnectFromDatabase();
}

void
SqliteDb::initialize() const
{
    connectToDatabase();
    if (isConnected())
    {
        //create tables
        IniTable();
        
    }
    //mSane = true;
}

void
SqliteDb::disconnectFromDatabase() const
{
    if (mConn)
    {
        //ComitCount(1);
        sqlite3_close_v2(mConn);
        mConn = 0;
        setConnected(false);
    }
}

int
SqliteDb::connectToDatabase() const
{
    // Disconnect from database first (if required)
    disconnectFromDatabase();

    // Now try to connect
    resip_assert(mConn == 0);
    resip_assert(isConnected() == false);

    int n = sqlite3_threadsafe();
    int rc = sqlite3_open_v2(dbFilePath.c_str(),
        &mConn,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_SHAREDCACHE, //SQLITE_OPEN_PRIVATECACHE,
        NULL);
    if (rc != SQLITE_OK || mConn == NULL)
    {
        //LogOut("Store", L_ERROR, "Open DB[%s] failed: %d", dbpathname, rc);
        setConnected(false);
        return rc;
    }
    else
    {
        setConnected(true);
        return 0;
    }
}

int
SqliteDb::query(const Data& queryCommand, std::list<std::vector<resip::Data>>& results) const
{
    int rc = 0;

    //initialize();

    DebugLog(<< "MySqlDb::query: executing query: " << queryCommand);

    Lock lock(mMutex);
    if (mConn == 0 || !isConnected())
    {
        rc = connectToDatabase();
    }
    if (rc == 0)
    {
        resip_assert(mConn != 0);
        resip_assert(isConnected());
        char** azResult = NULL;
        char* zErrMsg = 0;
        int nrow = 0, ncolumn = 0;
        rc = sqlite3_get_table(mConn, queryCommand.c_str(), &azResult, &nrow, &ncolumn, &zErrMsg);
        if (rc == SQLITE_OK && nrow > 0)
        {
            int countcol = 0;
            for (int i = 0; i < nrow; i++)
            {
                countcol = ncolumn * (i + 1);
                std::vector<resip::Data> fields;
                for (int j = 0; j < ncolumn; j++)
                {
                    fields.push_back(azResult[countcol++]);
                }
                results.push_back(fields);
            }
            sqlite3_free_table(azResult);
        }
        else
        {
            if (zErrMsg)
            {
                sqlite3_free(zErrMsg);
            }
        }
    }
    return rc;
}

int
SqliteDb::query(const Data& queryCommand) const
{
    std::list<std::vector<resip::Data>> results;
    return query(queryCommand, results);
}

int
SqliteDb::singleResultQuery(const Data& queryCommand, std::vector<Data>& fields) const
{
    StackLog(<< "executing query: " << queryCommand);
    std::list<std::vector<resip::Data>> results;
    int rc = query(queryCommand, results);

    if (rc == 0)
    {
        if (results.size() == 0)
        {
            return rc;
        }
        fields = *results.begin();
    }
    return rc;
}

resip::Data&
SqliteDb::escapeString(const resip::Data& str, resip::Data& escapedStr) const
{
    //特殊字符转义
    //escapedStr.truncate2(mysql_real_escape_string(mConn, (char*)escapedStr.getBuf(str.size() * 2 + 1), str.c_str(), str.size()));
    return escapedStr;
}

bool
SqliteDb::addUser(const AbstractDb::Key& key, const AbstractDb::UserRecord& rec)
{
    Data command;
    {
        DataStream ds(command);
        ds << "INSERT INTO " << tableName(UserTable) << " (user, domain, realm, passwordHash, passwordHashAlt, name, email, forwardAddress)"
            << " VALUES('"
            << rec.user << "', '"
            << rec.domain << "', '"
            << rec.realm << "', '"
            << rec.passwordHash << "', '"
            << rec.passwordHashAlt << "', '"
            << rec.name << "', '"
            << rec.email << "', '"
            << rec.forwardAddress << "')"
            << " ON DUPLICATE KEY UPDATE"
            << " user='" << rec.user
            << "', domain='" << rec.domain
            << "', realm='" << rec.realm
            << "', passwordHash='" << rec.passwordHash
            << "', passwordHashAlt='" << rec.passwordHashAlt
            << "', name='" << rec.name
            << "', email='" << rec.email
            << "', forwardAddress='" << rec.forwardAddress
            << "'";
    }
    return query(command) == 0;
}


AbstractDb::UserRecord
SqliteDb::getUser(const AbstractDb::Key& key) const
{
    AbstractDb::UserRecord  ret;

    Data command;
    {
        DataStream ds(command);
        ds << "SELECT user, domain, realm, passwordHash, passwordHashAlt, name, email, forwardAddress FROM " << tableName(UserTable) << " ";
        userWhereClauseToDataStream(key, ds);
    }

    std::list<std::vector<resip::Data>> results;
    if (query(command, results) != 0)
    {
        return ret;
    }

    if (results.size() == 0)
    {
        ErrLog(<< "getUser store result failed: error");
        return ret;
    }

    std::vector<resip::Data>& row = *results.begin();
    int col = 0;
    ret.user = Data(row[col++]);
    ret.domain = Data(row[col++]);
    ret.realm = Data(row[col++]);
    ret.passwordHash = Data(row[col++]);
    ret.passwordHashAlt = Data(row[col++]);
    ret.name = Data(row[col++]);
    ret.email = Data(row[col++]);
    ret.forwardAddress = Data(row[col++]);

    return ret;
}


resip::Data
SqliteDb::getUserAuthInfo(const AbstractDb::Key& key) const
{
    std::vector<Data> ret;

    Data user;
    Data domain;
    Data command;
    {
        DataStream ds(command);
        UserStore::getUserAndDomainFromKey(key, user, domain);
        ds << "SELECT passwordHash FROM " << tableName(UserTable) << " WHERE user = '" << user << "' AND domain = '" << domain << "' ";

        // Note: domain is empty when querying for HTTP admin user - for this special user, 
        // we will only check the repro db, by not adding the UNION statement below
        if (!mCustomUserAuthQuery.empty() && !domain.empty())
        {
            ds << " UNION " << mCustomUserAuthQuery;
            ds.flush();
            command.replace("$user", user);
            command.replace("$domain", domain);
        }
    }

    if (singleResultQuery(command, ret) != 0 || ret.size() == 0)
    {
        MD5Stream a1;
        a1 << user
            << Symbols::COLON
            << domain
            << Symbols::COLON;
        if (bDbPassword)
        {
            MyServerConfig& svrCfgi = GetSipServerConfig();
            a1 << svrCfgi.getConfigData("password", "12345", true);
        }
        else
        {
            a1 << "12345";
        }
        a1.flush();
        //rec.passwordHash = a1.getHex();
        ret.push_back(a1.getHex());

        // Some UAs might calculate A1
        // using user@domain:realm:password
        // so we store the hash of that permutation too
       /* MD5Stream a1b;
        a1b << username << Symbols::AT_SIGN << domain
            << Symbols::COLON
            << realm
            << Symbols::COLON
            << password;
        a1b.flush();
        rec.passwordHashAlt = a1b.getHex();*/
        //return Data::Empty;
    }

    DebugLog(<< "Auth password is " << ret.front());

    return ret.front();
}


AbstractDb::Key
SqliteDb::firstUserKey()
{
    // free memory from previous search 
    Data command;
    {
        DataStream ds(command);
        ds << "SELECT user, domain FROM " << tableName(UserTable);
    }
    std::list<std::vector<resip::Data>> results;
    if (query(command, results) != 0)
    {
        return Data::Empty;
    }

    if (results.size() == 0)
    {
        ErrLog(<< "firstUserKey store result failed: error=");
        return Data::Empty;
    }

    return nextUserKey(results);
}


AbstractDb::Key
SqliteDb::nextUserKey(const std::list<std::vector<resip::Data>> &results)
{
    const std::vector<resip::Data>& row = *results.begin();
    
    Data user(row[0]);
    Data domain(row[1]);

    return UserStore::buildKey(user, domain);
}


bool
SqliteDb::addTlsPeerIdentity(const AbstractDb::Key& key, const AbstractDb::TlsPeerIdentityRecord& rec)
{
    Data command;
    {
        DataStream ds(command);
        ds << "INSERT INTO " << tableName(TlsPeerIdentityTable) << " (peerName, tlsPeerIdentity)"
            << " VALUES('"
            << rec.peerName << "', '"
            << rec.authorizedIdentity << "')"
            << " ON DUPLICATE KEY UPDATE"
            << " peerName='" << rec.peerName
            << "', authorizedIdentity ='" << rec.authorizedIdentity
            << "'";
    }
    return query(command) == 0;
}


AbstractDb::TlsPeerIdentityRecord
SqliteDb::getTlsPeerIdentity(const AbstractDb::Key& key) const
{
    AbstractDb::TlsPeerIdentityRecord  ret;

    Data command;
    {
        DataStream ds(command);
        ds << "SELECT peerName, authorizedIdentity FROM " << tableName(TlsPeerIdentityTable) << " ";
        tlsPeerIdentityWhereClauseToDataStream(key, ds);
    }

    std::list<std::vector<resip::Data>> results;
    if (query(command, results) != 0)
    {
        return ret;
    }

    if (results.size() == 0)
    {
        ErrLog(<< "getTlsPeerIdentity store result failed: error=");
        return ret;
    }

    std::vector<resip::Data>& row = *results.begin();
    int col = 0;
    ret.peerName = Data(row[col++]);
    ret.authorizedIdentity = Data(row[col++]);

    return ret;
}


AbstractDb::Key
SqliteDb::firstTlsPeerIdentityKey()
{
    // free memory from previous search
    Data command;
    {
        DataStream ds(command);
        ds << "SELECT peerName, authorizedIdentity FROM " << tableName(TlsPeerIdentityTable);
    }
    std::list<std::vector<resip::Data>> results;
    if (query(command, results) != 0)
    {
        return Data::Empty;
    }

    if (results.size() == 0)
    {
        ErrLog(<< "firstTlsPeerIdentityKey store result failed: error=");
        return Data::Empty;
    }

    return nextTlsPeerIdentityKey(results);
}


AbstractDb::Key
SqliteDb::nextTlsPeerIdentityKey(const std::list<std::vector<resip::Data>>& results)
{
    const std::vector<resip::Data> &row = *results.begin(); 
    Data peerName(row[0]);
    Data authorizedIdentity(row[1]);

    return TlsPeerIdentityStore::buildKey(peerName, authorizedIdentity);
}


bool
SqliteDb::dbWriteRecord(const Table table,
    const resip::Data& pKey,
    const resip::Data& pData)
{
    Data command;

    // Check if there is a secondary key or not and get it's value
    char* secondaryKey;
    unsigned int secondaryKeyLen;
    Data escapedKey;
    if (AbstractDb::getSecondaryKey(table, pKey, pData, (void**)&secondaryKey, &secondaryKeyLen) == 0)
    {
        Data escapedSKey;
        Data sKey(Data::Share, secondaryKey, secondaryKeyLen);
        DataStream ds(command);
        ds << "REPLACE INTO " << tableName(table)
            << " SET attr='" << escapeString(pKey, escapedKey)
            << "', attr2='" << escapeString(sKey, escapedSKey)
            << "', value='" << pData.base64encode()
            << "'";
    }
    else
    {
        DataStream ds(command);
        ds << "REPLACE INTO " << tableName(table)
            << " SET attr='" << escapeString(pKey, escapedKey)
            << "', value='" << pData.base64encode()
            << "'";
    }

    return query(command) == 0;
}

bool
SqliteDb::dbReadRecord(const Table table,
    const resip::Data& pKey,
    resip::Data& pData) const
{
    Data command;
    Data escapedKey;
    {
        DataStream ds(command);
        ds << "SELECT value FROM " << tableName(table)
            << " WHERE attr='" << escapeString(pKey, escapedKey)
            << "'";
    }

    std::list<std::vector<resip::Data>> results;
    if (query(command, results) != 0)
    {
        return false;
    }

    if (results.size() == 0)
    {
        ErrLog(<< "dbReadRecord store result failed: error");
        return false;
    }
    else
    {
        bool success = false;
        std::vector<resip::Data > &row = *results.begin();
        pData = row[0].base64decode();
        success = true;
        return success;
    }
}


resip::Data
SqliteDb::dbNextKey(const Table table, bool first)
{
    std::list<std::vector<resip::Data>> results;
    if (first)
    {
        // free memory from previous search 
        Data command;
        {
            DataStream ds(command);
            ds << "SELECT attr FROM " << tableName(table);
        }
        
        if (query(command, results) != 0)
        {
            return Data::Empty;
        }

        if (results.size() == 0)
        {
            ErrLog(<< "dbNextKey store result failed: error");
            return Data::Empty;
        }
    }
    else
    {
        if (results.size() == 0)
        {
            return Data::Empty;
        }
    }
    std::vector<resip::Data>& row = *results.begin();

    return row[0];
}


bool
SqliteDb::dbNextRecord(const Table table,
    const resip::Data& key,
    resip::Data& data,
    bool forUpdate,  // specifying to add SELECT ... FOR UPDATE so the rows are locked
    bool first)  // return false if no more
{
    std::list<std::vector<resip::Data>> results;
    if (first)
    {
        // free memory from previous search 
        
        Data command;
        {
            DataStream ds(command);
            ds << "SELECT value FROM " << tableName(table);
            if (!key.empty())
            {
                Data escapedKey;
                // dbNextRecord is used to iterator through database tables that support duplication records
                // it is only appropriate for MySQL tables that contain the attr2 non-unique index (secondary key)
                ds << " WHERE attr2='" << escapeString(key, escapedKey) << "'";
            }
            if (forUpdate)
            {
                ds << " FOR UPDATE";
            }
        }
        
        if (query(command, results) != 0)
        {
            return false;
        }

        if (results.size() == 0)
        {
            ErrLog(<< "dbNextRecord store result failed: error");
            return false;
        }
    }

    if (results.size() == 0)
    {
        return false;
    }

    std::vector<resip::Data>& row = *results.begin();

    data = row[0].base64decode();

    return true;
}

bool
SqliteDb::dbBeginTransaction(const Table table)
{
    Data command("SET SESSION TRANSACTION ISOLATION LEVEL REPEATABLE READ");
    if (query(command) == 0)
    {
        command = "START TRANSACTION";
        return query(command) == 0;
    }
    return false;
}

void
SqliteDb::userWhereClauseToDataStream(const Key& key, DataStream& ds) const
{
    Data user;
    Data domain;
    UserStore::getUserAndDomainFromKey(key, user, domain);
    ds << " WHERE user='" << user
        << "' AND domain='" << domain
        << "'";
}

void
SqliteDb::tlsPeerIdentityWhereClauseToDataStream(const Key& key, DataStream& ds) const
{
    Data peerName;
    Data authorizedIdentity;
    TlsPeerIdentityStore::getTlsPeerIdentityFromKey(key, peerName, authorizedIdentity);
    ds << " WHERE peerName='" << peerName
        << "' AND authorizedIdentity='" << authorizedIdentity
        << "'";
}

int SqliteDb::IsColumExist(char* colName, char* tableName)
{
    char* szErrMsg = 0;
    ostringstream ss;
    ss << "select " << colName << " from " << tableName << " limit 0,1; ";
    ss.flush();
    char** azResult;
    int nrow = 0, ncolumn = 0;
    int rc = sqlite3_get_table(mConn, ss.str().c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
    if (rc == SQLITE_OK)
    {
        sqlite3_free_table(azResult);
        return 0;
    }
    else
    {
        sqlite3_free(szErrMsg);
        return -1;
    }
}

int SqliteDb::AddColumByType(int type, char* colName, char* tableName)
{
    if (IsColumExist(colName, tableName) != 0)
    {
        char sqlString[128] = { 0 };
        memset(sqlString, 0, 128);
        /*if (type == SDB_C_INT)
            snprintf(sqlString, 128, "ALTER TABLE %s ADD %s INT DEFAULT -1;", tableName, colName);
        else if (type == SDB_C_INT0)
            snprintf(sqlString, 128, "ALTER TABLE %s ADD %s INT DEFAULT 0;", tableName, colName);
        else if (type == SDB_C_FLOAT)
            snprintf(sqlString, 128, "ALTER TABLE %s ADD %s FLOAT DEFAULT 0;", tableName, colName);
        else if (type == SDB_C_VCHAR)
            snprintf(sqlString, 128, "ALTER TABLE %s ADD %s VARCHAR;", tableName, colName);
        else if (type == SDB_C_INT64)
            snprintf(sqlString, 128, "ALTER TABLE %s ADD %s INT64 DEFAULT -1;", tableName, colName);*/
        char* szErrMsg = 0;
        int rc = sqlite3_exec(mConn, sqlString, 0, 0, &szErrMsg);
        if (rc != SQLITE_OK)
            sqlite3_free(szErrMsg);
    }
    return 0;
}
bool SqliteDb::BeginTransaction()
{
    Data command("begin transaction");
    if (query(command) == 0)
    {
        return true;
    }
    return false;
}
bool SqliteDb::CommitTransaction()
{
    Data command("commit transaction");
    if (query(command) == 0)
    {
        return true;
    }
    return false;
}
bool SqliteDb::RollbackTransaction()
{
    Data command("rollback transaction");
    if (query(command) == 0)
    {
        return true;
    }
    return false;
}
void SqliteDb::IniTable() const
{
    //mGBDevMapper.IniTable();
}
//SipServerDeviceInfo& SqliteDb::queryDevice(string deviceid)
//{
//    SipServerDeviceInfo devinfo;
//    //query gb device 
//    Data command;
//    //"SELECT * FROM device WHERE deviceId = #{deviceId}"
//    DataStream ds(command);
//    ds << "SELECT * FROM gb_device WHERE deviceId ='"
//        << deviceid
//        << "'";
//    ds.flush();
//
//    char* szErrMsg = 0;
//    char** azResult = NULL;
//    int nrow = 0, ncolumn = 0;
//    int rc = sqlite3_get_table(mConn, command.c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
//    if (rc == SQLITE_OK)
//    {
//        int countcol = 0;
//        for (int i = 0; i < nrow; i++)
//        {
//            countcol = ncolumn * (i + 1);
//            devinfo.setUuId(azResult[countcol]++);
//            devinfo.setDeviceId(azResult[countcol]++);
//            devinfo.setName(azResult[countcol]++);
//            devinfo.setManufacturer(azResult[countcol]++);
//            devinfo.setModel(azResult[countcol]++);
//            devinfo.setFirmware(azResult[countcol]++);
//            devinfo.setTransport(azResult[countcol]++);
//            devinfo.setStreamMode(azResult[countcol]++);
//            devinfo.setOnline(atoi(azResult[countcol]++));
//            devinfo.setRegisterTime(azResult[countcol]++);
//            devinfo.setKeepaliveTime(azResult[countcol]++);
//            devinfo.setIp(azResult[countcol]++);
//            devinfo.setCreateTime(azResult[countcol]++);
//            devinfo.setUpdateTime(azResult[countcol]++);
//            devinfo.setPort(atoi(azResult[countcol]++));
//            devinfo.setExpires(atoi(azResult[countcol]++));
//            devinfo.setSubscribeCycleForCatalog(atoi(azResult[countcol]++));
//            devinfo.setSubscribeCycleForMobilePosition(atoi(azResult[countcol]++));
//            devinfo.setMobilePositionSubmissionInterval(atoi(azResult[countcol]++));
//            devinfo.setSubscribeCycleForAlarm(atoi(azResult[countcol]++));
//            devinfo.setHostAddress(azResult[countcol]++);
//            devinfo.setCharset(azResult[countcol]++); 
//            devinfo.setSsrcCheck(azResult[countcol]++);
//            break;
//        }
//        sqlite3_free_table(azResult);
//    }
//    else
//    {
//        if (szErrMsg)
//        {
//            sqlite3_free(szErrMsg);
//        }
//    }
//    
//    return devinfo;
//}
//int SqliteDb::addDevice(SipServerDeviceInfo device)
//{
//    char* szErrMsg = 0;
//    //add gb device
//    Data command;
//    DataStream ds(command);
//    ds << "INSERT INTO gb_device ("
//        << "uuid, "
//        << "deviceId, "
//        << "name, "
//        << "manufacturer, "
//        << "model, "
//        << "firmware, "
//        << "transport,"
//        << "streamMode,"
//        << "online,"
//        << "registerTime,"
//        << "keepaliveTime,"
//        << "ip,"
//        << "createTime,"
//        << "updateTime,"
//        << "port,"
//        << "expires,"
//        << "subscribeCycleForCatalog,"
//        << "subscribeCycleForMobilePosition,"
//        << "mobilePositionSubmissionInterval,"
//        << "subscribeCycleForAlarm,"
//        << "hostAddress,"
//        << "ssrcCheck,"
//        << "charset,"
//        << ") VALUES ("
//        << device.getUuId()
//        << device.getDeviceId()
//        << device.getName()
//        << device.getManufacturer()
//        << device.getModel()
//        << device.getFirmware()
//        << device.getTransport()
//        << device.getStreamMode()
//        << device.getOnline()
//        << device.getRegisterTime()
//        << device.getKeepaliveTime()
//        << device.getIp()
//        << device.getCreateTime()
//        << device.getUpdateTime()
//        << device.getPort()
//        << device.getExpires()
//        << device.getSubscribeCycleForCatalog()
//        << device.getSubscribeCycleForMobilePosition()
//        << device.getMobilePositionSubmissionInterval()
//        << device.getSubscribeCycleForAlarm()
//        << device.getHostAddress()
//        << device.getCharset()
//        << device.isSsrcCheck()
//        << ")";
//    ds.flush();
//    int rc = sqlite3_exec(mConn, command.c_str(), 0, 0, &szErrMsg);
//    if (rc != SQLITE_OK)
//    {
//        if (szErrMsg)
//        {
//            sqlite3_free(szErrMsg);
//        }
//    }
//    return rc;
//}
//int SqliteDb::updateDevice(SipServerDeviceInfo device)
//{
//    char* szErrMsg = 0;
//    //add gb device
//    Data command;
//    DataStream ds(command);
//    ds << "UPDATE gb_device SET "
//        << " updateTime='" << device.getUpdateTime() << "'"
//        << " name='" << device.getName() << "'"
//        << " manufacturer='" << device.getManufacturer() << "'"
//        << " model='" << device.getModel() << "'"
//        << " firmware='" << device.getFirmware() << "'"
//        << " transport='" << device.getTransport() << "'"
//        << " streamMode='" << device.getStreamMode() << "'"
//        << " ip='" << device.getIp() << "'"
//        << " port='" << device.getPort() << "'"
//        << " hostAddress='" << device.getHostAddress() << "'"
//        << " online='" << device.getOnline() << "'"
//        << " registerTime='" << device.getRegisterTime() << "'"
//        << " keepaliveTime='" << device.getKeepaliveTime() << "'"
//        << " expires='" << device.getExpires() << "'"
//        << " charset='" << device.getCharset() << "'"
//        << " subscribeCycleForCatalog='" << device.getSubscribeCycleForCatalog() << "'"
//        << " subscribeCycleForMobilePosition='" << device.getSubscribeCycleForMobilePosition() << "'"
//        << " mobilePositionSubmissionInterval='" << device.getMobilePositionSubmissionInterval() << "'"
//        << " subscribeCycleForAlarm='" << device.getSubscribeCycleForAlarm() << "'"
//        << " ssrcCheck='" << device.isSsrcCheck() << "'"
//        << " WHERE deviceId='" << device.getDeviceId() << "'";
//    ds.flush();
//    int rc = sqlite3_exec(mConn, command.c_str(), 0, 0, &szErrMsg);
//    if (rc != SQLITE_OK)
//    {
//        if (szErrMsg)
//        {
//            sqlite3_free(szErrMsg);
//        }
//    }
//    return rc;
//}
//int SqliteDb::delDevice(string deviceid)
//{
//    char* szErrMsg = 0;
//    //add gb device
//    Data command;
//    DataStream ds(command);
//    ds << "DELETE FROM gb_device WHERE deviceId = '" << deviceid << "'";
//    ds.flush();
//    int rc = sqlite3_exec(mConn, command.c_str(), 0, 0, &szErrMsg);
//    if (rc != SQLITE_OK)
//    {
//        if (szErrMsg)
//        {
//            sqlite3_free(szErrMsg);
//        }
//    }
//    return rc;
//}
//list<IDeviceChannel>& SqliteDb::queryOnlineChannelsByDeviceId(string deviceid)
//{
//    list<IDeviceChannel> DevChlList;
//    //通道
//    Data command;
//    //"SELECT * FROM device WHERE deviceId = #{deviceId}"
//    DataStream ds(command);
//    ds << "SELECT * FROM gb_devicechannel WHERE deviceId ='"
//        << deviceid
//        << "'";
//    ds.flush();
//
//    char* szErrMsg = 0;
//    char** azResult = NULL;
//    int nrow = 0, ncolumn = 0;
//    int rc = sqlite3_get_table(mConn, command.c_str(), &azResult, &nrow, &ncolumn, &szErrMsg);
//    if (rc == SQLITE_OK)
//    {
//        int countcol = 0;
//        for (int i = 0; i < nrow; i++)
//        {
//            countcol = ncolumn * (i + 1);
//            devinfo.setUuId(azResult[countcol]++);
//            devinfo.setDeviceId(azResult[countcol]++);
//            devinfo.setName(azResult[countcol]++);
//            devinfo.setManufacturer(azResult[countcol]++);
//            devinfo.setModel(azResult[countcol]++);
//            devinfo.setFirmware(azResult[countcol]++);
//            devinfo.setTransport(azResult[countcol]++);
//            devinfo.setStreamMode(azResult[countcol]++);
//            devinfo.setOnline(atoi(azResult[countcol]++));
//            devinfo.setRegisterTime(azResult[countcol]++);
//            devinfo.setKeepaliveTime(azResult[countcol]++);
//            devinfo.setIp(azResult[countcol]++);
//            devinfo.setCreateTime(azResult[countcol]++);
//            devinfo.setUpdateTime(azResult[countcol]++);
//            devinfo.setPort(atoi(azResult[countcol]++));
//            devinfo.setExpires(atoi(azResult[countcol]++));
//            devinfo.setSubscribeCycleForCatalog(atoi(azResult[countcol]++));
//            devinfo.setSubscribeCycleForMobilePosition(atoi(azResult[countcol]++));
//            devinfo.setMobilePositionSubmissionInterval(atoi(azResult[countcol]++));
//            devinfo.setSubscribeCycleForAlarm(atoi(azResult[countcol]++));
//            devinfo.setHostAddress(azResult[countcol]++);
//            devinfo.setCharset(azResult[countcol]++);
//            devinfo.setSsrcCheck(azResult[countcol]++);
//            break;
//        }
//        sqlite3_free_table(azResult);
//    }
//    else
//    {
//        if (szErrMsg)
//        {
//            sqlite3_free(szErrMsg);
//        }
//    }
//    return 
//}
int SqliteDb::Sqlite_query(const char* zSql, char*** pazResult, int* pnRow, int* pnColumn, char** pzErrMsg)
{
    return sqlite3_get_table(mConn, zSql, pazResult, pnRow, pnColumn, pzErrMsg);
}
int SqliteDb::Sqlite_exec(const char* zSql, char** pzErrMsg)
{
    return sqlite3_exec(mConn, zSql, 0, 0, pzErrMsg);
}