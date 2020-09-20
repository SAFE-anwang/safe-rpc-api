#include <string>
#include <stdexcept>
#include <cmath>

#include <jsonrpccpp/client.h>
#include <jsonrpccpp/client/connectors/httpclient.h>

#include "safeapi.h"

using jsonrpc::Client;
using jsonrpc::JSONRPC_CLIENT_V1;

using jsonrpc::HttpClient;
using jsonrpc::JsonRpcException;

using Json::Value;
using Json::ValueIterator;

using std::map;
using std::string;
using std::vector;

safe_decoderawtransaction_t SafeAPI::safe_decoderawtransaction(const string& hexString) {
	string command = "decoderawtransaction";
	Value params, result;
	safe_decoderawtransaction_t ret;

	params.append(hexString);
	result = sendcommand(command, params);

	ret.txid = result["txid"].asString();
	ret.version = result["version"].asInt();
	ret.locktime = result["locktime"].asInt();
	for (ValueIterator it = result["vin"].begin(); it != result["vin"].end(); it++)
	{
		Value val = (*it);
		vin_t input;
		input.txid = val["txid"].asString();
		input.n = val["vout"].asUInt();
		input.scriptSig.assm = val["scriptSig"]["asm"].asString();
		input.scriptSig.hex = val["scriptSig"]["hex"].asString();
		input.sequence = val["sequence"].asUInt();
		ret.vin.push_back(input);
	}

	for (ValueIterator it = result["vout"].begin(); it != result["vout"].end(); it++)
	{
		Value val = (*it);
		safe_vout_t output;

		output.value = val["value"].asDouble();
		output.n = val["n"].asUInt();

		//safe
		output.txType = val["txType"].asUInt();
		output.nUnlockedHeight = val["nUnlockedHeight"].asUInt();
		output.reserve = val["reserve"].asString();

		output.scriptPubKey.assm = val["scriptPubKey"]["asm"].asString();
		output.scriptPubKey.hex = val["scriptPubKey"]["hex"].asString();
		output.scriptPubKey.reqSigs = val["scriptPubKey"]["reqSigs"].asInt();

		output.scriptPubKey.type = val["scriptPubKey"]["type"].asString();
		for (ValueIterator it2 = val["scriptPubKey"]["addresses"].begin(); it2 != val["scriptPubKey"]["addresses"].end(); it2++) {
			output.scriptPubKey.addresses.push_back((*it2).asString());
		}

		ret.vout.push_back(output);
	}

	return ret;
}

/* === Raw transaction calls === */
safe_getrawtransaction_t SafeAPI::safe_getrawtransaction(const std::string& txid, int verbose)
{
	string command = "getrawtransaction";
	Value params, result;
	safe_getrawtransaction_t ret;

	params.append(txid);
	params.append(verbose);
	result = sendcommand(command, params);

	ret.hex = ((verbose == 0) ? result.asString() : result["hex"].asString());

	if (verbose != 0) 
	{
		ret.txid = result["txid"].asString();
		ret.version = result["version"].asInt();
		ret.locktime = result["locktime"].asInt();
		for (ValueIterator it = result["vin"].begin(); it != result["vin"].end();it++) 
		{
			Value val = (*it);
			vin_t input;
			input.txid = val["txid"].asString();
			input.n = val["vout"].asUInt();
			input.scriptSig.assm = val["scriptSig"]["asm"].asString();
			input.scriptSig.hex = val["scriptSig"]["hex"].asString();
			input.sequence = val["sequence"].asUInt();
			ret.vin.push_back(input);
		}

		for (ValueIterator it = result["vout"].begin(); it != result["vout"].end();it++) 
		{
			Value val = (*it);
			safe_vout_t output;

			output.value = std::stod(val["value"].asString());
			output.n = val["n"].asUInt();

			//safe
			output.txType = val["txType"].asUInt();
			output.nUnlockedHeight = val["nUnlockedHeight"].asUInt();
			output.reserve = val["reserve"].asString();

			output.scriptPubKey.assm = val["scriptPubKey"]["asm"].asString();
			output.scriptPubKey.hex = val["scriptPubKey"]["hex"].asString();
			output.scriptPubKey.reqSigs = val["scriptPubKey"]["reqSigs"].asInt();

			output.scriptPubKey.type = val["scriptPubKey"]["type"].asString();
			for (ValueIterator it2 = val["scriptPubKey"]["addresses"].begin(); it2 != val["scriptPubKey"]["addresses"].end(); it2++) 
			{
				output.scriptPubKey.addresses.push_back((*it2).asString());
			}

			ret.vout.push_back(output);
		}
		ret.blockhash = result["blockhash"].asString();
		ret.confirmations = result["confirmations"].asUInt();
		ret.time = result["time"].asUInt();
		ret.blocktime = result["blocktime"].asUInt();
	}

	return ret;
}

