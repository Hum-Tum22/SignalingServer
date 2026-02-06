#pragma once
#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif
#include <mutex>
#include <vector>
#include <list>
class CDbManager
{
public:
    CDbManager();
    ~CDbManager();
public:
    static CDbManager& Instance();
    bool initDb();
    int CreateTables();
	int InitTableData();
    bool ConnectDb(const char* pCharSet = "utf8mb4");
	bool DisConnectDb();//调用int sqlite3_close(sqlite3 *);//CloseSqlite();
	bool IsConnected();
	bool CheckConnected();
	//打开数据库，执行后关闭数据库
	bool ExecInsert(const char* sql, uint32_t& nDbID);	// nDbID 需要所操作表有自增主机id时才会返回, 否则为0
	bool ExecInsert(const char* sql, uint64_t& nDbID);
	bool ExecSelect(const char* sql, std::list< std::vector<std::string> >& xResult);
	bool ExecIntValueSelect(const char* sql, uint32_t& nData);
	bool ExecUint64ValueSelect(const char* sql, uint64_t& nData);
	bool ExecStringValueSelect(const char* sql, std::string& strData);
	bool ExecOneStringColSelect(const char* sql, std::list< std::string >& xListString);
	bool ExecOneStringRowSelect(const char* sql, std::vector<std::string>& xResult);

	bool IsIndexExsitAndAlterIndex(const char* tablename, const char *pIndexName, const char *psql);
	bool IsColumExsitAndAddColum(const char* tablename, const char *columname, const char *psql);
	bool AlterColumMaxLength(const char* tablename, const char *columname, uint32_t nNewSize);
	// int(10) 修改为bigint(20)
	bool AlterColumnAutoIncrementTypeToBigInt(const char* pDbName, const char* pTableName, const char *pColumnName);



	//事务处理
	bool SetAutoCommit(bool bAutoCommit);
	bool BeginTransaction();
	bool CommitTransaction();
	bool Rollback();
	
protected:
public:

private:
#ifdef USE_MYSQL
	bool ExecDbSQL(const char* sql);
	void ClearQureyResult();
    MYSQL_RES* FetchExecuteResult();
	int GetExecuteResultRows(MYSQL_RES * pResult);
	uint32_t GetExecuteResultCols(MYSQL_RES * pResult);
	MYSQL_ROW FetchRow(MYSQL_RES * pResult);//返回数据的指针数组！
	void FreeExecuteResult(MYSQL_RES * pResult);
    MYSQL *mysql;
#endif
    std::mutex dbMtx;
    std::string user;
    std::string password;
    std::string mDbName;
    std::string mDbIp;
    int port;
    bool mIsConnect;
};