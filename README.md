# SX `Wallet` 🏦 - EOSIO Smart Contract

Deposit/withdraw assets from SX wallet contract

## Quickstart

```bash
# default deposit
cleos transfer myaccount wallet.sx "1.0000 EOS"

# deposit to specific account
cleos transfer myaccount wallet.sx "1.0000 EOS" "toaccount"

# withdraw
cleos push action wallet.sx withdraw '["myaccount", "eosio.token", "1.0000 EOS"]' -p myaccount

# transer
cleos push action wallet.sx transfer '["myaccount", "toaccount", "eosio.token", "1.0000 EOS", "my memo"]' -p myaccount
```

## Table of Content

- [TABLE `balances`](#table-balances)
- [ACTION `withdraw`](#action-withdraw)
- [ACTION `transfer`](#action-transfer)
- [STATIC `get_balance`](#static-get_balance)

## TABLE `balances`

**scope:** `account`

- `{name} contract` - token contract
- `{map<symbol_code, asset>} balances` - balances

### Example - cleos

```bash
$ cleos get table wallet.sx myaccount balances
```

### Example - json

```json
{
    "contract": "eosio.token",
    "balances": [
        { "key": "EOS", "value": "1.0000 EOS" }
    ]
}
```

## ACTION `withdraw`

Request to withdraw quantity

- **authority**: `from` or `get_self()`

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

## ACTION `transfer`

Transfer assets to an account

- **authority**: `from` or `get_self()`

### params

- `{name} from` - authorized sender account
- `{name} to` - receiver account
- `{name} contract` - token contract (ex: "eosio.token")
- `{asset} quantity` - transfer quantity amount (ex: "1.0000 EOS")
- `{string} [memo=""]` - memo used on transfer

### Example - cleos

```bash
cleos push action wallet.sx transfer '["myaccount", "toaccount", "eosio.token", "1.0000 EOS", "memo"]' -p myaccount
```

### Example - smart contract

```c++
// input variables
const name from = "myaccount"_n;
const name to = "toaccount"_n;
const name contract = "eosio.token"_n;
const asset quantity = asset{ 10000, symbol{"EOS", 4} };
const string memo = "my memo";

// send transaction
walletSx::transfer_action transfer( "wallet.sx"_n, { from, "active"_n });
transfer.send( from, to, contract, quantity, memo );
```

## STATIC `get_balance`

Get balance of account

### params

- `{name} code` - SX wallet contract account
- `{name} account` - account name
- `{name} contract` - token contract
- `{symbol_code} symcode` - symbol code

### example

```c++
const name account = "myaccount"_n;
const name contract = "eosio.token"_n;
const symbol_code symcode = symbol_code{"EOS"};

const asset balance = walletSx::get_balance( "wallet.sx"_n, account, contract, symcode );
//=> "1.0000 EOS"
```