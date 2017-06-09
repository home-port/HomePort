#!/bin/sh

mkdir -p build
cd build
rm -rf *
clear
cmake -DCMAKE_INSTALL_PREFIX=/tmp/build/usr/local/ -DDAT_PREFIX=/usr/local/share/house_gfx/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../rpi_toolchain.cmake ../ && make
