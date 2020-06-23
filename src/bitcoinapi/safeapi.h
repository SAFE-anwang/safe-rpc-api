#ifndef SAFE_API_H
#define SAFE_API_H

#include "types.h"
#include "safe_types.h"
#include "exception.h"
#include "bitcoinapi.h"

class SafeAPI : public BitcoinAPI
{

    SafeAPI(const std::string username = "safe", const std::string password = "safe", const std::string& address = "127.0.0.1", int port = 5554):BitcoinAPI(username, password, address, port){ }

    ~SafeAPI() { }

	/* === MasterNode functions === */
	int  getmasternodecount();
	int  getmasternodelist();

	
    /************************************************** application module **************************************************/

    /*
    * regest app
    * @param:strAppName,required (Max. 50 bytes)
    * @param:strAppDesc,required (Max. 600 bytes)
    * @param:nDevType,required (1--company 2--individual)
    * @param:strDevName,required (Max. 100 bytes, If company, fill company name, If individual, fill individual name)
    * @param:strWebUrl,optional (Max. 300 bytes)
    * @param:strAppLogoUrl,optional (Max. 300 bytes)
    * @param:strAppCoverUrl,optional (Max. 300 bytes)
    * @return:admin safe address, app ID(start at 1001), transaction ID
    */
    registerapp_response_t registerapp(appdata_t & app);

    /*
    * set authorization 
    * @param:nDevType, required (1--add 2--delete)
    * @param:strAppId, required
    * @param:strUserSafeAddress, required
    * @param:dAppAuthCmd, required (Defined by the developers themselves, it can only be a number, range from 1000 to 4294967296)
    * @return:transaction ID
    */
    std::string setappauth(authdata_t & auth);

    /*
    * create extend data transaction
    * @param:nAppTxType, required (1--Normal transaction 2--Instant transaction)
    * @param:strUserSafeAddress, required
    * @param:strAppId, required
    * @param:dAppAuthCmd, required
    * @param:strExtendData, required (Max. 2986 bytes)
    * @return:transaction ID
    */
    std::string createextenddatatx(extenddata_t & ed);

    /*
    * Return external data according to the transaction ID
    * @param:strTxId
    * @return:extend data list
    */
    std::vector<extenddata_list_response_t> getextenddata(const std::string& strTxId);

    /*
    * Return transaction list based on AppID 
    * @param:strAppId, required
    * @param:nAppOperType, required (1--All 2--Registration application transaction 3--Set permission transaction 4--Create external data transactions)
    * @param:nSetType, required (Valid when appOperType=3, 1--add 2--delete)
    * @return:transaction list
    */
    std::vector<std::string> getapptxids(apptxids_t);

    /*
    * Return to trading list based on wallet address and AppID 
    * @param:strSafeAddress, required
    * @param:strAppId, required
    * @param:nAppOperType, optional (The app operator type, 1=all, 2=register, 3=setauth, 4=createextendatatx)
    * @param:nSetType, optional (The set auth type, it is valid when appOperType is 3)
    * @return:transaction list
    */
    std::vector<std::string> getaddressapptxids(address_apptxids_t & aat);

    /*
     * Get App list
     * @param:
     * @return: app list
     */
    std::vector<std::string> getapplist();

    /*
    * Get App info
    * @param:strAppId, required
    * @return:AppId infomation
    */
    appdata_t getappinfo(const std::string& strAppId);

    /*
    * Get App list by Address
    * @param:strAppAddress, required
    * @return:App list
    */
    std::vector<std::string> getapplistbyaddress(const std::string& strAppAddress);

    /*
    * Get detail App infomation
    * @param:strTxId, required
    * @return:App detail infomation
    */
    bool getappdetails(const std::string& strTxId, appdata_t & appData, authdata_response_t & ar, extenddata_response_t & er);

    /*
    * Get App auth list
    * @param:strAppId, required
    * @param:strAddress, required
    * @return:App detail infomation
    */
    bool getauthlist(const std::string& strAppId, const std::string& strAddress, std::vector<int> & all_user,std::vector<int> & current_user);

    /************************************************** safe asset module **************************************************/

    /*
    * issue asset
    * @param:strAssetShortName, required (Max. 20 bytes)
    * @param:strAssetName, required (Max. 20 bytes)
    * @param:strAssetDesc, required (Max. 300 bytes)
    * @param:strAssetUnit, required (Max. 10 bytes)
    * @param:dTotalAmount, required (The total amount of assets shall not exceed 200000000000000)
    * @param:dFirstIssueAmount, required (It can't be less than or equal to 0, it can't be greater than the total amount of assets)
    * @param:nAssetDecimals, required (The smallest 4 digits, the largest 10 digits, for example, 100000000, is 10 to the minus 8)
    * @param:bIsDestory, required (false -- not destructible,true-- destructiblee)
    * @param:bIsPayCandy, required (false -- no distribution, true-- distribution)
    * @param:dAssetCandyAmount, optional
    * @param:nCandyExpired, optional (Minimum 1 month and maximum 6 months)
    * @param:strRemarks, optional (Max. 500 bytes)
    * @return:asset ID and Tx ID;
    */
    assetdata_response_t issueasset(assetdata_t & ad);

