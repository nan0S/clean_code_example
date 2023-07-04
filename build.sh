#!/bin/sh

mkdir -p build
pushd build

g++ ../cleancode.cpp -ocleancode -std=c++17 -O3

popd