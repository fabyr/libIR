#!/bin/bash

rm -rf build
mkdir build
cd build

export SINGLE_PRECISION=1

cmake -DCMAKE_SHARED_LINKER_FLAGS="-static-libgcc -static" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build . --config Release

ERRORCODE=$?
if [ $ERRORCODE -ne 0 ]; then
    echo "Failed."
    exit 1
fi

cp libIR.dll ../libirsharp/libs/IR.dll