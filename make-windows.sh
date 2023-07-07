#!/bin/bash

rm -rf build
mkdir build
cd build

export SINGLE_PRECISION=1

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build . --config Release

cp libIR.dll ../libirsharp/libs/IR.dll