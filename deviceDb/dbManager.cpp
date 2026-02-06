#include "dbManager.h"

#include <string.h>


const static char *gpDeviceInfoSql = "CREATE TABLE IF NOT EXISTS `device_info` (" \
"  `id` INT NOT NULL AUTO_INCREMENT,"  \
"  `deviceId` VARCHAR(20) NOT NULL,"  \
"  `deviceIp` VARCHAR(45) NULL,"  \
"  `devicePort` INT NULL,"  \
"  `deviceUser` VARCHAR(45) NULL,"  \
"  `devicePswd` VARCHAR(45) NULL,"  \
"  `deviceName` VARCHAR(255) NULL,"  \
"  `createTime` DATETIME NULL DEFAULT CURRENT_TIMESTAMP,"  \
"  `updateTime` DATETIME NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"  \
"  PRIMARY KEY(`id`),"  \
"      UNIQUE INDEX `id_UNIQUE` (`id` ASC) VISIBLE,"  \
"      UNIQUE INDEX `deviceId_UNIQUE` (`deviceId` ASC) VISIBLE)"  \
"  ENGINE = InnoDB"  \
"  DEFAULT CHARACTER SET = utf8mb4"  \
"  COLLATE = utf8mb4_0900_ai_ci;";

const static char *gpDeviceChannelInfoSql = "CREATE TABLE IF NOT EXISTS `sub_device_info` (" \
"  `id` INT NOT NULL AUTO_INCREMENT,"  \
"  `deviceId` VARCHAR(20) NOT NULL,"  \
"  `channelNo` INT NULL,"  \
"  `channelName` VARCHAR(255) NULL,"  \
"  `createTime` DATETIME NULL DEFAULT CURRENT_TIMESTAMP,"  \
"  `updateTime` DATETIME NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"  \
"  PRIMARY KEY(`id`),"  \
"      UNIQUE INDEX `id_UNIQUE` (`id` ASC) VISIBLE,"  \
"      UNIQUE INDEX `deviceId_UNIQUE` (`deviceId` ASC) VISIBLE)"  \
"  ENGINE = InnoDB"  \
"  DEFAULT CHARACTER SET = utf8mb4"  \
"  COLLATE = utf8mb4_0900_ai_ci;";

CDbManager& CDbManager::Instance()
{
    static CDbManager g_CDbManager;
    return g_CDbManager;
}
CDbManager::CDbManager() : user("root")
, password("qd123456")
, mDbName("GB28181")
, mDbIp("192.168.1.223")
, port(3306)
, mIsConnect(false)
{
#ifdef USE_MYSQL
    mysql = mysql_init(NULL);
#endif
    // memset(mstrDbError, 0, gnErrBufSize + 1);

	// mbConnected = false;
	// mnConnectedValidTime = 0;
	// NLogDebug(MOD8, "new CMySql end: %d", mbConnected);
}

CDbManager::~CDbManager()
{
	// NLogDebug(MOD8, "~CMySql start: %d", mbConnected);
	// //if (mbConnected)
	// {
	// 	try
	// 	{
	// 		NLogDebug(MOD8, "~CMySql start: 1 %d", mbConnected);
	// 		DisConnectDb();			//	此处会出现问题？ free abort？
	// 		NLogDebug(MOD8, "~CMySql start: 2 %d", mbConnected);
	// 	}
	// 	catch (...)
	// 	{
	// 	}
	// }
}

bool CDbManager::initDb()
{
    if(ConnectDb())
    {
        return !CreateTables();
    }
    return false;
}
int CDbManager::CreateTables()
{
	if (!ExecDbSQL(gpDeviceInfoSql))
	{
		return -1;
    }
    
    if(!ExecDbSQL(gpDeviceChannelInfoSql))
	{
		return -2;
	}
	
    return 0;
}
int CDbManager::InitTableData()
{
    return 0;
}

