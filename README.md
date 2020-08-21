# SX `Wallet` 👛 - EOSIO Smart Contract

> Deposit/withdraw funds to SX contracts

## Table of Content

- [Examples](#examples)
- [Build](#build)
- [ACTION `withdraw`](#action-withdraw)
- [ACTION `transfer`](#action-transfer)

## Examples

```bash
cleos transfer myaccount wallet.sx "1.0000 EOS" "myaccount"
cleos push action wallet.sx withdraw '["myaccount", "eosio.token", "1.0000 EOS"]' -p myaccount
```

## ACTION `withdraw`

Request to withdraw quantity

- **authority**: `owner` or `self`

### params

- `{name} owner` - owner of wallet assets
- `{name} contract` - token contract (ex: "eosio.token")
- `{asset} quantity` - withdraw quantity amount (ex: "1.0000 EOS")

### Example - cleos

```bash
cleos push action wallet.sx withdraw '["myaccount", "eosio.token", "1.0000 EOS"]' -p myaccount
```

### Example - smart contract

```c++
// input variables
const name owner = "myaccount"_n;
const name contract = "eosio.token"_n;
const asset quantity = asset{ 10000, symbol{"EOS", 4} };

// send transaction
wallet::withdraw_action withdraw( "wallet.sx"_n, { owner, "active"_n });
withdraw.send( owner, contract, quantity );
```
