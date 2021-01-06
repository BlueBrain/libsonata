#!/usr/bin/env bash

# This file runs the C++ tests, as well as compiling the code with warnings on
# so that errors should be caught quicker

set -euxo pipefail

BUILD_DIR=build/build-cpp-test
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

SONATA_VERSION=
if ! git describe --tags > /dev/null 2>&1; then
    SONATA_VERSION='-DSONATA_VERSION=0.0.0'
    echo "Likely shallow git clone, faking SONATA_VERSION -> $SONATA_VERSION"
fi

cmake $SONATA_VERSION                                                      \
    -DCMAKE_BUILD_TYPE=Release                                             \
    -DEXTLIB_FROM_SUBMODULES=ON                                            \
    -DSONATA_CXX_WARNINGS=ON                                               \
    ../..

make -j all test
