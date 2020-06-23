
#ifndef SAFE_API_TYPES_H
#define SAFE_API_TYPES_H

#include <string>
#include <vector>

#include <json/json.h>

/* === APP types === */

struct apptxids_t   //used by getapptxids
{ 
	std::string strAppId;
    int nAppOperType;
    int nSetType;
};

struct address_apptxids_t : apptxids_t   //used by getaddressapptxids
{
	std::string strSafeAddress;
};

/*
    * @param:strAppName,required (Max. 50 bytes)
    * @param:strAppDesc,required (Max. 600 bytes)
    * @param:nDevType,required (1--company 2--individual)
    * @param:strDevName,required (Max. 100 bytes, If company, fill company name, If individual, fill individual name)
    * @param:strWebUrl,optional (Max. 300 bytes)
    * @param:strAppLogoUrl,optional (Max. 300 bytes)
    * @param:strAppCoverUrl,optional (Max. 300 bytes)
	*/

struct appdata_t   //returned by getappdetails , used by getappdetails
{
	std::string     strAdminAddress;
	std::string     strAppName;
    std::string     strAppDesc;
    int             nDevType;
    std::string     strDevName;
    std::string     strWebUrl;
    std::string     strLogoUrl;
    std::string     strCoverUrl;
};

struct registerapp_response_t   //returned by registerapp
{
	std::string adminAddress;
	std::string appId;//app ID(start at 1001)
	std::string txId;
};

/* === AUTH types === */

struct authdata_response_t  //returned by getappdetails
{
    int             nSetType;
	std::string     appId;
    std::string     userSafeAddress;
    int             appAuthCmd;
};

struct authdata_t  //used by setappauth
{
    int             nSetType;
	std::string     strAdminAddress;
    std::string     strUserAddress;
    int             nAuth;
};

struct extenddata_response_t  //returned by getappdetails
{
     std::string strUserSafeAddress;
     std::string strAppId;
     int		 dAppAuthCmd;
     std::string strExtendData;
}

struct extenddata_t : extenddata_response_t  //used by createextenddatatx
{
     int		 nAppTxType;
}

struct extenddata_list_response_t  //returned by getextenddata
{
	std::string strAppId;
	std::string strExtendData;
}


/* === ASSET types === */

struct asset_amount_t						//used by getaddrassetbalance
{
    double     ReceiveAmount;
    double     SendAmount;
    double     totalAmount;
    double     lockAmount;
}

struct asset_base_t						//used by addissueasset , destroyasset
{
    std::string     strAssetId;
    double          dAmount;
    std::string     strRemarks;
}

struct asset_transfer_t : asset_base_t   //used by transferasset
{
    std::string     strSafeAddress;
    int		        nLockTime;
}

struct assetdata_t       //used by issueasset, getassetinfo
{
    std::string     strShortName;
    std::string     strAssetName;
    std::string     strAssetDesc;
    std::string     strAssetUnit;
    double          dTotalAmount;
    double			dFirstIssueAmount;
    double			dFirstActualAmount;
	double          dAlreadyIssueAmount;
    int				nDecimals;
    bool            bDestory;
    bool            bPayCandy;
    double			dCandyAmount;
    int				nCandyExpired;
	double			dDestoryTotalAmout;
	long			issueTime;
    std::string     strRemarks;
	std::string     adminSafeAddress;
};

struct assetdata_response_t		  //returned by issueasset
{
    std::string     strAssetId;
    std::string     strTxId;
}

struct local_asset_amount_t       //used by getlocalassetinfo
{
    std::string     assetShortName;
    std::string     strAssetName;
    std::string     assetDesc;
    std::string     assetUnit;
    int				assetDecimals;
	long			issueTime;
    double			assetAvailAmount;
    double			assetWaitAmount;
	double			assetLockAmount;
	double			assetLocalTotalAmount;
};

/* === CANDY types === */

struct candy_info_t   //used by putcandy
{
    std::string		strAssetId;
    double			dAmount;
	int				nExpired;
    std::string     strRemarks;
}

struct available_candy_list_t  //returned by getavailablecandylist;
{
	long			putTime;
	std::string		assetId;
    double			assetCandyAmount;
	int				candyExpired;
}

struct address_candy_amount_t  //getaddresscandylist
{
	std::string		safeAddress;
	double			candyAmount;
}

struct address_candy_list_reponse_t   //returned by getaddresscandylist
{
	long			candyBlockTime;
	std::vector<address_candy_amount_t> details;
}

#endif
