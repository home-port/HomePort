#!/bin/sh

mkdir -p build
cd build
rm -rf *
clear
cmake -DCMAKE_INSTALL_PREFIX=/usr/local/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=Toolchains/armhf.cmake ../ && make
