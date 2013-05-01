#!/bin/sh

mkdir -p build
cd build
rm -rf *
clear
cmake -DCMAKE_BUILD_TYPE=Debug ../ && make