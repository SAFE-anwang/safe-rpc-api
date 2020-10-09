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
	//safe.ver();
	let accounts = await web3.eth.getAccounts()
	await web3.eth.personal.unlockAccount(accounts[0],'12345')
	//await burnAll();
	
	var one = new BigNumber(Math.pow(10,await safe.decimals()));

	var myamount = new BigNumber(amount * one)
	var myfee = new BigNumber(fee * one)

	await safe.getinfo()
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

module.exports.safe2eth = safe2eth

