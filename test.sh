#!/usr/bin/env bash

# unlock wallet
mkdir -p dist
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# flash.sx
eosio-cpp flash.sx.cpp -o flash.sx.wasm -I eosio.token

# create accounts
$ cleos system newaccount eosio flash.sx EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV --stake-net "1.0000 EOS" --stake-cpu "10.0000 EOS" --buy-ram "100.0000 EOS"
$ cleos system newaccount eosio basic EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV --stake-net "1.0000 EOS" --stake-cpu "10.0000 EOS" --buy-ram "100.0000 EOS"
$ cleos system newaccount eosio callback EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV --stake-net "1.0000 EOS" --stake-cpu "10.0000 EOS" --buy-ram "100.0000 EOS"

# deploy smart contract
cleos set contract flash.sx ./dist flash.sx.wasm flash.sx.abi
cleos set contract basic ./dist basic.wasm basic.abi
cleos set contract callback ./dist callback.wasm callback.abi

# add eosio.code
cleos set account permission flash.sx active --add-code
cleos set account permission basic active --add-code
cleos set account permission callback active --add-code

# basic
cleos push action basic myaction '["eosio.token", "1.0000 EOS", "memo"]' -p basic

# callback
cleos push action callback myaction '["myaccount", "eosio.token", "1.0000 EOS", "memo", "callback"]' -p myaccount
