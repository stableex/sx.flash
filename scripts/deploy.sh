#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create accounts
cleos create account eosio flash.sx EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio basic EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio vaults.sx EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio callback EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosio.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myaccount EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# deploy
cleos set contract flash.sx . flash.sx.wasm flash.sx.abi
cleos set contract eosio.token . eosio.token.wasm eosio.token.abi
cleos set contract basic . basic.wasm basic.abi
cleos set contract callback . callback.wasm callback.abi

# permission
cleos set account permission flash.sx active --add-code
cleos set account permission basic active --add-code
cleos set account permission callback active --add-code
cleos set account permission myaccount active callback --add-code

# create EOS token
cleos push action eosio.token create '["eosio", "100000000.0000 EOS"]' -p eosio.token
cleos push action eosio.token issue '["eosio", "5000000.0000 EOS", "init"]' -p eosio

# send token balances
cleos transfer eosio myaccount "50000.0000 EOS" "init"
cleos transfer eosio basic "1.0000 EOS"
cleos transfer eosio callback "1.0000 EOS"
