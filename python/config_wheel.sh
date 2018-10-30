function pre_build
{
    rm -rf build

    mkdir build
    pushd build

    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DSONATA_PYTHON=ON \
        -DPYBIND11_PYTHON_VERSION=${MB_PYTHON_VERSION} \
       ..

    make -j4 sonata_python

    popd

    install -m 644 python/setup.py build/python
}


function run_tests
{
    python --version
    python -c 'from sonata import *'
}
