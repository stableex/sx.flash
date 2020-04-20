#!/usr/bin/env bash

mkdir -p dist
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# deploy smart contract
cleos set contract flash.sx ./dist flash.sx.wasm flash.sx.abi
cleos set contract basic ./dist basic.wasm basic.abi
cleos set contract notifiers ./dist notifiers.wasm notifiers.abi

# add eosio.code
cleos set account permission flash.sx active --add-code
cleos set account permission basic active --add-code
cleos set account permission notifiers active --add-code
