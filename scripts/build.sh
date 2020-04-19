#!/usr/bin/env bash

mkdir -p dist

# flash.sx
eosio-cpp src/flash.sx.cpp -o dist/flash.sx.wasm -I external -I include -R ricardian

# examples
eosio-cpp examples/basic.cpp -o dist/basic.wasm -I external
eosio-cpp examples/notifiers.cpp -o dist/notifiers.wasm -I external
