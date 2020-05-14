#!/usr/bin/env bash

# Install the desired clang-format, check the lines
# changed by this diff are formatted correctly

set -euo pipefail

VENV=build/venv-docstrings
CLANG_VERSION=6.0.0.2

if [[ ! -d $VENV ]]; then
    python3 -mvenv "$VENV"
    "$VENV/bin/pip" install clang=="$CLANG_VERSION"
fi

# regenerate the docstrings
$VENV/bin/python ./python/pybind11/tools/mkdoc.py ./include/bbp/sonata/*.h -o ./python/generated/docstrings.h

# fail if there are diffs in the generated docstrings
git diff --exit-code -- ./python/generated/docstrings.h
