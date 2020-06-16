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
#
# TODO: on GitHub actions we are forced to create a symlink to
# for libclang.so. It would be preferable if we instead set the
# path to the library with the clang bindings. This doesn't seem
# to be possible at the moment.

set -euo pipefail

VENV=build/venv-docstrings
python3 -mvenv "$VENV"

# regenerate the docstrings
# Note: the path to mkdoc.py (python/pybind11/tools) must be in the PYTHONPATH
$VENV/bin/python \
  -m mkdoc ./include/bbp/sonata/*.h \
  -o ./python/generated/docstrings.h \
  -Wno-pragma-once-outside-header \
  -ferror-limit=100000 \
  -I/usr/include/hdf5/serial \
  -I./extlib/HighFive/include \
  -I./include

# fail if there are diffs in the generated docstrings
git diff --exit-code -- ./python/generated/docstrings.h
