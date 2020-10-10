#include <bitcoinapi/safeapi.h>
#include <bitcoinapi/exception.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <sstream>    
#include <string>
#include <fstream>
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
}safe2eth;

typedef std::map<std::string, safe2eth*> Safe2EthMap;
typedef std::map<std::string, safe2eth*>::const_iterator Safe2EthMapIterator;

typedef std::vector<std::string> StringVec;


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


struct safenode : public SafeAPI
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

	bool getSafe2EthList(const StringVec& hashs, Safe2EthMap& safemap, const std::string& myAddr, const int min_confirms)
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

					//如果发送的不是SAFE，或者有锁定时间，都跳过。
					if (it_vout->txType != 1 || it_vout->nUnlockedHeight != 0) continue;

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
};

int nBeginIndex = 2371250;
std::string myAddress = "XnfpiZJCDwgeJ6MdV9WQPUxX1MuCviZFbe";
int need_confirms = 30;
bool bExit = false;

bool usage(int argc, char* argv[])
{
	printf("\nSafe2Eth: Transfer SAFE to the ETH network.\nUsage: safe2eth [address=XnfpiZJCDwgeJ6MdV9WQPUxX1MuCviZFbe] [nBeginIndex=2371250]\n\n\n");
	if (argc == 1) return true;
	if (argc == 2)
	{
		myAddress = atoi(argv[1]);
		printf("address: %s accepted.\n", myAddress.c_str());
	}
	if (argc == 3)
	{
		int nFromIndex = atoi(argv[2]);
		if (nFromIndex > nBeginIndex)
			nBeginIndex = nFromIndex;
		else
			printf("too small index: %d, should be larger than %d.\n", nFromIndex, nBeginIndex);

		printf("blockchain index: %d accepted.\n", nBeginIndex);
	}
	if (argc > 3)
	{
		printf("error params count: %d, should be no more than %d. exit...\n", argc, 3);
		return false;
	}
	return true;
}

int getBlockCount(safenode& node)
{
	int nBlockCount = 0;
	do
	{
		THROW_RETURN(nBlockCount = node.getblockcount(), 0);
		printf("current blockcount: %d\n", nBlockCount);

		if (nBeginIndex >= nBlockCount)
		{
			printf("blockchain count: %d, beginIndex: %d, blockchain sync not completed, waiting for 5 minutes...\n", nBlockCount, nBeginIndex);
			std::this_thread::sleep_for(std::chrono::milliseconds(5 * 60 * 1000));
		}

	} while (nBeginIndex >= nBlockCount);

	return nBlockCount;
}