int SafeAPI::getmasternodecount()
{
	string command = "masternode";
	Value params, result;
	params.append("count");
	result = sendcommand(command, params);

	return result.asInt();
}

std::string SafeAPI::getmasternodelist()
{
	string command = "masternode";
	Value params, result;
	params.append("list");
	result = sendcommand(command, params);

	return result.asString();
}

registerapp_response_t SafeAPI::registerapp(appdata_t & app)
{
	string command = "registerapp";
	Value params, result;

	params.append(app.strAppName);
	params.append(app.strAppDesc);
	params.append(app.nDevType);
	params.append(app.strDevName);
	if(app.strWebUrl != "") params.append(app.strWebUrl);
	if(app.strLogoUrl != "") params.append(app.strLogoUrl);
	if(app.strCoverUrl != "") params.append(app.strCoverUrl);

	result = sendcommand(command, params);
	
	registerapp_response_t reg;
	reg.adminAddress = result["adminSafeAddress"].asString();
	reg.txId = result["txId"].asString();
	reg.appId = result["appId"].asString();
	
    return reg;
}

std::string SafeAPI::setappauth(authdata_t & auth)
{
	string command = "setappauth";
	Value params, result;

	params.append(auth.nSetType);
	params.append(auth.strAdminAddress);
	params.append(auth.strUserAddress);
	params.append(auth.nAuth);
	result = sendcommand(command, params);

	return result.asString();
}

std::string SafeAPI::createextenddatatx(extenddata_t & ed)
{
    string command = "createextenddatatx";
	Value params, result;

	params.append(ed.nAppTxType);
	params.append(ed.strUserSafeAddress);
	params.append(ed.strAppId);
	params.append(ed.dAppAuthCmd);
	params.append(ed.strExtendData);
	result = sendcommand(command, params);

    return result.asString();
}

std::vector<extenddata_list_response_t> SafeAPI::getextenddata(const std::string& strTxId)
{
    string command = "getextenddata";
	Value params, result;
	std::vector<extenddata_list_response_t> ver;

	params.append(strTxId);
	result = sendcommand(command, params);

	for(ValueIterator it = result["extendDataList"].begin(); it != result["extendDataList"].end(); it++)
	{
		extenddata_list_response_t er;
		er.strAppId = (*it)["appId"].asString();
		er.strExtendData = (*it)["appData"].asString();
		ver.push_back(er);
	}

    return ver;
}

std::vector<std::string> SafeAPI::getapptxids(apptxids_t & apptxid)
{
	string command = "getapptxids";
	Value params, result;
	std::vector<std::string> txs;

	params.append(apptxid.strAppId);
	params.append(apptxid.nAppOperType);
	params.append(apptxid.nSetType);
	result = sendcommand(command, params);

	for(ValueIterator it = result["txList"].begin(); it != result["txList"].end(); it++)
	{
		txs.push_back((*it).asString());
	}

    return txs;
}

std::vector<std::string> SafeAPI::getaddressapptxids(address_apptxids_t & aat)
{
	string command = "getaddressapptxids";
	Value params, result;
	std::vector<std::string> txs;
	
	params.append(aat.strSafeAddress);
	params.append(aat.strAppId);
	params.append(aat.nAppOperType);
	params.append(aat.nSetType);
	result = sendcommand(command, params);

	for(ValueIterator it = result["txList"].begin(); it != result["txList"].end(); it++)
	{
		txs.push_back((*it).asString());
	}

    return txs;
}

std::vector<std::string> SafeAPI::getapplist()
{
	string command = "getapplist";
	Value params, result;
	std::vector<std::string> apps;
	
	result = sendcommand(command, params);

	for(ValueIterator it = result["appList"].begin(); it != result["appList"].end(); it++)
	{
		apps.push_back((*it).asString());
	}

    return apps;
}

