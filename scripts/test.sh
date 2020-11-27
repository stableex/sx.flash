#!/bin/bash

# create vault
cleos push action eosio.token open '["flash.sx", "4,EOS", "flash.sx"]' -p flash.sx
cleos push action flash.sx setvault '[["4,EOS", "eosio.token"], "SXEOS"]' -p flash.sx

# deposit
cleos -v transfer myaccount flash.sx "10.0000 EOS"

# withdraw
cleos -v transfer myaccount flash.sx "10000.0000 SXEOS" --contract token.sx

# settings
cleos push action flash.sx setsettings '[[2]]' -p flash.sx

# basic
cleos push action basic init '["1.0000 EOS"]' -p basic

# callback
cleos push action callback init '["myaccount", "1.0000 EOS"]' -p callback

# ERROR - borrow
echo "❗️ERROR - attempt to borrow without paying back loan"
cleos push action flash.sx borrow '["myaccount", ["1.0000 EOS", "eosio.token"], "", ""]' -p myaccount

# ERROR - borrow
echo "❗️ERROR - attempt to borrow too much"
cleos push action flash.sx borrow '["myaccount", ["1000.0000 EOS", "eosio.token"], "", ""]' -p myaccount
