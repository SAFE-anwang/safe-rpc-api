var web3 = require('./web3')
var safe = require('./safe')
var jayson = require('jayson')
var BigNumber = require('bignumber.js');
const jsonParser = require('body-parser').json;
const connect = require('connect');
const app = connect();

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

async function safe2eth(to,amount,fee)
{
	var one = new BigNumber(Math.pow(10,18));
	var myamount = new BigNumber(amount * one)
	var myfee = new BigNumber(fee * one)

	return await safe.safe2eth(to,myamount,myfee)
}


// create a server
var server = jayson.server({
  safe2eth: async function (args,callback)
  {
    console.log("safe2eth incoming request: ",args)
	callback(null,await safe2eth(args[0],args[1],args[2]))
  },
  eth2safe: async function (args,callback)
  {
    console.log("eth2safe incoming request ")
	callback(null,safe.eth2safe())
  },
  getbalance: async function (args,callback)
  {
	console.log("getbalance incoming request ")
    var ret = new Object();
	ret.amount = parseFloat(web3.utils.fromWei(await safe.totalSupply(), 'ether'))
	console.log(ret)
	callback(null,ret)
  }
})

app.use(jsonParser());
app.use(server.middleware());
app.listen(50505)