appdata_t SafeAPI::getappinfo(const std::string& strAppId)
{
	string command = "getappinfo";
	Value params, result;
	appdata_t appData;
	
	params.append(strAppId);
	result = sendcommand(command, params);

	appData.strAppName = result["appName"].asString();
	appData.strAppDesc = result["appDesc"].asString();
	appData.nDevType = result["devType"].asInt();
	appData.strDevName = result["devName"].asString();
	appData.strWebUrl = result["webUrl"].asString();
	appData.strLogoUrl = result["appLogoUrl"].asString();
	appData.strCoverUrl = result["appCoverUrl"].asString();
	appData.strAdminAddress = result["adminSafeAddress"].asString();

    return appData;
}

std::vector<std::string> SafeAPI::getapplistbyaddress(const std::string& strAppAddress)
{
	string command = "getapplistbyaddress";
	Value params, result;
	std::vector<std::string> apps;
	
	params.append(strAppAddress);
	result = sendcommand(command, params);

	for(ValueIterator it = result["appList"].begin(); it != result["appList"].end(); it++)
	{
		apps.push_back((*it).asString());
	}

    return apps;
}

bool SafeAPI::getappdetails(const std::string& strTxId, appdata_t & appData, authdata_response_t & ar, extenddata_response_t & er)
{
    string command = "getappdetails";
	Value params, result;
	
	params.append(strTxId);
	result = sendcommand(command, params);

	for(ValueIterator it = result["txData"].begin(); it != result["txData"].end(); it++)
	{
		Value & appinfo = (*it);
		Value obj = appinfo[0];

		appData.strAppName = obj["appName"].asString();
		appData.strAppDesc = obj["appDesc"].asString();
		appData.nDevType = obj["devType"].asInt();
		appData.strDevName = obj["devName"].asString();
		appData.strWebUrl = obj["webUrl"].asString();
		appData.strLogoUrl = obj["appLogoUrl"].asString();
		appData.strCoverUrl = obj["appCoverUrl"].asString();
		appData.strAdminAddress = obj["adminSafeAddress"].asString();

		Value obj2 = appinfo[1];
		ar.nSetType = obj2["setType"].asInt();
		ar.appId = obj2["appId"].asString();
		ar.userSafeAddress = obj2["userSafeAddress"].asString();
		ar.appAuthCmd = obj2["appAuthCmd"].asInt();

		Value obj3 = appinfo[2];
		er.strUserSafeAddress = obj3["userSafeAddress"].asString();
		er.strAppId = obj3["appId"].asString();
		er.dAppAuthCmd = obj3["appAuthCmd"].asInt();
		er.strExtendData = obj3["extendData"].asString();
	}

    return true;
}

bool SafeAPI::getauthlist(const std::string& strAppId, const std::string& strAddress, std::vector<int> & all_user,std::vector<int> & current_user)
{
    string command = "getauthlist";
	Value params, result;
	
	params.append(strAppId);
	params.append(strAddress);
	result = sendcommand(command, params);

    for(ValueIterator it = result["ALL_USER"].begin(); it != result["ALL_USER"].end(); it++)
	{
		all_user.push_back((*it).asInt());
	}

    for(ValueIterator it = result["CURRENT_USER"].begin(); it != result["CURRENT_USER"].end(); it++)
	{
		all_user.push_back((*it).asInt());
	}

    return true;
}

assetdata_response_t SafeAPI::issueasset(assetdata_t & ad)
{
    string command = "issueasset";
	Value params, result;
	assetdata_response_t ar;
	
	params.append(ad.strShortName);
	params.append(ad.strAssetName);
	params.append(ad.strAssetDesc);
	params.append(ad.strAssetUnit);
	params.append(ad.dTotalAmount);
	params.append(ad.dFirstIssueAmount);
	params.append(ad.dFirstActualAmount);
	params.append(ad.nDecimals);
	params.append(ad.bDestory);
	params.append(ad.bPayCandy);
	params.append(ad.dCandyAmount);
	params.append(ad.nCandyExpired);
	params.append(ad.strRemarks);
	
	result = sendcommand(command, params);

	ar.strAssetId = result["assetId"].asString();
	ar.strTxId =	result["txId"].asString();

    return ar;
}

std::string SafeAPI::addissueasset(const asset_base_t & ab)
{
    string command = "addissueasset";
	Value params, result;
	
	params.append(ab.strAssetId);
	params.append(ab.dAmount);
	params.append(ab.strRemarks);
	
	result = sendcommand(command, params);

    return result["txId"].asString();
}

std::string SafeAPI::transferasset(const asset_transfer_t & at)
{
    string command = "transferasset";
	Value params, result;
	
	params.append(at.strSafeAddress);
	params.append(at.strAssetId);
	params.append(at.dAmount);
	params.append(at.nLockTime);
	params.append(at.strRemarks);

	result = sendcommand(command, params);

    return result["txId"].asString();
}