bool CDbManager::ConnectDb(const char* pCharSet/* = "utf8mb4"*/)
{
    std::unique_lock<std::mutex> lock(dbMtx);
    if(!mysql)
		return false;
	
	int bReconnet = 0;
	// 连接数据库设置为5s超时, 设置10s超时时, 客户端会登录失败
	// 数据库查询时间从5s修改为30s, 怀疑error no=2013,msg = Lost connection to MySQL server during query是执行超时导致!
	unsigned int nCnOut = 5, nRW = 30;
	int nRes = mysql_options(mysql, MYSQL_OPT_RECONNECT, &bReconnet);//不设置重连 //重连+多线程并发 会导致mysql_real_query卡死或漰溃
	nRes = mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &nCnOut);
	nRes = mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, &nRW);
	nRes = mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT, &nRW);

    /* 初始化 SSL 设置 */
    const char *key_file = "/var/lib/mysql/client-key.pem";
    const char *cert_file = "/var/lib/mysql/client-cert.pem";
    const char *ca_file = "/var/lib/mysql/ca.pem";

    if (mysql_ssl_set(mysql, key_file, cert_file, ca_file, NULL, NULL) != 0)
    {         /* CRL path */
        // NLogDebug(MOD8, "Error setting SSL parameters");
        return false;
    }

	//nRes = mysql_options(mysql, MYSQL_SET_CHARSET_NAME, MYSQL_AUTODETECT_CHARSET_NAME);
	//nRes = mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");

	bool result = false;
	if(mysql_real_connect(mysql, mDbIp.c_str(), user.c_str(), password.c_str(), mDbName.c_str(), port,NULL,CLIENT_SSL))//localhost
	{ 
		// 这个日志会在 获取版本信息中也打印出来， 不再显示。
		//NLogDebug(MOD8, "CDbManager::ConnectDb 1 ip:%s port:%d name:%s pswd:%s successful!", mxDbConfig.mstrDbIP.c_str(), mxDbConfig.mnDbPort, mxDbConfig.mstrDbName.c_str(), mxDbConfig.mstrDbPassword.c_str());
		
		// mbConnected = true;
		// mnConnectedValidTime = time(0) + gnDbStateValidPeriodS;
		result = true;
	}
	else 
	{
		int nErr = mysql_errno(mysql);
		if (nErr == CR_ALREADY_CONNECTED)
		{
			// snprintf(mstrDbError, gnErrBufSize, "connect error no=%d,msg = %s, res:%d\n", nErr, mysql_error(mysql), nRes);
			// NLogDebug(MOD8, "CDbManager::ConnectDb 2 ip:%s port:%d name:%s pswd:%s successful!", mxDbConfig.mstrDbIP.c_str(), mxDbConfig.mnDbPort, mxDbConfig.mstrDbName.c_str(), mxDbConfig.mstrDbPassword.c_str(), mstrDbError);

            mIsConnect = true;
			result = true;
		}
		else
		{
			// error:CDbManager::ConnectDb 3 ip:127.0.0.1 port:3306 name:iv_db pswd:Ivillege error: connect error no=2003,msg = Can't connect to MySQL server on '127.0.0.1' (111)
			// snprintf(mstrDbError, gnErrBufSize, "connect error no=%d,msg = %s\n", nErr, mysql_error(mysql));	
			// NLogError(MOD8, "CDbManager::ConnectDb 3 ip:%s port:%d name:%s pswd:%s error: %s", mxDbConfig.mstrDbIP.c_str(), mxDbConfig.mnDbPort, mxDbConfig.mstrDbName.c_str(), mxDbConfig.mstrDbPassword.c_str(), mstrDbError);
			result = false;
		}
	}

	if (result && pCharSet != NULL && pCharSet[0] != 0)
	{
		// pCharSet = "utf8mb4"
		const char *charset_name = mysql_character_set_name(mysql);
		// NLogDebug(MOD8, "cur connect character set: %s", charset_name);
		if(strcmp(charset_name, pCharSet) != 0)
		{
            if (!mysql_set_character_set(mysql, pCharSet))
            {
                // NLogDebug(MOD8, "mysql New client character set: %s", mysql_character_set_name(mysql));
            }
		}
	}

	return result;
}
//todo:mysql如何关闭连接，而不释放mysql结构？
bool CDbManager::DisConnectDb()
{
	// NLogDebug(MOD8, "CDbManager::DisConnectDb msyql close");
	try
    {
        std::unique_lock<std::mutex> lock(dbMtx);
        if(mIsConnect && mysql)
		{
			mIsConnect = false;
            // NLogDebug(MOD8, "CDbManager::mysql_close msyql close %p", mysql);
            mysql_close(mysql);
            mysql = NULL;
		}
	}
	catch (...)
	{
		// NLogDebug(MOD8, "CDbManager::DisConnectDb msyql exceptions");
	}

	// NLogDebug(MOD8, "CDbManager::DisConnectDb end");
	return true;
}

