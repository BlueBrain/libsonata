#!/bin/bash
set -e -x


export INPUT=$(cd $(dirname "$1") && pwd -P)/$(basename "$1")
export OUTPUT="$INPUT/install-$CIBW_ARCHS_MACOS"

: "${UNIXY_HDF5_VERSION:=1.14.3}"

function download_unpack_hdf5 {
    pushd "$INPUT"
    local name=CMake-hdf5-$UNIXY_HDF5_VERSION.tar.gz
    if [[ ! -e $name ]]; then
        echo "Downloading & unpacking HDF5 ${UNIXY_HDF5_VERSION}"
        curl -fsSLO "https://www.hdfgroup.org/ftp/HDF5/releases/hdf5-${UNIXY_HDF5_VERSION%.*}/hdf5-$UNIXY_HDF5_VERSION/src/$name"
    fi
    tar xzf "$name"
    popd
}

if [[ "$OSTYPE" == "darwin"* ]]; then
    lib_name=libhdf5.dylib
    NPROC=$(sysctl -n hw.ncpu)
else
    lib_name=libhdf5.so
    NPROC=$(nproc)
fi

if [[ -f "$OUTPUT/lib/$lib_name" ]]; then
    echo "using cached build"
else
    if [[ "$OSTYPE" == "darwin"* ]]; then
        export CC="/usr/bin/clang"
        export CXX="/usr/bin/clang"
        export CFLAGS="$CFLAGS -arch $CIBW_ARCHS_MACOS"
        export CPPFLAGS="$CPPFLAGS -arch $CIBW_ARCHS_MACOS"
        export CXXFLAGS="$CXXFLAGS -arch $CIBW_ARCHS_MACOS"
    fi

    echo "Building & installing hdf5"
    download_unpack_hdf5

    install="$OUTPUT/install"
    mkdir -p "$OUTPUT/build"
    pushd "$OUTPUT/build"
    cmake -G'Unix Makefiles' \
        -DCMAKE_BUILD_TYPE:STRING=Release \
        -DBUILD_SHARED_LIBS:BOOL=OFF \
        -DHDF5_BUILD_UTILS:BOOL=OFF \
        -DHDF5_BUILD_HL_LIB:BOOL=OFF \
        -DHDF5_BUILD_EXAMPLES:BOOL=OFF \
        -DBUILD_TESTING:BOOL=OFF \
        -DHDF5_BUILD_TOOLS:BOOL=OFF \
        -DCMAKE_INSTALL_PREFIX="$install" \
        "$INPUT/CMake-hdf5-$UNIXY_HDF5_VERSION/hdf5-$UNIXY_HDF5_VERSION"
    make -j "$NPROC"
    make install
    popd

    find "$OUTPUT"
fi
