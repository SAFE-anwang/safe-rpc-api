//导入solc编译器
var solc = require('solc')
var web3 = require('./web3')
//var fs = require('fs');

function compile(sol)
{
	//读取合约
	let fs = require('fs')
	let sourceCode = fs.readFileSync(sol)

	let output = solc.compile(sourceCode, 1)
	console.log('output:', output)

	console.log('abi:______',output['contracts'][':SimpleStorage']['interface'])

	//导出合约
	return output['contracts'][':SimpleStorage']
}

async function deploy(bytecode, abi)
{

	var address = ''

		try
		{
			//此地址需要使用地址
			const account = await web3.eth.getAccounts()

			//1.拼接合约数据interface
			let contract = new web3.eth.Contract(JSON.parse(abi))
			//2.拼接bytecode
			contract.deploy({
			    data: bytecode,//合约的bytecode
			    arguments: []//给构造函数传递参数，使用数组
			}).send({
			    from:account,
			    gas:'3000000',
			    gasPrice:'1',
			}).then(instance =>{
			    console.log('address:',instance.options.address)
			    address = instance.options.address
			})
			
			return address
		}
		catch (e) 
		{
			console.log("----deploy error----")
			console.log(e)
			console.log("----deploy error----")
		}
}

module.exports.deploy  = deploy
module.exports.compile = compile