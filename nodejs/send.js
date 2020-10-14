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
	return await safe.safe2eth(to,myamount,myfee)
}

function eth2safe()
{
	console.log("send::eth2safe...")
	return safe.eth2safe()
}
//var res = safe2eth('0x795b6119dbccab340b1426d6d6359c443390dd40', 1, 0.1).then(console.log)
safe.push_eth2safe(1,2,3,4)
safe.push_eth2safe(5,6,7,8)
safe.push_eth2safe(9,10,11,12)
//var res = eth2safe()
//console.log(res)

module.exports.safe2eth = safe2eth
module.exports.eth2safe = eth2safe
