#!/usr/bin/env bash

# unlock wallet
mkdir -p dist
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# build
eosio-cpp flash.sx.cpp -o flash.sx.wasm
eosio-cpp examples/basic.cpp -o basic.wasm
eosio-cpp examples/callback.cpp -o callback.wasm

# create accounts
cleos system newaccount eosio flash.sx EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV --stake-net "1.0000 EOS" --stake-cpu "10.0000 EOS" --buy-ram "100.0000 EOS"
cleos system newaccount eosio basic EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV --stake-net "1.0000 EOS" --stake-cpu "10.0000 EOS" --buy-ram "100.0000 EOS"
cleos system newaccount eosio callback EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV --stake-net "1.0000 EOS" --stake-cpu "10.0000 EOS" --buy-ram "100.0000 EOS"
cleos system newaccount eosio myaccount EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV --stake-net "1.0000 EOS" --stake-cpu "10.0000 EOS" --buy-ram "100.0000 EOS"

# deploy smart contract
cleos set contract flash.sx . flash.sx.wasm flash.sx.abi
cleos set contract basic . basic.wasm basic.abi
cleos set contract callback . callback.wasm callback.abi

# add eosio.code
cleos set account permission flash.sx active --add-code
cleos set account permission basic active --add-code
cleos set account permission callback active --add-code
cleos set account permission myaccount callback --add-code

# basic
cleos push action basic myaction '["eosio.token", "1.0000 EOS", "memo"]' -p basic

# callback
cleos push action callback myaction '["myaccount", "eosio.token", "1.0000 EOS", "memo", "callback"]' -p myaccount
