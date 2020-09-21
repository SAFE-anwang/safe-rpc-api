#pragma once
#include <sqlite3.h>
#include <string>

typedef int (*SQLITE3_CALLBACK)(void*, int, char**, char**);

class mySQLiteDB
{
public:
	mySQLiteDB() {}
	~mySQLiteDB() { close(); }
	mySQLiteDB(std::string dbfilename, std::string tabName = "", std::string tabSQL = "") { open(dbfilename,tabName,tabSQL); }

	bool open(std::string dbfilename, std::string tabName = "", std::string tabSQL = "");
	void close();
	bool exec(std::string sql, SQLITE3_CALLBACK callback = nullptr, void* pData = nullptr);


private:
	bool tableExist(const std::string& strTableName);
	static int tableExist_callback(void* pHandle, int iRet, char** szSrc, char** szDst);
	std::string tableName;
	std::string tableSQL;
	sqlite3* mydb{ nullptr };
};