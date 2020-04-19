#!/usr/bin/env bash

# basic
cleos push action basic myaction '["eosio.token", "1.0000 EOS", "memo"]' -p basic

# notifiers
cleos push action notifiers myaction '["myaccount", "eosio.token", "1.0000 EOS", "memo", ["notifiers"]]' -p notifiers
