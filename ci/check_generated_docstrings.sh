#!/usr/bin/env bash

# Install the desired clang-format, check the lines
# changed by this diff are formatted correctly

set -euo pipefail

VENV=build/venv-docstrings

if [[ ! -d $VENV ]]; then
    python3 -mvenv "$VENV"
fi

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

# regenerate the docstrings
python ./python/pybind11/tools/mkdoc.py ./include/bbp/sonata/*.h -o ./python/docstrings.h
sed -i '1s;^;/* clang-format off */\n;' ./python/docstrings.h

# fail if there are diffs in the generated docstrings
git diff --exit-code -- ./python/docstrings.h
