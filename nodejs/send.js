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
	var one = new BigNumber(Math.pow(10,18));
	var myamount = new BigNumber(amount * one)
	var myfee = new BigNumber(fee * one)

	console.log("send::safe2eth...")
	var result = await safe.safe2eth(to,myamount,myfee)

	return result
}

//var res = safe2eth('0x795b6119dbccab340b1426d6d6359c443390dd40', 1, 0.1).then(console.log)
module.exports.safe2eth = safe2eth

