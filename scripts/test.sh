#!/bin/bash

# basic
cleos push action basic init '["1.0000 EOS"]' -p basic

# callback
cleos push action callback init '["myaccount", "1.0000 EOS"]' -p callback

# save & check
cleos push action flash.sx savebalance '["myaccount", [{"sym": "4,EOS", "contract": "eosio.token"}]]' -p myaccount
cleos push action flash.sx checkbalance '["myaccount", [{"sym": "4,EOS", "contract": "eosio.token"}]]' -p myaccount
