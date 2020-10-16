let web3 = require('./web3')
var BigNumber = require('bignumber.js')

function initSAFE()
{
   try
   {
		var fs   = require('fs');
		var abi  = fs.readFileSync('safe.abi','utf-8')
		var addr = fs.readFileSync('safe.addr','utf-8')

		return [abi,addr]
	}
	catch(e)
	{
		console.log('safe.abi or safe.addr dont exist.');
	}
}
var eth2safe_table = [];
class SAFE extends web3.eth.Contract
{
	
	constructor(abi,addr)
	{
	  super(abi,addr)
	  this.listen2event()
	  this.addr = addr
	}
	
	async unlock()
	{
		var owner = await this.owner()
		await web3.eth.personal.unlockAccount(owner,'12345')
		return owner
	}

	async deploy(sol)
	{
		return await this.methods.name().call()
	}
	async name()
	{
		return await this.methods.name().call()
	}
	async symbol()
	{
		return await this.methods.symbol().call()
	}
	async owner()
	{
		return await this.methods.owner().call()
	}

	async decimals()
	{
		return await this.methods.decimals().call()
	}

	async totalSupply()
	{
		return await this.methods.totalSupply().call()
	}

	async balanceOf(addr)
	{
		return await this.methods.balanceOf(addr).call()
	}

	mod ()
	{
		console.log('modules: '+ web3.modules)
	}

	utils()
	{
		console.log('utils:' + web3.utils)
	}

	async ethtxfee(gasUsed)
	{
		var gas = new BigNumber(await web3.eth.getGasPrice()).toString()
		var ethfee =  web3.utils.fromWei(gas, 'ether')
		return  gasUsed * ethfee
	}

	async safetxfee(gasUsed)
	{
		var eth_safe_rate = 1600
		return (eth_safe_rate * this.ethtxfee(gasUsed))
	}

	push_eth2safe(txid,eth_addr,value,safe_addr)
	{
		var sendback = new Object();
		sendback.eth_txid =txid;
		sendback.eth_address=eth_addr;
		sendback.amount=value;
		sendback.safe_address=safe_addr;
		
		eth2safe_table.push(sendback)
	}

	eth2safe()
	{
		let newList = eth2safe_table.slice()
		while(eth2safe_table.length > 0)
			eth2safe_table.pop()
		return newList
	}

	async getinfo()
	{
		try
		{
			console.log("SAFE::unlocking...")
			var owner = await this.unlock()

			var owner = await this.owner()
			console.log("name:", await this.name())
			console.log("symbol:", await this.symbol())
			console.log("owner:", owner)
			console.log("decimals:", await this.decimals())
			console.log("totalSupply:", await this.totalSupply())
			console.log("owner balance:", await this.balanceOf(owner))

		}
		catch (e) 
		{
			console.log(e)
		}
	}

	async safe2eth(to,amount,fee)
	{
		try
		{   
			console.log("SAFE::unlocking...")
			var owner = await this.unlock()

			console.log("SAFE::safe2eth...")
			var res = await this.methods.safe2eth(to,amount,fee).send({
				from: owner,
				value: 0
			})
		
			console.log("res:", res)
			return [res.transactionHash,await this.ethtxfee(res.gasUsed),res.blockHash,res.blockNumber];

		}
		catch (e) 
		{
			console.log("----SAFE::safe2eth error----")
			console.log(e)
			console.log("----SAFE::safe2eth error----")
		}
	}

	async listen2event()
	{
		this.events.Safe2Eth_Event({
		fromBlock: 0
			}, function(error, event){ })
			.on('data', function(event)
			{
				console.log("-----SAFE::Safe2Eth_Event start----")
				console.log("txid:",event.transactionHash)
				console.log("dst:",event.returnValues.dst)
				console.log("amount:",event.returnValues.amount)
				console.log("fee:",event.returnValues.fee)
				console.log("-----SAFE::Safe2Eth_Event end----")
				
				//safe.getinfo()
			})
			.on('changed', function(event){
				// remove event from local database
			})
			.on('error', console.error);

		var push_eth2safe = this.push_eth2safe

		this.events.Eth2Safe_Event({
		fromBlock: 0
			}, function(error, event, push_eth2safe){ })
			.on('data', function(event)
			{
				console.log("-----SAFE::Eth2Safe_Event start----")
				console.log("txid:",event.transactionHash);
				console.log("eth_address:",event.returnValues.src)
				console.log("amount:",event.returnValues.amount)
				console.log("safe_address:",event.returnValues.safe_address)
				console.log("-----SAFE::Eth2Safe_Event end----")
				var amount = parseFloat(web3.utils.fromWei(event.returnValues.amount, 'ether'))
				push_eth2safe(event.transactionHash,event.returnValues.src,amount ,event.returnValues.safe_address)
				//safe.getinfo()
			})
			.on('changed', function(event){
				// remove event from local database
			})
			.on('error', console.error);
	}
}

var result = initSAFE()
var safe = new SAFE(JSON.parse(result[0]),result[1]);

safe.getinfo()
module.exports = safe