// 这个接口认为可以不加锁调用!!
bool CDbManager::IsConnected()
{
	return mIsConnect;
}

bool CDbManager::CheckConnected()
{
    std::unique_lock<std::mutex> lock(dbMtx);
    if(!mysql)
		return false;
    return mIsConnect;
}

//如果数据没有打开，则打开，但是不关闭
bool CDbManager::ExecInsert(const char* sql, uint32_t& nDbID)
{
    std::unique_lock<std::mutex> lock(dbMtx);
    if(!mIsConnect || !mysql)
    {
        return false;
    }
	nDbID = 0;
	bool bOk = ExecDbSQL(sql);
	if( bOk )
	{
		try
		{
			nDbID = (uint32_t)mysql_insert_id(mysql); // mysql_insert_id()返回的是自增主键的id
			return true;
		}
		catch(...){}
	}
	else
	{
		// snprintf(mstrDbError, gnErrBufSize, "ExecSQL error no=%d,msg = %s,sql=%s", mysql_errno(mysql), mysql_error(mysql), sql);
		// NLogError(MOD8, "CDbManager::ExecInsert error:%s", mstrDbError);
		return false;
	}
	
	return false;
}

bool CDbManager::ExecInsert(const char* sql, uint64_t& nDbID)
{
    std::unique_lock<std::mutex> lock(dbMtx);
    if(!mIsConnect || !mysql)
    {
        return false;
    }
	nDbID = 0;
	bool bOk = ExecDbSQL(sql);
	if (bOk)
	{
		try
		{
			nDbID = (uint64_t)mysql_insert_id(mysql); // mysql_insert_id()返回的是自增主键的id
			return true;
		}
		catch (...) {}
	}
	else
	{
		// snprintf(mstrDbError, gnErrBufSize, "ExecSQL error no=%d,msg = %s,sql=%s", mysql_errno(mysql), mysql_error(mysql), sql);
		// NLogError(MOD8, "CDbManager::ExecInsert error:%s", mstrDbError);
		return false;
	}

	return false;
}

void CDbManager::ClearQureyResult()
{
    if(!mysql)
		return;
	
	MYSQL_ROW row;
	MYSQL_RES * pResult = mysql_use_result(mysql);
	if (pResult != NULL)
	{
		while ((row = mysql_fetch_row(pResult)) != NULL)
		{
		}		
	}

	mysql_free_result(pResult);
}

bool CDbManager::ExecSelect(const char* sql, std::list< std::vector<std::string> >& xResult)
{
    std::unique_lock<std::mutex> lock(dbMtx);
    if(!mysql)
		return false;
	
	if (ExecDbSQL(sql))
	{
		MYSQL_ROW row;
		MYSQL_RES * pResult = mysql_use_result(mysql);
		if (pResult != NULL)
		{
			int nCols = mysql_num_fields(pResult);
			std::vector<std::string> vecValues;
			vecValues.resize(nCols);
			while ((row = mysql_fetch_row(pResult)) != NULL)
			{
				for (int i = 0; i < nCols; i++)
				{
					if (row[i] != NULL)
					{
						vecValues[i] = row[i];
					}
					else
					{
						vecValues[i] = "";
					}
				}

				xResult.push_back(vecValues);
			}
		}

		mysql_free_result(pResult); // NULL时也可以调用 mysql_free_result

		// if ((int)(t1-t0) > 100)
		// {
		// 	CMilliTime t2;
		// 	NLogDebug(MOD8, "CDbManager::ExecSelect cost %d %d ms, sql: %s", (int)(t1 - t0), (int)(t2 - t1), sql);
		// }
		return true;
	}
	// NLogError(MOD8, "CDbManager::ExecSelect ExecDbSQL error: %s", mstrDbError);
	return false;
}

