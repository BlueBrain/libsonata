#!/usr/bin/env bash

# Install the desired clang-format, check the lines
# changed by this diff are formatted correctly

set -euo pipefail

VENV=build/venv-docstrings
CLANG_FORMAT_VERSION=9.0.0

if [[ ! -d $VENV ]]; then
    python3 -mvenv "$VENV"
    "$VENV/bin/pip" install clang-format=="$CLANG_FORMAT_VERSION"
fi

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

# regenerate the docstrings
python ./python/pybind11/tools/mkdoc.py ./include/bbp/sonata/*.h -o ./python/docstrings.h
sed -i '1s;^;/* clang-format off */\n;' ./python/docstrings.h

# fail if there are diffs in the generated docstrings
git diff --exit-code -- ./python/docstrings.h
