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


#define NO_THROW(METHOD,RET)  try {(METHOD);} catch (BitcoinException& e) {           \
							 std::cout << "Error (" << e.getCode() << "): "<< e.getMessage() << std::endl; return RET;}

typedef struct 
{
	std::string txid;
	int nVout;
	std::string blockhash;
	std::string safe_address;
	double amount;
	std::string eth_address;
	int confirmations;
}safe2eth;

typedef std::map<std::string, safe2eth*> Safe2EthMap;
typedef std::map<std::string, safe2eth*>::const_iterator Safe2EthMapIterator;

typedef std::vector<std::string> StringVec;


 
/**
 * #purpose    : 字符转十六进制
 * #note    : 不适用于汉字字符
 * #param ch    : 要转换成十六进制的字符
 * #return    : 接收转换后的字符串
 */
std::string chToHex(unsigned char ch)
{
    const std::string hex = "0123456789ABCDEF";
 
    std::stringstream ss;
    ss << hex[ch >> 4] << hex[ch & 0xf];
 
    return ss.str();
}
 /*
// C prototype : void StrToHex(byte *pbDest, char *pszSrc, int nLen)
// parameter(s): [OUT] pbDest - 输出缓冲区
//	[IN] pszSrc - 字符串
//	[IN] nLen - 16进制数的字节数(字符串的长度/2)
// return value:
// remarks : 将字符串转化为16进制数
*/
void StrToHex(char*pbDest, const char *pszSrc, int nLen)
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
void HexToStr(char *pszDest,const char*pbSrc, int nLen)
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

    safenode(std::string username = "safe", std::string password = "safe", std::string address = "127.0.0.1", int port = 5554): \
		SafeAPI(username, password, address, port){ }
    ~safenode() { }

	void outputInfo(getinfo_t info)
	{
		std::cout <<  "=== getinfo ===" << std::endl;
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

	bool GetBlockHashes(const int from_blocknumber,const int to_blocknumber,StringVec &ret) 
	{
		std::string hash;
		int Count = 0;
		double percent = 0.0;
		 
		std::cout.precision(2);

		for(int i = from_blocknumber; i <= to_blocknumber; i++)
		{
			NO_THROW(hash = getblockhash(i),false);
			ret.push_back(hash);
			percent = 100 *  Count/(to_blocknumber-from_blocknumber+1);
			std::cout << "\rgetblcokhashes progress: " << percent << "%" << std::flush;
			Count++;
		}
		std::cout << std::endl;
		return true;
	}

	bool getEthAddr(const std::string & addString, std::string& eth)
	{
		if (addString.size() < 188) return false;
		int skipLen = 48;

		char* buff [3000];
		memset(buff,0,3000);

		StrToHex((char*)buff, addString.c_str(), addString.size()/2);
		char* eth_addr = (char*)buff + skipLen;

		//需要有ETH：这四个关键字，后面跟着ETH地址；
		if (memcmp(eth_addr, "eth:", 4) != 0)
		{
			std::cout << "getEthAddr: eth error: "<<eth_addr << std::endl;
			return false;
		}
		eth_addr = (char*)(eth_addr + 4);

		//去除前面和后面可能的空格
		while (*eth_addr == 20) { *eth_addr = 0; eth_addr++;}

		if (memcmp(eth_addr, "0x", 2) != 0 || strlen(eth_addr) != 42)
		{
			std::cout << "getEthAddr 0x: " << eth_addr << std::endl;
			return false;
		}

		eth = eth_addr;
		eth = trim(eth);
		return true;
	}

	bool getSafe2EthList(const StringVec &hashs,Safe2EthMap &safemap,const std::string & myAddr, const int min_confirms)
	{
		std::cout.precision(2);

		for(StringVec::const_iterator it_hash = hashs.begin(); it_hash != hashs.end(); it_hash++)
		{
			std::string hash = *it_hash;
			blockinfo_t bc;
			NO_THROW(bc = getblock(hash),0);
			
			//decode transactions
			for(StringVec::const_iterator it_tx = bc.tx.begin(); it_tx != bc.tx.end(); it_tx++)//block hashs
			{
				safe_getrawtransaction_t raw_tx;
				NO_THROW(raw_tx = safe_getrawtransaction(*it_tx,true),0);

				//if it is coinbase, then continue. we dont need coinbase tx.
				if(raw_tx.vin.size() == 1 && raw_tx.vin[0].txid.empty()) continue;

				//确认数量未到要求的min_confirms，则跳过。
				if(raw_tx.confirmations < (unsigned int)min_confirms) continue;
				
				//vout 
				for(std::vector<safe_vout_t> ::const_iterator it_vout = raw_tx.vout.begin(); it_vout != raw_tx.vout.end();it_vout++)
				{
					//如果发送的不是SAFE，或者有锁定时间，都跳过。
					if(it_vout->txType != 1 || it_vout->nUnlockedHeight != 0) continue;

					StringVec * pVec = (StringVec*)&((*it_vout).scriptPubKey.addresses);
					for(StringVec::const_iterator it_addr = pVec->begin(); it_addr != pVec->end(); it_addr++)//
					{
						std::string addr_recv = *it_addr;
						std::string eth;
						//如果对接地址对应，而且外带数据超过188个字节，有可能是我们要找的交易。
						if(myAddr.compare(addr_recv) == 0 && getEthAddr(it_vout->reserve, eth))
						{
							safe2eth* record = new safe2eth;
							record->eth_address = eth;
							record->txid = raw_tx.txid;
							record->blockhash = raw_tx.blockhash;
							record->safe_address = addr_recv;
							record->amount = it_vout->value;
							record->nVout = it_vout->n;
							record->confirmations = raw_tx.confirmations;
							safemap[record->txid] = record;
						}
					}					
				}
			}
		}

		return true;
	}
};