std::string SafeAPI::destoryasset(const asset_transfer_t & at)
{
    string command = "destoryasset";
	Value params, result;
	
	params.append(at.strAssetId);
	params.append(at.dAmount);
	params.append(at.strRemarks);
	
	result = sendcommand(command, params);

    return result["txId"].asString();
}

std::string SafeAPI::putcandy(const candy_info_t & pc)
{
    string command = "putcandy";
	Value params, result;
	
	params.append(pc.strAssetId);
	params.append(pc.dAmount);
	params.append(pc.strRemarks);
	params.append(pc.nExpired);
	
	result = sendcommand(command, params);

    return result["txId"].asString();
}

std::map<std::string,double> SafeAPI::getcandy(const std::string& strAssetId)
{
    string command = "getcandy";
	Value params, result;
	std::map<std::string,double> candymap;
	
	params.append(strAssetId);
	result = sendcommand(command, params);

	for (int i = 0; i < (int)result.size(); i++)
	{
		candymap[result[i]["txId"].asString()] = result[i]["assetAmount"].asDouble();
	}

    return candymap;
}

std::vector<available_candy_list_t> SafeAPI::getavailablecandylist()
{
    string command = "getavailablecandylist";
	Value params, result;
	std::vector<available_candy_list_t> acl;
	
	result = sendcommand(command, params);

	for(ValueIterator it = result["candyList"].begin(); it != result["candyList"].end(); it++)
	{
		Value & candy = (*it);
		available_candy_list_t ac;

		ac.putTime = candy["putTime"].asUInt();
		ac.assetId = candy["assetId"].asString();
		ac.assetCandyAmount = candy["assetCandyAmount"].asDouble();
		ac.candyExpired = candy["candyExpired"].asInt();
		
		acl.push_back(ac);
	}
	
    return acl;
}

   
assetdata_t SafeAPI::getassetinfo(const std::string& strAssetId)
{
    string command = "getassetinfo";
	Value params, result;
	assetdata_t at;
	
	params.append(strAssetId);
	result = sendcommand(command, params);

	at.strShortName = result["assetShortName"].asString();
    at.strAssetName = result["assetName"].asString();
    at.strAssetDesc = result["assetDesc"].asString();
    at.strAssetUnit = result["assetUnit"].asString();
    at.dTotalAmount = result["assetTotalAmount"].asDouble();
    at.dFirstIssueAmount = result["firstIssueAmount"].asDouble();
    at.dFirstActualAmount = result["firstActualAmount"].asDouble();
	at.dAlreadyIssueAmount = result["alreadyIssueAmount"].asDouble();
    at.nDecimals = result["assetDecimals"].asInt();
    at.bDestory = result["isDestory"].asBool();
    at.bPayCandy = result["isPayCandy"].asBool();
    at.dCandyAmount = result["candyTotalAmount"].asDouble();
    at.nCandyExpired = result["destoryTotalAmout"].asInt();
	at.dDestoryTotalAmout = result["candyExpired"].asDouble();
	at.issueTime = result["remarks"].asUInt();
    at.strRemarks = result["issueTime"].asString();
	at.adminSafeAddress = result["adminSafeAddress"].asString();
    
    return at;
}

std::vector<std::string> SafeAPI::getassetidtxids(const std::string& strAppId, const uint8_t & nTxClass)
{
    string command = "getassetidtxids";
	Value params, result;
	std::vector<std::string> txs;
	
	params.append(strAppId);
	params.append(nTxClass);
	result = sendcommand(command, params);

	for(ValueIterator it = result["txList"].begin(); it != result["txList"].end(); it++)
	{
		Value & txid = (*it);
		txs.push_back(txid["txId"].asString());
	}

    return txs;
}

std::vector<std::string> SafeAPI::getaddrassettxids(const std::string& strSafeAddress, const std::string& strAssetId, const uint8_t& nTxClass)
{
    string command = "getaddrassettxids";
	Value params, result;
	std::vector<std::string> txs;
	
	params.append(strSafeAddress);
	params.append(strAssetId);
	params.append(nTxClass);
	result = sendcommand(command, params);

	for(ValueIterator it = result["txList"].begin(); it != result["txList"].end(); it++)
	{
		Value & txid = (*it);
		txs.push_back(txid["txId"].asString());
	}

    return txs;
}

