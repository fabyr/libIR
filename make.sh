#!/bin/bash

LIBTORCH="/home/fabyr/Desktop/libIR/libtorch"

rm -rf build
mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH=$LIBTORCH ..
cmake --build . --config Release