bool CDbManager::ExecIntValueSelect(const char* sql, uint32_t& nData)
{
    std::unique_lock<std::mutex> lock(dbMtx);
    if(!mysql)
		return false;
	
	nData = 0;
	if (ExecDbSQL(sql))
	{
		MYSQL_ROW row;
		MYSQL_RES * pResult = mysql_use_result(mysql);
		if (pResult != NULL)
		{
			int nCols = mysql_num_fields(pResult);
			while ((row = mysql_fetch_row(pResult)) != NULL)
			{
				if (nCols > 0 && row[0] != NULL)
				{
					nData = (uint32_t)std::stoi(row[0]);
				}
			}
		}

		mysql_free_result(pResult);
		return true;
	}
	// NLogError(MOD8, "ExecDbSQL error %s", sql);
	return false;
}

bool CDbManager::ExecUint64ValueSelect(const char* sql, uint64_t& nData)
{
    std::unique_lock<std::mutex> lock(dbMtx);
    if(!mysql)
		return false;
	
	nData = 0;
	if (ExecDbSQL(sql))
	{
		MYSQL_ROW row;
		MYSQL_RES * pResult = mysql_use_result(mysql);
		if (pResult != NULL)
		{
			int nCols = mysql_num_fields(pResult);
			while ((row = mysql_fetch_row(pResult)) != NULL)
			{
				if (nCols > 0 && row[0] != NULL)
				{
					nData = std::stoll(row[0]);
				}
			}
		}

		mysql_free_result(pResult);
		return true;
	}
	// NLogError(MOD8, "ExecDbSQL error %s", sql);
	return false;
}

bool CDbManager::ExecStringValueSelect(const char* sql, std::string& strData)
{
    std::unique_lock<std::mutex> lock(dbMtx);
    if(!mysql)
		return false;
	if(ExecDbSQL(sql))
	{
		MYSQL_ROW row;
		MYSQL_RES * pResult = mysql_use_result(mysql);
		if (pResult != NULL)
		{
			int nCols = mysql_num_fields(pResult);
			while ((row = mysql_fetch_row(pResult)) != NULL)
			{
				if (nCols > 0 && row[0] != NULL)
				{
					strData = row[0];
				}
			}
		}

		mysql_free_result(pResult);
		return true;
	}
	// NLogError(MOD8, "CDbManager:: ExecDbSQL error");
	return false;
}

bool CDbManager::ExecOneStringColSelect(const char* sql, std::list< std::string >& xListString)
{
    std::unique_lock<std::mutex> lock(dbMtx);
    if(!mysql)
		return false;
	if(ExecDbSQL(sql))
	{
		MYSQL_ROW row;
		MYSQL_RES * pResult = mysql_use_result(mysql);
		if (pResult != NULL)
		{
			int nCols = mysql_num_fields(pResult);
			while ((row = mysql_fetch_row(pResult)) != NULL)
			{
				if (nCols > 0 && row[0] != NULL)
				{
					xListString.push_back(row[0]);  // 只处理第一列
				}
			}
		}

		mysql_free_result(pResult);
		return true;
	}

	// NLogError(MOD8, "CDbManager::ExecOneStringColSelect ExecDbSQL error: %s", sql);
	return false;
}

bool CDbManager::ExecOneStringRowSelect(const char* sql, std::vector<std::string>& xResult)
{
    std::unique_lock<std::mutex> lock(dbMtx);
    if(!mysql)
		return false;
	if(ExecDbSQL(sql))
	{
		MYSQL_ROW row;
		MYSQL_RES * pResult = mysql_use_result(mysql);
		if (pResult != NULL)
		{
			int nCols = mysql_num_fields(pResult);
			while ((row = mysql_fetch_row(pResult)) != NULL)
			{
				for (int i = 0; i < nCols; i++)
				{
					if (row[i] != NULL)
					{
						xResult.push_back(row[i]);
					}
					else
					{
						xResult.push_back("");
					}
				}
			}
		}		
		
		mysql_free_result(pResult);
		return true;
	}

	// NLogError(MOD8, "CDbManager::ExecSelect ExecDbSQL error: %s", mstrDbError);
	return false;
}


