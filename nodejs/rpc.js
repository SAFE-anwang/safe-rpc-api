var web3 = require('./web3')
var safe = require('./safe')
var jayson = require('jayson')
var send = require('./send')

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

// create a server
var server = jayson.server({
  safe2eth: async function (args,callback)
  {
    console.log("coming request: ",args)
	callback(null,await send.safe2eth(args[0],args[1],args[2]))
  }
})

app.use(jsonParser());
app.use(server.middleware());
app.listen(50505)

