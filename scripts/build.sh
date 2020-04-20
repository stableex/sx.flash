#!/usr/bin/env bash

mkdir -p dist

# flash.sx
eosio-cpp src/flash.sx.cpp -o dist/flash.sx.wasm -I include -R ricardian

# examples
eosio-cpp examples/basic.cpp -o dist/basic.wasm  -I include
eosio-cpp examples/notifiers.cpp -o dist/notifiers.wasm -I include
