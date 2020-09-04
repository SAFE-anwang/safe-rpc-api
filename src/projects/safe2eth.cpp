#include <math.h>
#include <stdio.h>
#include <fstream>
#include <bitcoinapi/safeapi.h>
#include <bitcoinapi/exception.h>


#define NO_THROW(METHOD,RET)  try {(METHOD);} catch (BitcoinException& e) {           \
    std::cout << "Error (" << e.getCode() << "): "<< e.getMessage() << std::endl; return RET;}

typedef std::map<std::string,double> MasterNodeMap;
typedef std::map<std::string,double>::const_iterator ValueIterator;

class safenode: public SafeAPI
{
    safenode():SafeAPI(){getInfo();}
    ~safenode() { }

	bool getInfo()
	{
		getinfo_t info;
		
		NO_THROW(info = getinfo(),false);

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
}

int main(int argc,char* argv[])
{
	safenode node;
	int nBlockCount = 0 ;
	int nMasterNodeCount = 0 ;

	std::string bitcoinaddress = "XbGhY34UiGcu8PNwWbM2PAFkpRGYp73MES";
	
	double amount = node.getreceivedbyaddress(bitcoinaddress,1);
	std::cout << bitcoinaddress <<":amount " << info.errors << amount << std::endl;
	return 1;
}

