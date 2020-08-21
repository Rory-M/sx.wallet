#!/bin/bash

# deposit
cleos transfer myaccount wallet.sx "10.0000 EOS"

# withdraw
cleos push action wallet.sx withdraw '["myaccount", "eosio.token", "1.0000 EOS"]' -p myaccount

# transfer
cleos push action wallet.sx transfer '["myaccount", "basic", "eosio.token", "1.0000 EOS", null]' -p myaccount

# basic example
cleos push action basic deposit '["basic", "eosio.token", "2.0000 EOS"]' -p basic
cleos push action basic withdraw '["basic", "eosio.token", "1.0000 EOS"]' -p basic
cleos push action basic balance '["basic", "eosio.token", "EOS"]' -p basic
cleos push action basic transfer '["basic", "myaccount", "eosio.token", "1.0000 EOS", null]' -p basic
