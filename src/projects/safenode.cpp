#include <math.h>
#include <stdio.h>
#include <fstream>
#include <bitcoinapi/bitcoinapi.h>
#include <bitcoinapi/exception.h>


#define NO_THROW(METHOD,RET)  try {(METHOD);} catch (BitcoinException& e) {           \
    std::cout << "Error (" << e.getCode() << "): "<< e.getMessage() << std::endl; return RET;}

typedef std::map<std::string,double> MasterNodeMap;
typedef std::map<std::string,double>::const_iterator ValueIterator;

struct safenode {

	std::string username;
	std::string password;
	std::string address;
	int port;

	BitcoinAPI safe;

    safenode(): username("safe"),password("safe"),address("127.0.0.1"),
       port(5556), safe(username, password, address, port){ }

    ~safenode() { }

	bool GetInfo()
	{
		getinfo_t info;
		
		NO_THROW(info = safe.getinfo(),false);

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
		return true;
	}

	bool GetBlockHashes(const int from_blocknumber,const int to_blocknumber,std::vector<std::string> &ret) 
	{
		std::string fromhash,tohash;
		NO_THROW(fromhash = safe.getblockhash(from_blocknumber),false);
		NO_THROW(tohash = safe.getblockhash(to_blocknumber),false);

		std::cout << "From: " << fromhash << ", To: " << tohash << std::endl << std::endl;

		blockinfo_t from,to;
		NO_THROW(from = safe.getblock(fromhash),false);
		NO_THROW(to = safe.getblock(tohash),false);

		std::cout << "From: " << from.time << ", To: " << to.time << std::endl << std::endl;

		bool bRet = false;
		NO_THROW(bRet = safe.getblockhashes(to.time,from.time,ret),false);

		return bRet;
	}

	bool GetBlockHashesEx(const int from_blocknumber,const int to_blocknumber,std::vector<std::string> &ret) 
	{
		std::string hash;
		int Count = 0;
		double percent = 0.0;
		 
		std::cout.precision(2);

		for(int i = from_blocknumber; i <= to_blocknumber; i++)
		{
			NO_THROW(hash = safe.getblockhash(i),false);
			ret.push_back(hash);
			percent = 100 *  Count/(to_blocknumber-from_blocknumber+1);
			std::cout << "\rgetblcokhashes progress: " << percent << "%" << std::flush;
			Count++;
		}
		std::cout << std::endl;
		return true;
	}

	bool GetMasterNodeStatistics(const std::vector<std::string> &hashs,MasterNodeMap &mining_mnmap,MasterNodeMap &reward_mnmap)
	{
		std::cout.precision(2);
		//get blocks from hashs and decode it
		int hashCount = hashs.size();
		int Count = 0;
		double percent = 0;

		for(std::vector<std::string>::const_iterator it_hash = hashs.begin(); it_hash != hashs.end(); it_hash++)
		{
			std::string hash = *it_hash;
			blockinfo_t bc;
			NO_THROW(bc = safe.getblock(hash),0);
			
			//decode transactions
			for(std::vector<std::string>::const_iterator it_tx = bc.tx.begin(); it_tx != bc.tx.end(); it_tx++)
			{
				getrawtransaction_t raw_tx;
				NO_THROW(raw_tx = safe.getrawtransaction(*it_tx,true),0);

				//if it is not coinbase, then continue. we need coinbase tx.
				if(!(raw_tx.vin.size() == 1 && raw_tx.vin[0].txid.empty())) continue;
				
				//mining nodes and rewards
				std::string addr_mining = raw_tx.vout[0].scriptPubKey.addresses[0];
				ValueIterator itr = mining_mnmap.find(addr_mining);
				
				if(itr != mining_mnmap.end())
				{
					mining_mnmap[addr_mining] = raw_tx.vout[0].value + (double)itr->second;
				}
				else
				{
					mining_mnmap[addr_mining] = raw_tx.vout[0].value;
				}
				

				//masternodes and rewards
				std::string addr_reward = raw_tx.vout[1].scriptPubKey.addresses[0];
				itr = reward_mnmap.find(addr_reward);

				if(itr != reward_mnmap.end())
				{
					reward_mnmap[addr_reward] = raw_tx.vout[1].value + (double)itr->second;
				}
				else
				{
					reward_mnmap[addr_reward] = raw_tx.vout[1].value;
				}
				
			}
			percent = 100*  Count/hashCount;
			std::cout << "\rGetMasterNodeStatistics progress: " << percent << "%" << std::flush;
			Count++;
		}
		std::cout << std::endl;
		return true;
	}
};

