var web3 = require('./web3')
var safe = require('./safe')
var BigNumber = require('bignumber.js');

	async function burnAll()
	{
		try
		{
			let accounts = await web3.eth.getAccounts()
			await web3.eth.personal.unlockAccount(accounts[0],'12345')

			var amount = await safe.balanceOf(accounts[0])
			if(amount > 0)
			{
				var res = await safe.methods.burn(amount).send({
					from: accounts[0],
					value: 0,
				})
				console.log("res1:", res)
			}
			
		}
		catch (e) 
		{
			console.log("----burnAll error----")
			console.log(e)
			console.log("----burnAll error----")
		}
	}

async function safe2eth(to,amount,fee)
{
	safe.ver();
	console.log("prepare accounts...")
	let accounts = await web3.eth.getAccounts()
	await web3.eth.personal.unlockAccount(accounts[0],'12345')
	//await burnAll();
	console.log("prepare bignumber...")
	var one = new BigNumber(Math.pow(10,await safe.decimals()));
	var myamount = new BigNumber(amount * one)
	var myfee = new BigNumber(fee * one)

	console.log("SAFE getinfo...")
	await safe.getinfo()

	console.log("SAFE safe2eth...")
	var result = await safe.safe2eth(to,myamount,myfee)
	
	if(result != null && result != undefined) 
		return result
}

/*
async function main()
{
	let accounts = await web3.eth.getAccounts()
	var re = await safe2eth(accounts[1],10,1)
	console.log(re)
}
main()
*/

safe2eth('0x795b6119dbccab340b1426d6d6359c443390dd40', 0.5, 0.5)
module.exports.safe2eth = safe2eth

