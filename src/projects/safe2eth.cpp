#include <bitcoinapi/safeapi.h>
#include <bitcoinapi/exception.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <sstream>    
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include "mysqlite.h"
#include<chrono>
#include<thread>

#include <jsonrpccpp/client.h>
#include <jsonrpccpp/client/connectors/httpclient.h>

using jsonrpc::Client;
using jsonrpc::JSONRPC_CLIENT_V2;

using jsonrpc::HttpClient;
using jsonrpc::JsonRpcException;

using Json::Value;
using Json::ValueIterator;

#define THROW_RETURN(METHOD,RET)  try {(METHOD);} catch (BitcoinException& e) {           \
							 std::cout << "Error (" << e.getCode() << "): "<< e.getMessage() << std::endl; return RET;}
#define THROW_NO_RETURN(METHOD)  try {(METHOD);} catch (BitcoinException& e) {           \
							 std::cout << "Error (" << e.getCode() << "): "<< e.getMessage() << std::endl;}
typedef struct
{
	std::string txid;
	int n;
	double amount;
	int confirmations;
	int blockindex;
	std::string blockhash;
	std::string safe_address;
	std::string eth_address;
	std::string eth_txid;
	double eth_fee;
	std::string eth_blockhash;
	int eth_blockindex;
	double safe_fee;
}safe2eth;

typedef struct
{
	std::string eth_txid;
	std::string eth_address;
	double amount;
	std::string safe_address;
	std::string txid;
	int n;
	double fee;
}eth2safe;

typedef std::map<std::string, safe2eth*> Safe2EthMap;
typedef std::map<std::string, safe2eth*>::const_iterator Safe2EthMapIterator;

typedef std::map<std::string, eth2safe*> Eth2SafeMap;
typedef std::map<std::string, eth2safe*>::const_iterator Eth2SafeMapIterator;

typedef std::vector<std::string> StringVec;


int g_nBeginIndex = 2435830;//从该区块号开始处理
std::string g_myAddress = "XhjK5ySArqRtKmSThLZmAWe7MrjfprvnRH";//接收的兑换地址
int g_need_confirms = 1; //确认数，达到确认数才开始处理
double g_min_value = 0.1;//发送的最小SAFE金额
double g_txfee = 0.1;    //要扣除的SAFE@ETH费用
int g_scan_interval = 30; // 多长时间开始扫描最新交易：30 seconds
int g_noderpc_timeout = 300;//与nodesjs的联系多长时间超时，默认5分钟
std::string g_noderpc_url = "http://127.0.0.1:50505";//node js的RPC网址和端口
std::string g_sqlitedb_file = "safe2eth.db";//数据库名称
bool g_bExit = false;


