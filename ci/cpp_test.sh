#!/usr/bin/env bash

# This file runs the C++ tests, as well as compiling the code with warnings on
# so that errors should be caught quicker

set -euxo pipefail

BUILD_DIR=build/build-cpp-test
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake                           \
    -DCMAKE_BUILD_TYPE=Release  \
    -DEXTLIB_FROM_SUBMODULES=ON \
    -DSONATA_CXX_WARNINGS=ON    \
    ../..
make -j all test
