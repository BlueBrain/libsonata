#!/usr/bin/env bash

# This builds coverage information, including HTML output

set -euxo pipefail

BUILD_DIR=build/build-coverage

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake                           \
    -DCMAKE_BUILD_TYPE=Debug    \
    -DEXTLIB_FROM_SUBMODULES=ON \
    ../..
make -j all coverage