/*
// C prototype : void StrToHex(byte *pbDest, char *pszSrc, int nLen)
// parameter(s): [OUT] pbDest - 输出缓冲区
//	[IN] pszSrc - 字符串
//	[IN] nLen - 16进制数的字节数(字符串的长度/2)
// return value:
// remarks : 将字符串转化为16进制数
*/
void StrToHex(char* pbDest, const char* pszSrc, int nLen)
{
	char h1, h2;
	char s1, s2;
	for (int i = 0; i < nLen; i++)
	{
		h1 = pszSrc[2 * i];
		h2 = pszSrc[2 * i + 1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;

		pbDest[i] = s1 * 16 + s2;
	}
}

/*
// C prototype : void HexToStr(char *pszDest, byte *pbSrc, int nLen)
// parameter(s): [OUT] pszDest - 存放目标字符串
//	[IN] pbSrc - 输入16进制数的起始地址
//	[IN] nLen - 16进制数的字节数
// return value:
// remarks : 将16进制数转化为字符串
*/
void HexToStr(char* pszDest, const char* pbSrc, int nLen)
{
	char	ddl, ddh;
	for (int i = 0; i < nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pszDest[i * 2] = ddh;
		pszDest[i * 2 + 1] = ddl;
	}
}


class safenode : public SafeAPI
{
public:

	safenode(std::string username = "safe", std::string password = "safe", std::string address = "127.0.0.1", int port = 5554) : \
		SafeAPI(username, password, address, port) { }
	~safenode() { }

	void outputInfo(getinfo_t info)
	{
		std::cout << "=== getinfo ===" << std::endl;
		std::cout << "Version: " << info.version << std::endl;
		std::cout << "Protocol Version: " << info.protocolversion << std::endl;
		std::cout << "Wallet Version: " << info.walletversion << std::endl;
		std::cout << "Balance: " << info.balance << std::endl;
		std::cout << "Blocks: " << info.blocks << std::endl;
		std::cout << "Timeoffset: " << info.timeoffset << std::endl;
		std::cout << "Connections: " << info.connections << std::endl;
		std::cout << "Proxy: " << info.proxy << std::endl;
		std::cout << "Difficulty: " << info.difficulty << std::endl;
		std::cout << "Testnet: " << info.testnet << std::endl;
		std::cout << "Keypoololdest: " << info.keypoololdest << std::endl;
		std::cout << "Keypool size: " << info.keypoolsize << std::endl;
		std::cout << "Paytxfee: " << info.paytxfee << std::endl;
		std::cout << "Unlocked_until: " << info.unlocked_until << std::endl;
		std::cout << "Errors: " << info.errors << std::endl << std::endl;
	}

	bool getBlockHashes(const int from_blocknumber, const int to_blocknumber, StringVec& ret)
	{
		std::string hash;
		int Count = 0;
		double percent = 0.0;

		std::cout.precision(2);

		for (int i = from_blocknumber; i <= to_blocknumber; i++)
		{
			THROW_RETURN(hash = getblockhash(i), false);
			ret.push_back(hash);
			percent = 100 * Count / (to_blocknumber - from_blocknumber + 1);
			std::cout << "\rgetblockhashes progress: " << percent << "%" << std::flush;
			Count++;
		}
		std::cout << "\rgetblockhashes progress: " << 100 << "%" << std::flush;
		std::cout << std::endl;
		return true;
	}

	bool getEthAddr(const std::string& addString, std::string& eth)
	{
		if (addString.size() < 188) return false;
		int skipLen = 48;

		char* buff[3000];
		memset(buff, 0, 3000);

		StrToHex((char*)buff, addString.c_str(), addString.size() / 2);
		char* eth_addr = (char*)buff + skipLen;

		//需要有ETH：这四个关键字，后面跟着ETH地址；
		if (memcmp(eth_addr, "eth:", 4) != 0)
		{
			std::cout << "getEthAddr: eth error: " << eth_addr << std::endl;
			return false;
		}
		eth_addr = (char*)(eth_addr + 4);

		//去除前面和后面可能的空格
		while (*eth_addr == 20) { *eth_addr = 0; eth_addr++; }

		if (memcmp(eth_addr, "0x", 2) != 0 || strlen(eth_addr) != 42)
		{
			std::cout << "getEthAddr 0x: " << eth_addr << std::endl;
			return false;
		}

		eth = eth_addr;
		eth = trim(eth);
		return true;
	}

	double getBalanceByAddr(const std::string& addr)
	{
		double amount = - 1.0;
		THROW_RETURN(amount = this->getbalance(), false);
		return amount;
	}

	bool getSafe2EthList(const StringVec& hashs, Safe2EthMap& safemap, const std::string& myAddr, const int min_confirms, const int min_value)
	{
		std::cout.precision(2);

		int Count = 0;
		double percent = 100 * Count / hashs.size();
		std::cout << "\rgetSafe2EthList progress: " << percent << "%" << std::flush;

		for (StringVec::const_iterator it_hash = hashs.begin(); it_hash != hashs.end(); it_hash++)
		{
			std::string hash = *it_hash;
			blockinfo_t bc;
			THROW_RETURN(bc = getblock(hash), 0);

			//decode transactions
			for (StringVec::const_iterator it_tx = bc.tx.begin(); it_tx != bc.tx.end(); it_tx++)//block hashs
			{
				safe_getrawtransaction_t raw_tx;
				THROW_RETURN(raw_tx = safe_getrawtransaction(*it_tx, true), 0);

				//if it is coinbase, then continue. we dont need coinbase tx.
				if (raw_tx.vin.size() == 1 && raw_tx.vin[0].txid.empty()) continue;

				//确认数量未到要求的min_confirms，则跳过。
				if (raw_tx.confirmations < (unsigned int)min_confirms) continue;

				//vout 
				for (std::vector<safe_vout_t> ::const_iterator it_vout = raw_tx.vout.begin(); it_vout != raw_tx.vout.end(); it_vout++)
				{

					//如果发送的不是SAFE，或者有锁定时间，或者金额不到最小金额，这些交易都跳过。
					if (it_vout->txType != 1 || it_vout->nUnlockedHeight != 0 || it_vout->value < min_value) continue;

					StringVec* pVec = (StringVec*)&((*it_vout).scriptPubKey.addresses);
					for (StringVec::const_iterator it_addr = pVec->begin(); it_addr != pVec->end(); it_addr++)//
					{
						std::string addr_recv = *it_addr;
						std::string eth;

						//如果对接地址对应，而且外带数据超过188个字节，有可能是我们要找的交易。
						if (myAddr.compare(addr_recv) == 0 && getEthAddr(it_vout->reserve, eth))
						{
							//获得该交易的上个交易，找到发送地址
							safe_getrawtransaction_t vin_tx;
							THROW_NO_RETURN(vin_tx = safe_getrawtransaction(raw_tx.vin[0].txid, true));

							safe2eth* record = new safe2eth;
							record->eth_address = eth;
							record->txid = raw_tx.txid;
							record->blockhash = raw_tx.blockhash;
							record->safe_address = vin_tx.vout[raw_tx.vin[0].n].scriptPubKey.addresses[0];
							record->amount = it_vout->value;
							record->n = it_vout->n;
							record->confirmations = raw_tx.confirmations;
							record->blockindex = bc.height;
							safemap[record->txid] = record;
						}
					}
				}
			}
			Count++;
			percent = 100 * Count / hashs.size();
			std::cout << "\rgetSafe2EthList progress: " << percent << "%" << std::flush;
		}
		std::cout << "\rgetSafe2EthList progress: 100%   " << std::endl;
		return true;
	}

	int getBlockCount()
	{
		int nBlockCount = 0;
		do
		{
			THROW_RETURN(nBlockCount = getblockcount(), 0);
			printf("current blockcount: %d\n", nBlockCount);

			if (g_nBeginIndex >= nBlockCount)
			{
				printf("blockchain count: %d, beginIndex: %d, blockchain sync not completed, waiting for 5 minutes...\n", nBlockCount, g_nBeginIndex);
				std::this_thread::sleep_for(std::chrono::milliseconds(g_scan_interval * 1000));
			}

		} while (g_nBeginIndex >= nBlockCount);

		return nBlockCount;
	}
};

//更新safe2eth表
bool update_safe2eth(mySQLiteDB& db, std::string tab, safe2eth& eth)
{
	char sqlBuff[4096] = { 0 };
	if (eth.eth_txid.empty())//如果eth_txid为空，则是更新确认数
		sprintf(sqlBuff, "UPDATE %s SET confirmations = %d WHERE txid == '%s' ;", tab.c_str(), eth.confirmations, eth.txid.c_str());
	else//如果eth_txid不为空，则已经发出SAFE@ETH，更新eth_txid和eth_fee，eth_blockhash，eth_blockindex
		sprintf(sqlBuff, "UPDATE %s SET eth_txid = '%s', eth_fee = %f, eth_blockhash = '%s', eth_blockindex = %d,safe_fee =%f WHERE txid == '%s' ;", tab.c_str(), eth.eth_txid.c_str(), eth.eth_fee, eth.eth_blockhash.c_str(), eth.eth_blockindex,eth.safe_fee, eth.txid.c_str());
	std::string sql = sqlBuff;
	bool bRet = db.exec(sql, nullptr, nullptr);
	if (!bRet)
	{
		printf("cann't update data to db: %s\n", eth.txid.c_str());
	}
	return bRet;
}
//更新eth2safe表
bool update_eth2safe(mySQLiteDB& db, std::string tab, eth2safe& eth)
{
	//更新SAFE交易ID和索引
	char sqlBuff[4096] = { 0 };
	if (!eth.txid.empty())
		sprintf(sqlBuff, "UPDATE %s SET txid = '%s', n = '%d' ,fee = '%f' WHERE eth_txid == '%s' ;", tab.c_str(), eth.txid.c_str(), eth.n, eth.fee, eth.eth_txid.c_str());
	else
	{
		//如果SAFE交易ID为空，则返回错误
		//printf("update_eth2safe error: eth.txid is empty while eth_txid = %s\n", eth.eth_txid.c_str());
		return false;
	}

	std::string sql = sqlBuff;
	bool bRet = db.exec(sql, nullptr, nullptr);
	if (!bRet)
	{
		printf("update_eth2safe: cann't update data to db: %s\n", eth.txid.c_str());
	}
	return bRet;
}

int insert_callback(void* data, int argc, char** argv, char** azColName)
{
	if (argc == 1)
	{
		int* nRow = (int*)data;
		*nRow = atoi(argv[0]);
	}
	return 0;
}
//插入eth2safe表
bool insert_eth2safe(mySQLiteDB& db, std::string tab, eth2safe & eth)
{
	//查看相同的eth_txid是否已经存在于表中
	std::string sql = "SELECT count(eth_txid) AS count FROM " + tab + "  WHERE eth_txid == '" + eth.eth_txid + "';";
	int nRow = 0;
	bool bRet = db.exec(sql, &insert_callback, &nRow);
	if (!bRet)
	{
		std::cout << "insert_eth2safe: cann't select eth_txid count from db: " << eth.eth_txid.c_str() << std::endl;
	}

	if (nRow != 0) //已经有该交易ID，则只能更新SAFE交易ID和索引
		return update_eth2safe(db, tab, eth);

	//没有该交易ID则插入整行数据，其中txid为空，索引为0
	char sqlBuff[4096] = { 0 };
	sprintf(sqlBuff, "INSERT INTO %s VALUES('%s','%s',%f,'%s','',0,0);", tab.c_str(), eth.eth_txid.c_str(), eth.eth_address.c_str(), eth.amount,eth.safe_address.c_str());
	sql = sqlBuff;
	bRet = db.exec(sql, nullptr, nullptr);
	if (!bRet)
	{
		printf("cann't insert data to db: %s\n", eth.txid.c_str());
	}
	return bRet;
}
//插入safe2eth表
bool insert_safe2eth(mySQLiteDB& db, std::string tab, safe2eth& eth)
{
	std::string sql = "SELECT count(txid) AS count FROM " + tab + "  WHERE txid == '" + eth.txid + "';";
	int nRow = 0;
	bool bRet = db.exec(sql, &insert_callback, &nRow);
	if (!bRet)
	{
		std::cout << "cann't select count from db: " << eth.txid.c_str() << std::endl;
	}

	if (nRow != 0) //已经有该交易ID，则更新确认数
		return update_safe2eth(db, tab, eth);

	//没有该交易ID则插入整行数据
	char sqlBuff[4096] = { 0 };
	sprintf(sqlBuff, "INSERT INTO %s VALUES('%s',%d,%f,%d,%u,'%s','%s','%s','%s',0,'',0,0);", tab.c_str(), eth.txid.c_str(), eth.n, eth.amount, eth.confirmations, \
		eth.blockindex, eth.blockhash.c_str(), eth.safe_address.c_str(), eth.eth_address.c_str(), eth.eth_txid.c_str());
	sql = sqlBuff;
	bRet = db.exec(sql, nullptr, nullptr);
	if (!bRet)
	{
		printf("cann't insert data to db: %s\n", eth.txid.c_str());
	}
	return bRet;
}

int select_callback_safe2eth(void* data, int argc, char** argv, char** azColName)
{
	Safe2EthMap* psafe2ethMap = (Safe2EthMap*)data;
	if (psafe2ethMap == nullptr) return 0;
	if (argc != 13)
	{
		std::cout << "select_callback_eth2safe::argc is %d " << argc << ", should be 13 ." << std::endl;
		return 0;
	}

	safe2eth* record = new safe2eth;
	record->txid = argv[0];
	record->n = atoi(argv[1]);
	record->amount = atof(argv[2]);
	record->confirmations = atoi(argv[3]);
	record->blockindex = atoi(argv[4]);
	record->blockhash = argv[5];
	record->safe_address = argv[6];
	record->eth_address = argv[7];
	record->eth_txid = argv[8];
	record->eth_fee = atof(argv[9]);
	record->eth_blockhash = argv[10];
	record->eth_blockindex = atoi(argv[11]);
	record->safe_fee = atof(argv[12]);

	(*psafe2ethMap)[record->txid] = record;
	return 0;
}
int select_callback_eth2safe(void* data, int argc, char** argv, char** azColName)
{
	Eth2SafeMap* pMap = (Eth2SafeMap*)data;
	if (pMap == nullptr) return 0;
	if (argc != 7)
	{
		std::cout << "select_callback_eth2safe::argc is %d " << argc << ", should be 7." << std::endl;
		return 0;
	}

	eth2safe* record = new eth2safe;
	record->eth_txid = argv[0];
	record->eth_address = argv[1];
	record->amount = atof(argv[2]);
	record->safe_address = argv[3];
	record->txid = argv[4];
	record->n = atoi(argv[5]);
	record->fee = atoi(argv[6]);

	(*pMap)[record->eth_txid] = record;
	return 0;
}
bool select_safe2eth(mySQLiteDB& db, std::string tab, Safe2EthMap& safe2ethMap)
{
	std::string sql = "SELECT * FROM " + tab + "  WHERE eth_txid == '' OR eth_txid IS NULL;";
	bool bRet = db.exec(sql, &select_callback_safe2eth, &safe2ethMap);
	if (!bRet)
	{
		printf("cann't select data from db: %s.\n", tab.c_str());
	}
	return bRet;
}
bool select_eth2safe(mySQLiteDB& db, std::string tab, Eth2SafeMap& mymap)
{
	std::string sql = "SELECT * FROM " + tab + "  WHERE txid == '' OR txid IS NULL;";
	bool bRet = db.exec(sql, &select_callback_eth2safe, &mymap);
	if (!bRet)
	{
		printf("cann't select data from db: %s.\n", tab.c_str());
	}
	return bRet;
}
bool send_eth2safe(eth2safe & item)
{
	safenode safe;
	std::string txid;
	THROW_RETURN(txid = safe.sendtoaddress(item.safe_address, item.amount),false);
	item.txid = txid;
	item.n = 0;

	gettransaction_t tx;
	THROW_NO_RETURN(tx = safe.gettransaction(txid, true));
	item.fee = tx.fee;

	return true;
}
//safe,eth, 对账
bool checkfinance(mySQLiteDB& db)
{
	HttpClient* httpClient = new HttpClient(g_noderpc_url);
	Client* client = new Client(*httpClient, JSONRPC_CLIENT_V2);
	httpClient->SetTimeout(10 * 1000);//5 seconds

	std::string command = "getbalance";
	Json::Value params;

	Json::Value result;
	try
	{
		std::cout << "checkfinance::command: " << command << ",params: " << params << std::endl;
		result = client->CallMethod(command, params);
	}
	catch (JsonRpcException& e)
	{
		BitcoinException err(e.GetCode(), e.GetMessage());
		std::cout << "checkfinance error" << std::endl << std::endl;
		return false;
	}
	delete client;
	delete httpClient;

	std::cout << "checkfinance result: " << result << std::endl;

	if (result.isNull())
	{
		std::cout << "checkfinance rpc return error: " << result << std::endl << std::endl;
		return false;
	}

	double actual_contract_left = 0.0;
	//try
	//{
		actual_contract_left = result["amount"].asDouble();
	//}
	//catch (...)
	//{
	//	std::cout << "checkfinance return params number error" << result << std::endl << std::endl;
	//	return false;
	//}
	
	std::cout << "checkfinance safenode safe... ";
	safenode safe;
	std::cout << "checkfinance getBalanceByAddr... ";
	double actual_safe_left = safe.getBalanceByAddr(g_myAddress);

	//统计所有花费的SAFE交易费
	std::string sql = "SELECT sum(amount - safe_fee) AS count FROM " + db.tab;
	double income_contract = 0;
	bool bRet = db.exec(sql, &insert_callback, &income_contract);
	if (!bRet)
	{
		std::cout << "checkfinance: cann't sum income_contract: " << db.tab << std::endl;
		return false;
	}

	sql = "SELECT sum(amount) AS count FROM " + db.tab2;
	double outcome_contract = 0;
	bRet = db.exec(sql, &insert_callback, &outcome_contract);
	if (!bRet)
	{
		std::cout << "checkfinance: cann't sum outcome_contract: " << db.tab2 << std::endl;
		return false;
	}

	sql = "SELECT sum(fee) AS count FROM " + db.tab2;
	double safe_tx_fee = 0;
	bRet = db.exec(sql, &insert_callback, &safe_tx_fee);
	if (!bRet)
	{
		std::cout << "checkfinance: cann't sum safe_tx_fee: " << db.tab2 << std::endl;
		return false;
	}

	double should_left_contract = income_contract - outcome_contract;
	double should_left_safe = should_left_contract - safe_tx_fee;

	std::cout << "checkfinance: contract: income: " << income_contract << ", outcome: " << outcome_contract << std::endl;
	std::cout << "checkfinance: contract: actually left: " << actual_contract_left <<", should left = " << should_left_contract <<", diff = "<< actual_contract_left - should_left_contract << std::endl;
	std::cout << "checkfinance: safeaddr: actually left: " << actual_safe_left << ", shoule left: "<< should_left_safe <<", diff = "<< actual_safe_left - should_left_safe << std::endl << std::endl;

	return true;
}
//获得eth2safe列表
bool get_eth2safe(Eth2SafeMap& mymap)
{
	HttpClient* httpClient = new HttpClient(g_noderpc_url);
	Client* client = new Client(*httpClient, JSONRPC_CLIENT_V2);
	httpClient->SetTimeout(g_noderpc_timeout * 1000);//5 minutes

	std::string command = "eth2safe";
	Json::Value params;

	Json::Value result;
	try
	{
		std::cout << "get_eth2safe::command: " << command << ",params: " << params << std::endl;
		result = client->CallMethod(command, params);
	}
	catch (JsonRpcException& e)
	{
		BitcoinException err(e.GetCode(), e.GetMessage());
		std::cout << "get_eth2safe error" << std::endl << std::endl;
		return false;
	}
	delete client;
	delete httpClient;

	if (result.isNull())
	{
		std::cout << "get_eth2safe rpc return error: " << result << std::endl << std::endl;
		return false;
	}
	
	try
	{
		for (ValueIterator it = result.begin(); it != result.end(); it++)
		{
			eth2safe* safe = new eth2safe;
			safe->eth_txid =	 (*it)["eth_txid"].asString();
			safe->amount =		 (*it)["amount"].asDouble();
			safe->eth_address =  (*it)["eth_address"].asString();
			safe->safe_address = (*it)["safe_address"].asString();
			mymap[safe->eth_txid] = safe;

			std::cout << "get_eth2safe: eth_txid:  " << safe->eth_txid << ", amount: " << safe->amount << ", eth_address: " << safe->eth_address << ", safe_address: " << safe->safe_address << std::endl;
		}
	}
	catch (...)
	{
		std::cout << "get_eth2safe return params number error" << result << std::endl << std::endl;
		return false;
	}

	return true;
}
/*
request:

{
"jsonrpc":"2.0",
"id":"1",
"method":"safe2eth",
"params":
 {
	"dst":"0x9eF95776601dA991363a7A09667618f9FFFF0BD6",
	"amount":1000,
	"fee":10
 }
}

result:

{
	"jsonrpc": "2.0",
	"id": "1",
	"result": [
		"0xec51566f478a619e5057cc32ed1f1e61fddbf5d85fbe2f994cfc48404d5f5ebe",
		67425,
		"0x13f42507703b8ac6dcb67b54e1295158f9e41505875a285438b9ca61887250c4",
		160
	]
}
*/
//通过NODEJS RPC发送要发送的信息，需要nodejs配合
bool send_safe2eth(safe2eth& safe)
{
	HttpClient* httpClient = new HttpClient(g_noderpc_url);
	Client* client = new Client(*httpClient, JSONRPC_CLIENT_V2);
	httpClient->SetTimeout(g_noderpc_timeout * 1000);//5 minutes

	std::string command = "safe2eth";
	Json::Value params;

	params.append(safe.eth_address);
	double amount = safe.amount - g_txfee;

	if (amount <= 0)
	{
		std::cout << "send_safe2eth::amount less than or equel to zero. skipping..." << std::endl;
		return false;
	}

	params.append(safe.amount);
	params.append(g_txfee);

	Value result;
	try
	{
		std::cout << "send_safe2eth::command: " << command << ",params: " << params << std::endl;
		result = client->CallMethod(command, params);
	}
	catch (JsonRpcException& e)
	{
		BitcoinException err(e.GetCode(), e.GetMessage());
		std::cout << "send_safe2eth error" << std::endl << std::endl;
		return false;
	}
	delete client;
	delete httpClient;

	if (result.isNull() || !result.isArray() || result[0].asString().empty())
	{
		std::cout << "send_safe2eth rpc return error: " << result << std::endl << std::endl;
		return false;
	}

	try
	{
		safe.eth_txid = result[0].asString();
		safe.eth_fee = result[1].asDouble();
		safe.eth_blockhash = result[2].asString();
		safe.eth_blockindex = result[3].asInt();
	}
	catch (...)
	{
		std::cout << "send_safe2eth return params number error" << result << std::endl << std::endl;
		return false;
	}
	std::cout << "send_safe2eth: txid:  " << safe.eth_txid << ", fee: " << safe.eth_fee << ", hash: " << safe.eth_blockhash << ", index: " << safe.blockindex << std::endl;

	return true;
}
//从sqlite3数据库的safe3eth表中获得要求在ETH获得SAFE的列表，并且通过NODEJS WEB3在ETH网络上发送SAFE
static int safe2eth_thread(mySQLiteDB& db, std::string& tab)
{
	std::string name = "safe2eth_thread";
	Safe2EthMap mymap;
	std::cout << name << ": preparing to send safe to eth...\n";

	//从数据库中读取
	if (!select_safe2eth(db, tab, mymap))
	{
		std::cout << name << ": error happened to select_eth2safe: " << tab.c_str() << std::endl;
		return 0;
	}

	//逐一通过send_safe2eth发送
	std::cout << name << ": sending safe to eth, count: " << mymap.size() << std::endl;
	for (Safe2EthMapIterator it = mymap.begin(); it != mymap.end(); it++)
	{
		bool bSuccess = send_safe2eth(*it->second);
		if (bSuccess == false)
		{
			delete it->second;
			continue;
		}

		std::cout << name << ": txid: " << it->second->txid << ",amount: " << it->second->amount << ", eth_txid:" << it->second->eth_txid << std::endl;
		
		//把发送结果保存至数据库safe2eth表
		bool bRet = update_safe2eth(db, tab, *it->second);

		if (!bRet)
			std::cout << name << ": cann't update to db: txid: " << it->second->txid << ", eth_txid:" << it->second->eth_txid << std::endl;

		delete it->second;
	}

	mymap.clear();
	std::cout << name << ": exiting...\n\n";

	return 1;
}
//从sqlite3数据库的eth2safe表中读取列表, 逐一向SAFE网络发送SAFE
static int eth2safe_thread(mySQLiteDB& db, std::string& tab)
{
	std::string name = "eth2safe_thread";
	Eth2SafeMap mymap;
	std::cout << name << ": preparing to send SAFE@eth to SAFE network...\n";

	if (!select_eth2safe(db, tab, mymap))
	{
		std::cout << name << ": error happened to select_eth2safe: " << tab.c_str()<<std::endl;
		return 0;
	}

	std::cout << name << ": sending safe to SAFE network, count: " << mymap.size() << std::endl;
	for (Eth2SafeMapIterator it = mymap.begin(); it != mymap.end(); it++)
	{
		bool bSuccess = send_eth2safe(*it->second);
		if (bSuccess == false)
		{
			delete it->second;
			continue;
		}

		std::cout << name << ": eth_txid: " << it->second->eth_txid << ",amount: " << it->second->amount << ", txid:" << it->second->txid << std::endl;

		bool bRet = update_eth2safe(db, tab, *it->second);

		if (!bRet)
			std::cout << name << ": cann't update to db: eth_txid: " << it->second->eth_txid << ", txid:" << it->second->txid << std::endl;

		delete it->second;
	}

	mymap.clear();
	std::cout << name << ": exiting...\n\n";

	return 1;
}

static int mainthread(mySQLiteDB& db, std::string& myAddress, int& needed_confirms)
{
	safenode node;
	Safe2EthMap safe2ethMap;
	Eth2SafeMap eth2safeMap;
	std::cout << "\nmainthread: preparing to get SAFEs ready to enter eth...\n";

	while (!g_bExit)
	{
		//从sqlite3数据库的safe3eth表中获得要求在ETH获得SAFE的列表，并且通过NODEJS WEB3在ETH网络上发送SAFE
		std::cout << "mainthread: start safe2eth thread...\n";
		std::thread safe2eth(safe2eth_thread, std::ref(db), std::ref(db.tab));
		if (safe2eth.joinable()) safe2eth.join();

		//通过NODEJS RPC获得ETH网络上销毁SAFE在安网上发送SAFE的列表，并且存贮进sqlite3数据库的eth2safe表中
		std::cout << "mainthread: start eth2safe thread...\n";
		std::thread eth2safe(eth2safe_thread, std::ref(db), std::ref(db.tab2));
		if (eth2safe.joinable()) eth2safe.join();

		//获得目前最新区块索引
		int nToIndex = node.getBlockCount() - 1;

		printf("mainthread: getting blocks hashes %d, from: %d to %d\n", nToIndex - g_nBeginIndex + 1, g_nBeginIndex, nToIndex);

		StringVec hashs;
		//获得所有最新的区块HASH列表
		while (hashs.size() == 0)
		{
			THROW_NO_RETURN(node.getBlockHashes(g_nBeginIndex, nToIndex, hashs));
			if (hashs.size() == 0)
			{
				printf("mainthread: can't get required block hashs, waiting for 5 seconds...\n");
				std::this_thread::sleep_for(std::chrono::milliseconds(5 * 1000));
			}
		}

		if ((int)hashs.size() != nToIndex - g_nBeginIndex + 1)
		{
			printf("mainthread: warning: required hashs: %d, however we got %d hashs.\n", nToIndex - g_nBeginIndex + 1, (int)hashs.size());
		}
		//获得从SAFE兑换到SAFE@eth的列表到safe2ethMap中，要求是发送到myAddress，确认数到达needed_confirms，发送的SAFE金额最小为g_min_value
		THROW_NO_RETURN(node.getSafe2EthList(hashs, safe2ethMap, myAddress, needed_confirms, g_min_value));
		std::cout << "mainthread: safe2eth count: " << safe2ethMap.size() << std::endl;

		//存贮到数据库的safe2eth表中，以备safe2eth_thread线程发送
		for (Safe2EthMapIterator it = safe2ethMap.begin(); it != safe2ethMap.end(); it++)
		{
			bool bRet = insert_safe2eth(db, db.tab, *it->second);
			if (bRet)
				std::cout << "mainthread safe2eth: blockindex:" << it->second->blockindex << ", txid: " << it->first << ", amount: " << it->second->amount << ", eth: " << it->second->eth_address << std::endl;
			else
				std::cout << "mainthread safe2eth: insert_safe2eth error ,blockindex:" << it->second->blockindex << ", txid: " << it->first << std::endl;

			delete it->second;
		}
		//获得从SAFE@eth兑换到SAFE的列表
		if (get_eth2safe(eth2safeMap) == false)
		{
			//出错就准备下一次
			std::cout << "mainthread: get_eth2safe error: " << std::endl;
			safe2ethMap.clear();
			g_nBeginIndex = nToIndex - needed_confirms;

			std::cout << "mainthread: sleep_for " << g_scan_interval << " seconds, waiting for new tx...\n\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(g_scan_interval * 1000));
			continue;
		}
		//存贮到数据库的eth2safe表中
		std::cout << "mainthread: eth2safe count: " << eth2safeMap.size() << std::endl;
		for (Eth2SafeMapIterator it = eth2safeMap.begin(); it != eth2safeMap.end(); it++)
		{
			bool bRet = insert_eth2safe(db, db.tab2, *it->second);
			if (bRet)
				std::cout << "mainthread insert_eth2safe: eth_txid:" << it->second->eth_txid << ", amount: " << it->second->amount << ", eth_address: " << it->second->eth_address << std::endl;
			else
				std::cout << "mainthread insert_eth2safe: error ,eth_txid:" << it->second->eth_txid <<  std::endl;

			delete it->second;
		}
		
		safe2ethMap.clear(); eth2safeMap.clear();

		//返回needed_confirms个块再扫描，防止漏掉上次未达到确认数的交易
		g_nBeginIndex = nToIndex - needed_confirms;

		//休眠g_scan_interval秒
		std::cout << "mainthread: sleep_for " << g_scan_interval << " seconds, waiting for new tx...\n\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(g_scan_interval * 1000));

		//检查财务情况
		checkfinance(db);
	}

	return 1;
}
//保存配置参数
bool saveconfig(std::string& configfile)
{
	Json::Value config;
	Json::StyledWriter  json;
	std::ofstream strJsonContent(configfile.c_str(), std::ios::out|std::ios::trunc);

	config["g_nBeginIndex"] = Json::Value(g_nBeginIndex);
	config["g_myAddress"] = Json::Value(g_myAddress);
	config["g_need_confirms"] = Json::Value(g_need_confirms);
	config["g_min_value"] = Json::Value(g_min_value);
	config["g_txfee"] = Json::Value(g_txfee);
	config["g_scan_interval"] = Json::Value(g_scan_interval);
	config["g_noderpc_timeout"] = Json::Value(g_noderpc_timeout);
	config["g_noderpc_url"] = Json::Value(g_noderpc_url);
	config["g_sqlitedb_file"] = Json::Value(g_sqlitedb_file);

	strJsonContent << json.write(config) << std::endl;
	strJsonContent.close();
	return true;
}
//读取配置参数
bool loadconfig(std::string& configfile)
{
	std::ifstream strJsonContent(configfile.c_str(), std::ios::in);
	Json::Reader reader;
	Json::Value config;

	//如果没有配置文件,就先生成一个新的
	if (!strJsonContent.is_open())
	{
		std::cout << "Error opening config file: " << configfile << ", so this program will create this config file." << std::endl;
		saveconfig(configfile);
		return true;
	}
	//以JSON方式解析配置文件内容
	if (!reader.parse(strJsonContent, config))
	{
		std::cout << "Error parse config file: " << configfile << std::endl;
		strJsonContent.close();
		return false;
	}
	//获得所有配置参数
	if (config.isMember("g_nBeginIndex"))
		g_nBeginIndex = config["g_nBeginIndex"].asInt();//从该区块号开始处理

	if (config.isMember("g_myAddress"))
		g_myAddress = config["g_myAddress"].asString();//接收的兑换地址

	if (config.isMember("g_need_confirms"))
		g_need_confirms = config["g_need_confirms"].asInt(); //确认数，达到确认数才开始处理

	if (config.isMember("g_min_value"))
		g_min_value = config["g_min_value"].asInt();//发送的最小SAFE金额

	if (config.isMember("g_txfee"))
		g_txfee = config["g_txfee"].asDouble();    //要扣除的SAFE@ETH费用

	if (config.isMember("g_scan_interval"))
		g_scan_interval = config["g_scan_interval"].asInt(); // 多长时间开始扫描最新交易：30 seconds

	if (config.isMember("g_noderpc_timeout"))
		g_noderpc_timeout = config["g_noderpc_timeout"].asInt();//与nodesjs的联系多长时间超时，默认5分钟

	if (config.isMember("g_noderpc_url"))
		g_noderpc_url = config["g_noderpc_url"].asString();//node js的RPC网址和端口

	if (config.isMember("g_sqlitedb_file"))
		g_sqlitedb_file = config["g_sqlitedb_file"].asString();//数据库文件名称

	strJsonContent.close();
	Json::StyledWriter  json;
	std::cout << json.write(config) << std::endl;
	return true;
}
//获得命令行参数
bool usage(int argc, char* argv[])
{
	std::string config;
	printf("\nSafe2Eth: Monitoring SAFE network and transfer SAFE to the ETH network.\n \
Usage: safe2eth [config.json]\n\n \
\tnote: config.json is a json style config file.\n \
\tif it is not provided, then \"config.json\" will be the default config file.\n\n");

	//配置文件名
	if (argc == 1)
	{
		config = "config.json";
	}
	else if (argc == 2)
	{
		config = argv[1];
	}
	else
	{
		printf("too many params: %d, should be no more than %d.\n", argc, 1);
		return false;
	}
	printf("load params from config file: %s...\n", config.c_str());

	return loadconfig(config);
}

int main(int argc, char* argv[])
{
	bool bRet = usage(argc, argv);
	if (bRet == false) return 0;

	mySQLiteDB db(g_sqlitedb_file);

	std::thread main(mainthread, std::ref(db), std::ref(g_myAddress), std::ref(g_need_confirms));

	if (main.joinable()) main.join();

	db.close();

	return 1;
}