// 注意: 查询后要 获取数据, 或者执行ClearQureyResult(); 清空查询, 否则会出错: Commands out of sync; you can't run this command now
bool CDbManager::ExecDbSQL(const char* sql)
{
    if(!mysql)
		return false;
	try
	{
	    if(!mIsConnect)
        {
            return false;
        }
		if (mysql_real_query(mysql, sql, (unsigned int)strlen(sql)) == 0)
		{
			// mnConnectedValidTime = time(0) + gnDbStateValidPeriodS;
			return true;
		}

        int no = mysql_errno(mysql);
        std::string strErr = mysql_error(mysql);
        //http://blog.csdn.net/dengxingbo/archive/2010/08/30/5849576.aspx
		//	2006: error no=2006,msg = MySQL server has gone away
		if ((no >= 1042 && no <= 1045) || no == 1081 || no == 1129 ||
			no == 1130 || (no >= 1158 && no <= 1161) || no == 1177 || (no >= 2000 && no <= 2014))
		{
			if (no == CR_SERVER_LOST || no == CR_SERVER_GONE_ERROR)
			{
				// NLogError(MOD8, "mysqld down sql:%s ;error msg: %d %s", sql, no, mstrDbError);
				//system("killall -9 mysqld");
			}
			else if (no == CR_CONN_HOST_ERROR)
			{
				// NLogError(MOD8, " error database connect Exception 1 errorno: %d, %s", no, mstrDbError);
			}
        }
        else if(no == 1213)
        {
            if(strErr.find("Deadlock found when trying to get lock") != std::string::npos)
            {
                // mysql 死锁 重试以解决死锁问题
                return ExecDbSQL(sql);
            }
        }

		// NLogError(MOD8, " error errorno: %d,%s", no, mstrDbError);
	}
	catch (...)
	{
		// NLogError(MOD8, "CDbManager::ExecSQL  excepted! %s", sql);
	}

	return false;
}

MYSQL_RES * CDbManager::FetchExecuteResult()
{
	if(!mysql)
		return NULL;
	MYSQL_RES *pResult = NULL;
	if(!(pResult = mysql_use_result(mysql)))
	{ 
		// NLogError(MOD8, "database connect error mysql_use_result");
		return NULL;
	}
	return pResult;
}

int CDbManager::GetExecuteResultRows(MYSQL_RES * pResult)
{
	if(pResult == NULL) return 0;
	return (int)mysql_num_rows(pResult);
}

uint32_t CDbManager::GetExecuteResultCols(MYSQL_RES * pResult)
{
	return mysql_num_fields(pResult);
}

MYSQL_ROW CDbManager::FetchRow(MYSQL_RES * pResult)//返回数据的指针数组！//字符指针的指针
{
	if(pResult == NULL) return NULL;
	return mysql_fetch_row(pResult);
}

void CDbManager::FreeExecuteResult(MYSQL_RES * pResult)
{
	if(pResult != NULL) mysql_free_result(pResult);
}

bool CDbManager::BeginTransaction()
{
	return ExecDbSQL("START TRANSACTION;");
}

bool CDbManager::CommitTransaction()
{
	return ExecDbSQL("COMMIT;");
}

bool CDbManager::Rollback()
{
	if(!ExecDbSQL("ROLLBACK;")) return false;
	return true;
}

/*
1、不管autocommit 是1还是0
START TRANSACTION 。后只有当commit数据才会生效，ROLLBACK后就会回滚。

2、当autocommit 为 0 时
不管有没有START TRANSACTION。
只有当commit数据才会生效，ROLLBACK后就会回滚。
*/
//select @@autocommit=0;
bool CDbManager::SetAutoCommit(bool bTrue)
{
	if(bTrue) return  ExecDbSQL("SET AUTOCOMMIT = 1;");
	else return ExecDbSQL("SET AUTOCOMMIT = 0;");
}

/* 添加索引, 可能会卡住, 如果卡住, 需要手动执行:
#!/bin/sh
MYSQLADMIN="/usr/local/mysql/bin/mysqladmin"
for i in `$MYSQLADMIN processlist -uroot -pIvillege|grep -i sleep |grep  'iv' |awk '{print $2}'`
do
$MYSQLADMIN -uroot -pIvillege kill $i
done
*/
bool CDbManager::IsIndexExsitAndAlterIndex(const char* tablename, const char *pIndexName, const char *psql)
{
    char sqlstring[256] = { 0 };
	// SELECT * FROM information_schema.statistics WHERE table_name = 'node_info' AND index_name = 'db_node_info_type_fid'
	snprintf(sqlstring, 256 - 1, "SELECT * FROM information_schema.statistics WHERE table_name = '%s' AND index_name = '%s';", tablename, pIndexName);
	std::list< std::vector<std::string> >  xResult;
	if (ExecSelect(sqlstring, xResult))
	{
		if (xResult.size() > 0)
		{
			return true;
		}
		else
		{
            std::unique_lock<std::mutex> lock(dbMtx);
			bool bRet = ExecDbSQL(psql);
			ClearQureyResult();
			return bRet;
		}
	}
	// NLogError(MOD8, "IsIndexExsitAndAlterIndex ExecSelect error:%s", mstrDbError);
	return false;
}

