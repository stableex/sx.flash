#!/bin/bash

# basic
cleos push action basic init '["1.0000 EOS"]' -p basic

# callback
cleos push action callback init '["myaccount", "1.0000 EOS"]' -p callback

# FAIL - attempt to borrow without paying back loan
cleos push action flash.sx borrow '["myaccount", "eosio.token", "1.0000 EOS", "", ""]' -p myaccount

# FAIL - attempt to borrow too much
cleos push action flash.sx borrow '["myaccount", "eosio.token", "1000.0000 EOS", "", ""]' -p myaccount
