#!/bin/bash

# settings
cleos push action flash.sx setsettings '[[2]]' -p flash.sx
cleos transfer eosio flash.sx "100.0000 EOS" "init"

# basic
cleos push action basic init '["1.0000 EOS"]' -p basic

# callback
cleos push action callback init '["callback", "1.0000 EOS"]' -p callback

# ERROR - borrow without repaid
echo "❗️ERROR - flash.sx: borrowed quantity was not repaid before the end of inline action"
cleos push action flash.sx borrow '["myaccount", ["1.0000 EOS", "eosio.token"], "", ""]' -p myaccount

# ERROR - try to borrow too much
echo "❗️ERROR - flash.sx: maximum borrow amount is 99.8010 EOS"
cleos push action flash.sx borrow '["myaccount", ["1000.0000 EOS", "eosio.token"], "", ""]' -p myaccount

# ERROR - loan must be repaid by receiver
echo "❗️ERROR - flash.sx: during active flash loan, incoming transfers must be from `receiver`"
cleos push action callback init '["myaccount", "1.0000 EOS"]' -p callback