bool CDbManager::IsColumExsitAndAddColum(const char* tablename, const char *columname, const char *psql)
{
    char sqlstring[256] = { 0 };
	snprintf(sqlstring, 256 - 1, "Describe %s %s; ", tablename, columname);
	std::list< std::vector<std::string> >  xResult;
	if (ExecSelect(sqlstring, xResult))
	{
		if (xResult.size() > 0)
		{
			return true;
		}
		else
		{
            std::unique_lock<std::mutex> lock(dbMtx);
			/*snprintf(sqlstring, 256 - 1, "alter table %s add %s %s %s " \
				"add index(%s);", tablename, columname, columname, datatype, restraint);*/
			bool bRet = ExecDbSQL(psql);
			ClearQureyResult();
			return bRet;
		}
	}
	// NLogError(MOD8, "CDbManager::IsColumExsitAndAddColum ExecSelect error:%s", mstrDbError);
	return false;
}

bool CDbManager::AlterColumMaxLength(const char* tablename, const char *columname, uint32_t nNewSize)
{
    char sqlstring[256] = { 0 };
	snprintf(sqlstring, 256 - 1, "select CHARACTER_MAXIMUM_LENGTH from information_schema.columns where table_name='%s' and table_schema='iv_db' and COLUMN_NAME='%s'", tablename, columname);
	uint32_t nIntValue = 0;
	if (ExecIntValueSelect(sqlstring, nIntValue))
	{
		if (nIntValue != 0 && nIntValue < nNewSize)
		{
            std::unique_lock<std::mutex> lock(dbMtx);
			snprintf(sqlstring, 256 - 1, "alter table %s modify %s varchar(%d)", tablename, columname, nNewSize);
			if (ExecDbSQL(sqlstring))
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	// NLogError(MOD8, "CDbManager::ExecIntValueSelect error:%s", mstrDbError);
	return false;
}

// 把id从int(10) 改为bigint(20), id不能被其它表作为外键使用!
// select NUMERIC_PRECISION from information_schema.columns where TABLE_SCHEMA='iv_db' and TABLE_NAME='cms_log_info' and COLUMN_NAME='id'
// ALTER TABLE `cms_log_info` CHANGE `id` `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
bool CDbManager::AlterColumnAutoIncrementTypeToBigInt(const char* pDbName, const char* pTableName, const char *pColumnName)
{
    char sqlstring[256] = { 0 };
	snprintf(sqlstring, 256 - 1, "select NUMERIC_PRECISION from information_schema.columns where TABLE_SCHEMA='%s' and TABLE_NAME='%s' and COLUMN_NAME='%s'", pDbName, pTableName, pColumnName);
	uint32_t nIntValue = 0;
	if (ExecIntValueSelect(sqlstring, nIntValue))
	{
		if (nIntValue != 0 && nIntValue <= 11)
		{
			// NLog(MOD8, LV1, "start CDbManager::ExecDbSQL alter table %s", pTableName);
			// CMilliTime t0;
            // 这里执行的时间很长 41万条, 耗时 102秒
            std::unique_lock<std::mutex> lock(dbMtx);
            snprintf(sqlstring, 256 - 1, "ALTER TABLE `%s` CHANGE `%s` `%s` bigint(20) unsigned NOT NULL AUTO_INCREMENT;", pTableName, pColumnName, pColumnName);
			if (ExecDbSQL(sqlstring))
			{
				// CMilliTime t1;
				// NLog(MOD8, LV1, "end CDbManager::ExecDbSQL %s cost:%d ms", sqlstring, (uint32)(t1-t0));
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	// NLogError(MOD8, "CDbManager::ExecIntValueSelect error:%s", mstrDbError);
	return false;
}
