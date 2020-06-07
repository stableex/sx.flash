#!/bin/bash

# basic
cleos push action basic init '["1.0000 EOS"]' -p basic

# callback
cleos push action callback init '["myaccount", "1.0000 EOS"]' -p callback
