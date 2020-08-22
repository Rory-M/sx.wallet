#!/bin/bash

eosio-cpp wallet.sx.cpp -I include
cleos set contract wallet.sx . wallet.sx.wasm wallet.sx.abi

eosio-cpp examples/basic.cpp -I include
cleos set contract basic . basic.wasm basic.abi
