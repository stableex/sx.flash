#!/bin/bash

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
