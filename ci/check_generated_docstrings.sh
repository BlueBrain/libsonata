#!/usr/bin/env bash

# Check the generated docstrings
#
# pybind11 ships with mkdoc.py which extracts python docstrings
# from the C++ doxygen comments. This is beneficial for maintaining
# consistency and reducing duplication.
#
# In order to enable libsonata to be built without enforcing clang
# as a dependency (required by mkdoc.py), we include the generated
# docstrings in source control.
#
# In order to ensure they are being kept up-to-date, the CI process
# regenerates them and checks there are no diffs.

set -x
set -euo pipefail

if [[ -z $LIBCLANG_PATH ]]; then
    echo "Expect a \$LIBCLANG_PATH in the environment, it should have the path to the libclang.so"
    exit -1
fi

VERSION=bd40db3b4e24cd14d0f02fc782c7fdae4e17d351
PACKAGE=git+git://github.com/pybind/pybind11_mkdoc.git@$VERSION

VENV=build/venv-docstrings
if [[ ! -d $VENV ]]; then
    python3 -mvenv "$VENV"
    $VENV/bin/pip install -U pip setuptools wheel
    $VENV/bin/python -m pip install $PACKAGE
fi

DOCSTRING_PATH=./python/generated/docstrings.h

rm -f $DOCSTRING_PATH

$VENV/bin/python -m pybind11_mkdoc \
  -o $DOCSTRING_PATH \
  ./include/bbp/sonata/*.h \
  -Wno-pragma-once-outside-header \
  -ferror-limit=100000 \
  -I/usr/include/hdf5/serial \
  -I./extlib/HighFive/include \
  -I./include

# fail if there are diffs in the generated docstrings
git diff --exit-code -- $DOCSTRING_PATH
