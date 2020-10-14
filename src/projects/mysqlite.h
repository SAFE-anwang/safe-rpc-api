#pragma once
#include <sqlite3.h>
#include <string>

typedef int (*SQLITE3_CALLBACK)(void*, int, char**, char**);

class mySQLiteDB
{
public:
	mySQLiteDB() {}
	~mySQLiteDB() { close(); }
	mySQLiteDB(std::string dbfilename) { open(dbfilename); }

	bool open(std::string dbfilename);
	void close();
	bool exec(std::string sql, SQLITE3_CALLBACK callback = nullptr, void* pData = nullptr);




	std::string tab = "safe2eth";
	std::string sql = "CREATE TABLE " + tab + "( \
		txid			VARCHAR(64) PRIMARY KEY UNIQUE, \
		n				INTEGER NOT NULL, \
		amount			DOUBLE  NOT NULL, \
		confirmations	INTEGER NOT NULL, \
		blockindex		INTEGER NOT NULL, \
		blockhash       VARCHAR(64), \
		safe_address    VARCHAR(40), \
		eth_address     VARCHAR(50), \
		eth_txid		VARCHAR(64), \
		eth_fee			DOUBLE, \
		eth_blockhash   VARCHAR(64), \
		eth_blockindex	INTEGER);";


	std::string tab2 = "eth2safe";
	std::string sql2 = "CREATE TABLE " + tab2 + "( \
		eth_txid		VARCHAR(64) PRIMARY KEY UNIQUE, \
		eth_address     VARCHAR(50) NOT NULL, \
		amount			DOUBLE  NOT NULL, \
	    safe_address    VARCHAR(40) NOT NULL, \
		txid			VARCHAR(64), \
		n				INTEGER);";

private:
	bool tableExist(const std::string& strTableName);
	static int tableExist_callback(void* pHandle, int iRet, char** szSrc, char** szDst);
	std::string tableName;
	std::string tableSQL;
	sqlite3* mydb{ nullptr };
};