# SX `Wallet` 🏦 - EOSIO Smart Contract

Deposit/withdraw assets from SX wallet contract

## Quickstart

```bash
# default deposit
cleos transfer myaccount wallet.sx "1.0000 EOS"

# deposit to specific account
cleos push action wallet.sx open '["toaccount", "eosio.token", "EOS", "myaccount"]' -p myaccount
cleos transfer myaccount wallet.sx "1.0000 EOS" "toaccount"

# withdraw
cleos push action wallet.sx withdraw '["myaccount", "eosio.token", "1.0000 EOS"]' -p myaccount

# move
cleos push action wallet.sx move '["myaccount", "toaccount", "eosio.token", "1.0000 EOS", "my memo"]' -p myaccount
```

## Table of Content

- [TABLE `balances`](#table-balances)
- [ACTION `withdraw`](#action-withdraw)
- [ACTION `move`](#action-move)
- [ACTION `open`](#action-open)
- [ACTION `close`](#action-close)
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

## ACTION `move`

Move assets to an account

- **authority**: `from` or `get_self()`

### params

- `{name} from` - authorized sender account & funds to be deducted
- `{name} to` - receiver account
- `{name} contract` - token contract (ex: "eosio.token")
- `{asset} quantity` - transfer quantity amount (ex: "1.0000 EOS")
- `{string} [memo=""]` - memo used on move

### Example - cleos

```bash
cleos push action wallet.sx move '["myaccount", "toaccount", "eosio.token", "1.0000 EOS", "memo"]' -p myaccount
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
walletSx::move_action move( "wallet.sx"_n, { from, "active"_n });
move.send( from, to, contract, quantity, memo );
```

## ACTION `open`

Open contract & symbol balance for account

- **authority**: `ram_payer`

### params

- `{name} account` - account to open balance
- `{name} contract` - token contract (ex: "eosio.token")
- `{symbol_code} symcode` - symcode code (ex: "EOS")
- `{name} ram_payer` - authorized account to pay for RAM

### Example - cleos

```bash
cleos push action wallet.sx open '["myaccount", "eosio.token", "EOS", "myaccount"]' -p myaccount
```

### Example - smart contract

```c++
const name account = "myaccount"_n;
const name contract = "eosio.token"_n;
const symbol_code symcode = symbol_code{"EOS"};
const name ram_payer = "myaccount";

walletSx::open_action open( "wallet.sx"_n, { ram_payer, "active"_n });
open.send( account, contract, symcode, ram_payer );
```

## ACTION `close`

Close contract & symbol balance for account

- **authority**: `account`

### params

- `{name} account` - account to close balance
- `{name} contract` - token contract (ex: "eosio.token")
- `{symbol_code} symcode` - symcode code (ex: "EOS")

### Example - cleos

```bash
cleos push action wallet.sx close '["myaccount", "eosio.token", "EOS"]' -p myaccount
```

### Example - smart contract

```c++
const name account = "myaccount"_n;
const name contract = "eosio.token"_n;
const symbol_code symcode = symbol_code{"EOS"};

walletSx::close_action close( "wallet.sx"_n, { account, "active"_n });
close.send( account, contract, symcode );
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