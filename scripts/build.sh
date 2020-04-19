#!/usr/bin/env bash

mkdir -p dist

# flash.sx
eosio-cpp flash.sx.cpp -o dist/flash.sx.wasm -I external -R ricardian

# examples
eosio-cpp examples/basic.cpp -o dist/basic.wasm -I external
eosio-cpp examples/notifiers.cpp -o dist/notifiers.wasm -I external
