#!/bin/bash

eosio-cpp flash.sx.cpp -I include
cleos set contract flash.sx . flash.sx.wasm flash.sx.abi

eosio-cpp examples/basic.cpp -I include
cleos set contract basic . basic.wasm basic.abi

eosio-cpp examples/callback.cpp -I include
cleos set contract callback . callback.wasm callback.abi
