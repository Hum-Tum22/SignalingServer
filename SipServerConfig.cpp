
#include "SipServerConfig.h"
#include "SqliteDb.h"
#include "rutil/Data.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::TEST
using namespace resip;
using namespace repro;
using namespace regist;

MyServerConfig::MyServerConfig() :mAbstractDb(0)
{
    Data defaultConfigFilename("repro.config");
    try
    {
        char* p = NULL;
        parseConfig(1, &p, defaultConfigFilename);
    }
    catch (BaseException& ex)
    {
        std::cerr << "Error parsing configuration: " << ex << std::endl;
#ifndef WIN32
        syslog(LOG_DAEMON | LOG_CRIT, "%s", ex.getMessage().c_str());
#endif
        return;
    }
    return;
}
AbstractDb* MyServerConfig::getDatabase(int configIndex)
{
    if (mAbstractDb)
        return mAbstractDb;
    ConfigParse::NestedConfigMap m = getConfigNested("Database");
    ConfigParse::NestedConfigMap::iterator it = m.find(configIndex);
    if (it == m.end())
    {
        WarningLog(<< "Failed to find Database settings for index " << configIndex);
        return 0;
    }
    ConfigParse& dbConfig = it->second;
    Data dbType = dbConfig.getConfigData("Type", "");
    dbType.lowercase();
    if (dbType == "sqlite")
    {
        Data path = dbConfig.getConfigData("Path", getConfigData("DatabasePath", "./", true), true);
        Data dbName = "sipserver";
        mAbstractDb = new SqliteDb(dbConfig, path, dbName);
        return mAbstractDb;
    }
    else if (dbType == "mysql")
    {
#ifdef USE_MYSQL
        Data mySQLServer = dbConfig.getConfigData("Host", Data::Empty);
        if (!mySQLServer.empty())
        {
            mAbstractDb = new MySqlDb(dbConfig, mySQLServer,
                dbConfig.getConfigData("User", Data::Empty),
                dbConfig.getConfigData("Password", Data::Empty),
                dbConfig.getConfigData("DatabaseName", Data::Empty),
                dbConfig.getConfigUnsignedLong("Port", 0),
                dbConfig.getConfigData("CustomUserAuthQuery", Data::Empty));
            return mAbstractDb;
        }
#else
        ErrLog(<< "Database" << configIndex << " type MySQL support not compiled into repro");
        return 0;
#endif
    }
    else if (dbType == "postgresql")
    {
#ifdef USE_POSTGRESQL
        Data postgreSQLConnInfo = dbConfig.getConfigData("ConnInfo", Data::Empty);
        Data postgreSQLServer = dbConfig.getConfigData("Host", Data::Empty);
        if (!postgreSQLConnInfo.empty() || !postgreSQLServer.empty())
        {
            mAbstractDb = new PostgreSqlDb(dbConfig, postgreSQLConnInfo, postgreSQLServer,
                dbConfig.getConfigData("User", Data::Empty),
                dbConfig.getConfigData("Password", Data::Empty),
                dbConfig.getConfigData("DatabaseName", Data::Empty),
                dbConfig.getConfigUnsignedLong("Port", 0),
                dbConfig.getConfigData("CustomUserAuthQuery", Data::Empty));
            return mAbstractDb;
        }
#else 
        ErrLog(<< "Database" << configIndex << " type PostgreSQL support not compiled into repro");
        return 0;
#endif
    }
    else
    {
        ErrLog(<< "Database" << configIndex << " type '" << dbType << "' not supported / invalid");
    }
    return 0;
}
regist::AbstractDb* MyServerConfig::CreateDatabase()
{
    if (mAbstractDb)
        return mAbstractDb;
    int defaultDatabaseIndex = getConfigInt("DefaultDatabase", -1);
    if (defaultDatabaseIndex >= 0)
    {
        mAbstractDb = getDatabase(defaultDatabaseIndex);
        if (!mAbstractDb)
        {
            CritLog(<< "Failed to get configuration database");
            //cleanupObjects();
            return NULL;
        }
    }
    else     // Try legacy configuration parameter names
    {
#ifdef USE_MYSQL
        Data mySQLServer;
        mProxyConfig->getConfigValue("MySQLServer", mySQLServer);
        if (!mySQLServer.empty())
        {
            WarningLog(<< "Using deprecated parameter MySQLServer, please update to indexed Database definitions.");
            mAbstractDb = new MySqlDb(*this, mySQLServer,
                this->getConfigData("MySQLUser", Data::Empty),
                this->getConfigData("MySQLPassword", Data::Empty),
                this->getConfigData("MySQLDatabaseName", Data::Empty),
                this->getConfigUnsignedLong("MySQLPort", 0),
                this->getConfigData("MySQLCustomUserAuthQuery", Data::Empty));
        }
#endif
        if (!mAbstractDb)
        {
            //mAbstractDb = new BerkeleyDb(mProxyConfig->getConfigData("DatabasePath", "./", true));
            mAbstractDb = getDatabase(defaultDatabaseIndex);
        }
    }
    return mAbstractDb;
}
static MyServerConfig *g_pServerConfig = NULL;
MyServerConfig& GetSipServerConfig()
{
    if (g_pServerConfig == NULL)
        g_pServerConfig = new MyServerConfig();
    return *g_pServerConfig;
}
AbstractDb* g_mAbstractDb = NULL;
AbstractDb* getCurDatabase()
{
    if (g_mAbstractDb)
        return g_mAbstractDb;
    g_mAbstractDb = GetSipServerConfig().CreateDatabase();
    return g_mAbstractDb;
//    int defaultDatabaseIndex = g_ServerConfig.getConfigInt("DefaultDatabase", -1);
//    if (defaultDatabaseIndex >= 0)
//    {
//        g_mAbstractDb = g_ServerConfig.getDatabase(defaultDatabaseIndex);
//        if (!g_mAbstractDb)
//        {
//            CritLog(<< "Failed to get configuration database");
//            //cleanupObjects();
//            return NULL;
//        }
//    }
//    else     // Try legacy configuration parameter names
//    {
//#ifdef USE_MYSQL
//        Data mySQLServer;
//        g_ServerConfig.getConfigValue("MySQLServer", mySQLServer);
//        if (!mySQLServer.empty())
//        {
//            WarningLog(<< "Using deprecated parameter MySQLServer, please update to indexed Database definitions.");
//            mAbstractDb = new MySqlDb(g_ServerConfig, mySQLServer,
//                g_ServerConfig.getConfigData("MySQLUser", Data::Empty),
//                g_ServerConfig.getConfigData("MySQLPassword", Data::Empty),
//                g_ServerConfig.getConfigData("MySQLDatabaseName", Data::Empty),
//                g_ServerConfig.getConfigUnsignedLong("MySQLPort", 0),
//                g_ServerConfig.getConfigData("MySQLCustomUserAuthQuery", Data::Empty));
//        }
//#endif
//        if (!g_mAbstractDb)
//        {
//            //mAbstractDb = new BerkeleyDb(mProxyConfig->getConfigData("DatabasePath", "./", true));
//            g_mAbstractDb = g_ServerConfig.getDatabase(defaultDatabaseIndex);
//        }
//    }
    return g_mAbstractDb;
}