bool updateDB(mySQLiteDB& db, std::string tab, safe2eth& eth)
{
	char sqlBuff[4096] = { 0 };
	if (eth.eth_txid.empty())
		sprintf(sqlBuff, "UPDATE %s SET confirmations = %d WHERE txid == '%s' ;", tab.c_str(), eth.confirmations, eth.txid.c_str());
	else
		sprintf(sqlBuff, "UPDATE %s SET eth_txid = '%s', eth_fee = %f, eth_blockhash = '%s', eth_blockindex = %d WHERE txid == '%s' ;", tab.c_str(), eth.eth_txid.c_str(),eth.eth_fee,eth.eth_blockhash.c_str(),eth.eth_blockindex, eth.txid.c_str());
	std::string sql = sqlBuff;
	bool bRet = db.exec(sql, nullptr, nullptr);
	if (!bRet)
	{
		printf("cann't update data to db: %s\n", eth.txid.c_str());
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
bool insertDB(mySQLiteDB& db, std::string tab, safe2eth& eth)
{
	std::string sql = "SELECT count(txid) AS count FROM " + tab + "  WHERE txid == '" + eth.txid + "';";
	int nRow = 0;
	bool bRet = db.exec(sql, &insert_callback, &nRow);
	if (!bRet)
	{
		std::cout << "cann't select count from db: " << eth.txid.c_str() << std::endl;
	}

	if (nRow != 0) //已经有该交易ID，则更新确认数
		return updateDB(db, tab, eth);

	//没有该交易ID则插入整行数据
	char sqlBuff[4096] = { 0 };
	sprintf(sqlBuff, "INSERT INTO %s VALUES('%s',%d,%f,%d,%u,'%s','%s','%s','%s',0,'',0);", tab.c_str(), eth.txid.c_str(), eth.n, eth.amount, eth.confirmations, \
		eth.blockindex, eth.blockhash.c_str(), eth.safe_address.c_str(), eth.eth_address.c_str(), eth.eth_txid.c_str());
	sql = sqlBuff;
	bRet = db.exec(sql, nullptr, nullptr);
	if (!bRet)
	{
		printf("cann't insert data to db: %s\n", eth.txid.c_str());
	}
	return bRet;
}

int select_callback(void* data, int argc, char** argv, char** azColName)
{
	Safe2EthMap* psafe2ethMap = (Safe2EthMap*)data;
	if (psafe2ethMap == nullptr) return 0;
	if (argc != 12) return 0;

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

	(*psafe2ethMap)[record->txid] = record;
	return 0;
}

bool selectDB(mySQLiteDB& db, std::string tab, Safe2EthMap& safe2ethMap)
{
	std::string sql = "SELECT * FROM " + tab + "  WHERE eth_txid == '' OR eth_txid IS NULL;";
	bool bRet = db.exec(sql, &select_callback, &safe2ethMap);
	if (!bRet)
	{
		printf("cann't select data from db: %s, waiting for 5 minutes.\n", tab.c_str());
	}
	return bRet;
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
bool send_safe2eth(safe2eth& safe)
{
	HttpClient* httpClient = new HttpClient("http://127.0.0.1:50505");
	Client* client = new Client(*httpClient, JSONRPC_CLIENT_V2);
	httpClient->SetTimeout(5000);

	std::string command = "safe2eth";
	Json::Value params;

	params.append(safe.eth_address);
	double fee = 0.1; 
	double amount = safe.amount - fee;

	if (amount <= 0)
	{
		std::cout << "send_safe2eth::amount less than or equel to zero. skipping..."  << std::endl;
		return false;
	}

	params.append(amount);
	params.append(fee);

	Value result;
	try
	{
		std::cout << "send_safe2eth::command: " << command << ",params: " << params <<std::endl;
		result = client->CallMethod(command, params);
	}
	catch (JsonRpcException& e)
	{
		BitcoinException err(e.GetCode(), e.GetMessage());
		throw err;
		return false;
	}
	delete client;
	delete httpClient;

	if (result[0].asString().empty()) return false;

	safe.eth_txid = result[0].asString();
	safe.eth_fee =  result[1].asDouble();
	safe.eth_blockhash = result[2].asString();
	safe.eth_blockindex = result[3].asInt();

	return true;
}

static int sendthread(mySQLiteDB& db, std::string& tab)
{
	Safe2EthMap safe2ethMap;
	std::cout << "\nsendthread: preparing to send safe to eth...\n";

	if (!selectDB(db, tab, safe2ethMap))
	{
		printf("sendthread: error happened to selectDB: %s, waiting for 1 minutes\n", tab.c_str());
		//std::this_thread::sleep_for(std::chrono::milliseconds(1 * 60 * 1000));
		return 0;
	}

	std::cout << "sendthread: sending safe to eth, count: " << safe2ethMap.size() << std::endl;
	for (Safe2EthMapIterator it = safe2ethMap.begin(); it != safe2ethMap.end(); it++)
	{
		bool bSuccess = send_safe2eth(*it->second);
		if (bSuccess == false)
		{
			delete it->second;
			continue;
		}

		std::cout << "sendthread: txid: " << it->second->txid << ",amount: " << it->second->amount << ", eth_txid:" << it->second->eth_txid << std::endl;

		bool bRet = updateDB(db, tab, *it->second);

		if (!bRet)
			std::cout << "sendthread: cann't update to db: txid: " << it->second->txid << ", eth_txid:" << it->second->eth_txid << std::endl;

		delete it->second;
	}

	safe2ethMap.clear();
	std::cout << "sendthread: exiting...\n\n";
	//std::cout << "sendthread: sleep_for 5 minutes, waiting for new tx...\n\n";
	//std::this_thread::sleep_for(std::chrono::milliseconds(5 * 60 * 1000));

	return 1;
}


static int mainthread(mySQLiteDB& db, std::string& tab, std::string& myAddress, int& needed_confirms)
{
	safenode node;
	Safe2EthMap safe2ethMap;
	std::cout << "\nmainthread: preparing to get SAFEs ready to enter eth...\n";
	while (!bExit)
	{
		int nToIndex = getBlockCount(node) - 1;

		printf("mainthread: getting blocks hashes %d, from: %d to %d\n", nToIndex - nBeginIndex + 1, nBeginIndex, nToIndex);

		StringVec hashs;

		while (hashs.size() == 0)
		{
			THROW_NO_RETURN(node.getBlockHashes(nBeginIndex, nToIndex, hashs));
			if (hashs.size() == 0)
			{
				printf("mainthread: can't get required block hashs, waiting for 5 seconds...\n");
				std::this_thread::sleep_for(std::chrono::milliseconds(5 * 1000));
			}
		}

		if ((int)hashs.size() != nToIndex - nBeginIndex + 1)
		{
			printf("mainthread: warning: required hashs: %d, however we got %d hashs.\n", nToIndex - nBeginIndex + 1, (int)hashs.size());
		}

		THROW_NO_RETURN(node.getSafe2EthList(hashs, safe2ethMap, myAddress, needed_confirms));
		std::cout << "mainthread: safe2eth count: " << safe2ethMap.size() << std::endl;
		for (Safe2EthMapIterator it = safe2ethMap.begin(); it != safe2ethMap.end(); it++)
		{
			bool bRet = insertDB(db, tab, *it->second);
			if (bRet)
				std::cout << "mainthread: blockindex:" << it->second->blockindex << ", txid: " << it->first << ", amount: " << it->second->amount << ", eth: " << it->second->eth_address << std::endl;
			else
				std::cout << "mainthread: insertDB error ,blockindex:" << it->second->blockindex << ", txid: " << it->first << std::endl;

			delete it->second;
		}

		if (safe2ethMap.size())
		{
			std::cout << "mainthread: start sendthread...\n";
			std::thread send(sendthread, std::ref(db), std::ref(tab));
			if (send.joinable()) send.join();
		}
		safe2ethMap.clear();
		nBeginIndex = nToIndex - needed_confirms;

		std::cout << "mainthread: sleep_for 5 minutes, waiting for new tx...\n\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(5 * 60 * 1000));
	}

	return 1;
}

int main(int argc, char* argv[])
{
	bool bRet = usage(argc, argv);
	if (bRet == false) return 0;

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

	mySQLiteDB db("safe2eth.db", tab, sql);

	std::thread send(sendthread, std::ref(db), std::ref(tab));
	if (send.joinable()) send.join();

	std::thread main(mainthread, std::ref(db), std::ref(tab), std::ref(myAddress), std::ref(need_confirms));

	if (main.joinable()) main.join();

	db.close();

	return 1;
}