    /*
    * add issue asset
    * @param:strAssetId, required
    * @param:dAddAmount, required
    * @param:strRemarks, optional (Max. 500 bytes)
    * @return:transaction ID;
    */
    std::string addissueasset(const asset_base_t & ab);

    /*
    * transfer asset
    * @param:strSafeAddress, required
    * @param:strAssetId, required
    * @param:dAssetAmount, required
    * @param:nLockTime, required (Minimum 1 month, maximum 120 months)
    * @param:strRemarks, optional (Max. 500 bytes)
    * @return:transaction ID;
    */
    std::string transferasset(const asset_transfer_t & at);

    /*
    * destory asset
    * @param:strAssetId, required
    * @param:dAssetAmount, required
    * @param:strRemarks, optional (Max. 500 bytes)
    * @return:transaction ID;
    */
    std::string destoryasset(const asset_transfer_t & at);

    /*
    * issue candy
    * @param:strAssetId, required
    * @param:dAassetCandyAmount, required
    * @param:nCandyExpired, required (Minimum 1 month and maximum 6 months)
    * @param:strRemarks, optional (Max. 500 bytes)
    * @return:transaction ID;
    */
    std::string putcandy(const candy_info_t & pc);

    /*
    * get candy
    * @param:strAssetId, required
    * @return:transaction ID and asset amount;
    */
    std::map<std::string,double> getcandy(const std::string& strAssetId);

    /*
    * Get available candy list
    * @param:
    * @return: candy list
    */
    std::vector<available_candy_list_t> getavailablecandylist();

    /*
    * Get asset info
    * @param:strAssetId, required
    * @return:asset info
    */
    assetdata_t getassetinfo(const std::string& strAssetId);

    /*
    * Query the transaction list by asset ID
    * @param:strAppId, required (Max. 20 bytes)
    * @param:nTxClass, required (1--All 2--Normal 3--Locked 4--Issue 5--Add Issue 6--Destorid)
    * @return:transaction ID
    */
    std::vector<std::string> getassetidtxids(const std::string& strAppId, const uint8_t& nTxClass);

    /*
    * Query the transaction list by asset ID and safe address
    * @param:strSafeAddress, required
    * @param:strAssetId, required (Max. 20 bytes)
    * @param:strTxClass, required (1--All 2--Normal 3--Locked 4--Issue 5--Add Issue 6--Destorid)
    * @return:transaction ID;
    */
    std::vector<std::string> getaddrassettxids(const std::string& strSafeAddress, const std::string& strAssetId, const uint8_t& strTxClass);

    /*
    * Query the transaction list by asset ID and safe address
    * @param:strSafeAddress, required
    * @param:strAssetId, required (Max. 20 bytes)
    * @return:1.receive amount 2.send amount 3.total amount 4.lock amount
    */
    asset_amount_t getaddrassetbalance(const std::string& strSafeAddress, const std::string& strAssetId);

    /*
    * Query details asset info by transaction ID
    * @param:strTxId, required
    * @return:transaction data could be (1.issue 2.add issue 3.transfer accounts 4.destory 5.put candy 6.get candy)
    */
    std::string getassetdetails(const std::string& strTxId);

    /*
    * Get all assets list
    * @param:
    * @return: asset list
    */
    std::vector<std::string> getassetlist();

    /*
    * Return the asset list according to the safe address
    * @param:strSafeAddress, required
    * @return:asset list
    */
    std::vector<std::string> getassetlistbyaddress(const std::string& strSafeAddress);

    /*
    * Get SAFE amount for the specified height and address
    * @param:nHeight, required
    * @param:strSafeAddress, required
    * @return:amount(The SAFE amount of the address at the current height)
              total amout(Ssum of all SAFE addresses at the current height)
    */
    double getaddressamountbyheight(const int& nHeight, const std::string& strSafeAddress, double & total_amount);

    /*
    * Get all candy height
    * @param:
    * @return: height list
    */
    std::vector<int> getallcandyheight();

    /*
    * Get the candy records of the address collection
    * @param:strAssetId, required
    * @param:strSafeAddress, optional
    * @return: candy block time
               details(safe address and candy amount)
    */
    address_candy_list_reponse_t getaddresscandylist(const std::string& strAssetId, const std::string& strSafeAddress = "");

    /*
    * Query local asset information
    * @param:strAssetId, required
    * @return: asset information
    */
    local_asset_amount_t getlocalassetinfo(const std::string& strAssetId);

    /*
    * Query the local asset list
    * @param:
    * @return: asset list
    */
    std::vector<std::string> getlocalassetlist();


};

#endif