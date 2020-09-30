var web3 = require('./web3')
var fs = require('fs');

var abi = JSON.parse(fs.readFileSync("safe.abi"));
var bytecode = fs.readFileSync('safe.bytecode');