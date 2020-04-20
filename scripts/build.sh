#!/usr/bin/env bash

mkdir -p dist

# flash.sx
eosio-cpp src/flash.sx.cpp -o dist/flash.sx.wasm -I include -R ricardian

# examples
eosio-cpp examples/basic.sx.cpp -o dist/basic.sx.wasm  -I include
eosio-cpp examples/callback.sx.cpp -o dist/callback.sx.wasm -I include