double getmasternoderewardvalue(const MasterNodeMap &mnmap)
{
	double sum = 0.0;
	for(ValueIterator it = mnmap.begin(); it != mnmap.end(); it++)
	{
		sum += (double)it->second;
	}
	return sum;
}

double getmasternodevaluepercent(const double mining_sum, const MasterNodeMap &mnmap,MasterNodeMap &dstmap)
{
	double sum = 0;
	for(ValueIterator it = mnmap.begin(); it != mnmap.end(); it++)
	{
		sum += (double)it->second;
	}
	
	for(ValueIterator it = mnmap.begin(); it != mnmap.end(); it++)
	{
		dstmap[(std::string)it->first] = mining_sum * (double)it->second/sum;
	}
	return sum;
}

void outputmap(const MasterNodeMap &mnmap)
{
	std::cout << std::endl << "address, amount" << std::endl;
	for(ValueIterator it = mnmap.begin(); it != mnmap.end(); it++)
	{
		std::cout << it->first << "," << it->second << std::endl;
	}
	std::cout << std::endl;
}

bool outputmap_file(const MasterNodeMap &mnmap)
{
	std::ofstream of("out.txt");

	if(!of)
	{
		std::cout << "can't create file out.txt" << std::endl;
		return false;
	}

	for(ValueIterator it = mnmap.begin(); it != mnmap.end(); it++)
	{
		of << it->first << "," << it->second << std::endl;
	}

	of.close();
	return true;
}

int main(int argc,char* argv[])
{
	safenode node;
	int nBlockCount = 0 ;
	int nMasterNodeCount = 0 ;

	printf("Calurate the mining rewards of main nodes in the SAFE network.\nUsage: safenode nFromIndex nToIndex\n\n\n");
	if(argc != 3) 
	{
		printf("error argc num: %d, should be 2 params. exiting...\n",argc);
		return 0;
	}

	int nFromIndex = atoi(argv[1]);
	int nToIndex   = atoi(argv[2]);

	if(nFromIndex < 1500000 || nToIndex <= nFromIndex)
	{
		printf("wrong argc: from %d to %d, check your params. exiting...\n",nFromIndex,nToIndex);
		return 0;
	}

	NO_THROW(node.GetInfo(),0);

	NO_THROW(nBlockCount = node.safe.getblockcount(),0);
	printf("current blockcount: %d\n",nBlockCount);

	if(nFromIndex >= nBlockCount)
	{
		printf("current blockcount: %d, wrong nFromIndex: %d. exiting...\n",nBlockCount,nFromIndex);
		return 0;
	}

	if(nToIndex > nBlockCount)
	{
		printf("current blockcount: %d, so nToIndex will be set to it.\n",nBlockCount);
		nToIndex = nBlockCount;
	}

	NO_THROW(nMasterNodeCount = node.safe.getmasternodecount(),0);
	printf("current masternode count: %d\n",nMasterNodeCount);

	printf("getting  blocks hashes %d, from: %d to %d\n",nToIndex-nFromIndex+1,nFromIndex,nToIndex);
    std::vector<std::string> hashs;
	NO_THROW(node.GetBlockHashesEx(nFromIndex,nToIndex,hashs),0);
	
	if(hashs.size() == 0)
	{
		printf("can't get required hashs, exiting...\n");
		return 0;
	}

	if((int)hashs.size() != nToIndex - nFromIndex + 1)
	{
		printf("warning: required hashs: %d, however we got %d hashs.\n",nToIndex - nFromIndex+1,(int)hashs.size());
	}
	
	MasterNodeMap mining_map,reward_map,percent_reward_map;
	
	NO_THROW(node.GetMasterNodeStatistics(hashs,mining_map,reward_map),0);

	printf("blocks_count = %d,mining_map: %d, reward_map: %d.\n",(int)hashs.size(),(int)mining_map.size(),(int)reward_map.size());
	
	double mining_sum = getmasternoderewardvalue(mining_map);

	printf("mining nodes: %d, mining value: %0.2f\n",(int)mining_map.size(),mining_sum); 

	outputmap(mining_map);

	double reward_sum = getmasternodevaluepercent(mining_sum,reward_map,percent_reward_map);
	printf("master nodes: %d, reward value: %0.2f\n",(int)reward_map.size(),reward_sum); 
	
	outputmap_file(percent_reward_map);
	printf("output master node reward fie to: output.txt\n"); 
	
	return 1;
}