int main(int argc,char* argv[])
{
	safenode node;
	int nBeginIndex = 2371250, nToIndex = 0;
	int nBlockCount = 0 ;
	std::string myAddress = "XnfpiZJCDwgeJ6MdV9WQPUxX1MuCviZFbe";

	printf("\n\nSafe2Eth: Transfer SAFE to the ETH network.\nUsage: safe2eth [nBeginIndex=2371250]\n\n\n");
	if(argc == 2) 
	{
		int nFromIndex = atoi(argv[1]);
		if(nFromIndex > nBeginIndex) 
			nBeginIndex = nFromIndex;
		else 
			printf("too small index: %d, should be larger than %d.\n",nFromIndex,nBeginIndex);
		
		printf("blockchain index %d accepted.\n",nBeginIndex);
	}
	if(argc > 2)
	{
		printf("error params: %d, should be %d. exit...\n",argc,2);
		return 0;
	}

	NO_THROW(nBlockCount = node.getblockcount(),0);
	printf("current blockcount: %d\n",nBlockCount);

	if(nBeginIndex >= nBlockCount)
	{
		printf("blockchain: %d, beginIndex: %d, blockchain sync not completed, exiting...\n",nBlockCount,nBeginIndex);
		return 0;
	}
	
	nToIndex = nBlockCount -1;

	printf("getting blocks hashes %d, from: %d to %d\n",nToIndex-nBeginIndex + 1,nBeginIndex,nToIndex);
	
    StringVec hashs;
	NO_THROW(node.GetBlockHashes(nBeginIndex,nToIndex,hashs),0);
	
	if(hashs.size() == 0)
	{
		printf("can't get required hashs, exiting...\n");
		return 0;
	}

	if((int)hashs.size() != nToIndex - nBeginIndex + 1)
	{
		printf("warning: required hashs: %d, however we got %d hashs.\n",nToIndex - nBeginIndex+1,(int)hashs.size());
	}
	
	Safe2EthMap safe2ethMap;

	NO_THROW(node.getSafe2EthList(hashs, safe2ethMap, myAddress, 30),0);

	for (Safe2EthMapIterator it = safe2ethMap.begin(); it != safe2ethMap.end(); it++)
	{
		std::cout << "txid: " << it->first << ", amount: " << it->second->amount << ",eth: " << it->second->eth_address << std::endl;
	}

	return 1;
}

