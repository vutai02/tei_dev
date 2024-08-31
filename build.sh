#!/bin/bash
pushd `pwd`
mkdir -p build
cd build
cmake .. -G "Ninja" -DESP_PLATFORM=1 -DCMAKE_TOOLCHAIN_FILE=$IDF_PATH/tools/cmake/toolchain-esp32s3.cmake && ninja