asset_amount_t SafeAPI::getaddrassetbalance(const std::string& strSafeAddress, const std::string& strAssetId)
{
    string command = "getaddrassettxids";
	Value params, result;
	asset_amount_t am;
	
	params.append(strSafeAddress);
	params.append(strAssetId);
	result = sendcommand(command, params);

	am.ReceiveAmount = result["ReceiveAmount"].asDouble();
	am.SendAmount = result["SendAmount"].asDouble();
	am.totalAmount = result["totalAmount"].asDouble();
	am.lockAmount = result["lockAmount"].asDouble();

    return am;
}
//to do
std::string SafeAPI::getassetdetails(const std::string& strTxId)
{
    string command = "getaddrassettxids";
	Value params, result;
	
	params.append(strTxId);
	result = sendcommand(command, params);

    return result["txData"].asString();
}

std::vector<std::string> SafeAPI::getassetlist()
{
    string command = "getassetlist";
	Value params, result;
	std::vector<std::string> al;
	
	result = sendcommand(command, params);

	for(ValueIterator it = result["assetList"].begin(); it != result["assetList"].end(); it++)
	{
		Value & txid = (*it);
		al.push_back(txid["assetId"].asString());
	}

    return al;
}

std::vector<std::string> SafeAPI::getassetlistbyaddress(const std::string& strSafeAddress)
{
    string command = "getassetlistbyaddress";
	Value params, result;
	std::vector<std::string> al;

	params.append(strSafeAddress);
	result = sendcommand(command, params);

	for(ValueIterator it = result["assetList"].begin(); it != result["assetList"].end(); it++)
	{
		Value & txid = (*it);
		al.push_back(txid["assetId"].asString());
	}

    return al;
}

double SafeAPI::getaddressamountbyheight(const int& nHeight, const std::string& strSafeAddress, double & total_amount)
{
    string command = "getaddressamountbyheight";
	Value params, result;

	params.append(nHeight);
	params.append(strSafeAddress);
	result = sendcommand(command, params);

	total_amount =  result["totalAmount"].asDouble();

    return result["amount"].asDouble();
}

std::vector<int> SafeAPI::getallcandyheight()
{
    string command = "getallcandyheight";
	Value params, result;
	std::vector<int> hl;

	result = sendcommand(command, params);

	for(ValueIterator it = result["heightList"].begin(); it != result["heightList"].end(); it++)
	{
		Value & v = (*it);
		hl.push_back(v.asInt());
	}

    return hl;
}

address_candy_list_reponse_t SafeAPI::getaddresscandylist(const std::string& strAssetId, const std::string& strSafeAddress)
{
	string command = "getaddresscandylist";
	Value params, result;
	address_candy_list_reponse_t acl;

	params.append(strAssetId);
	params.append(strSafeAddress);

	result = sendcommand(command, params);

	acl.candyBlockTime = result["candyBlockTime"].asUInt();
	for(ValueIterator it = result["details"].begin(); it != result["details"].end(); it++)
	{
		Value & v = (*it);
		address_candy_amount_t aca;
		aca.safeAddress = v["safeAddress"].asString();
		aca.candyAmount = v["candyAmount"].asDouble();

		acl.details.push_back(aca);
	}
    return acl;
}

local_asset_amount_t SafeAPI::getlocalassetinfo(const std::string& strAssetId)
{
	string command = "getlocalassetinfo";
	Value params, result;
	local_asset_amount_t laa;

	params.append(strAssetId);

	result = sendcommand(command, params);

	laa.assetShortName = result["assetShortName"].asString();
	laa.strAssetName = result["strAssetName"].asString();
	laa.assetDesc = result["assetDesc"].asString();
	laa.assetUnit = result["assetUnit"].asString();

	laa.assetDecimals = result["assetDecimals"].asInt();
	laa.issueTime = result["issueTime"].asUInt();

	laa.assetAvailAmount = result["assetAvailAmount"].asDouble();
	laa.assetWaitAmount = result["assetWaitAmount"].asDouble();
	laa.assetLockAmount = result["assetLockAmount"].asDouble();
	laa.assetLocalTotalAmount = result["assetLocalTotalAmount"].asDouble();

	return laa;
}

std::vector<std::string> SafeAPI::getlocalassetlist()
{
    string command = "getlocalassetlist";
	Value params, result;
	std::vector<std::string> al;
	
	result = sendcommand(command, params);

	for(ValueIterator it = result["assetList"].begin(); it != result["assetList"].end(); it++)
	{
		Value & txid = (*it);
		al.push_back(txid["assetId"].asString());
	}

    return al;
}

