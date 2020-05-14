#!/usr/bin/env bash

# Install the desired clang-format, check the lines
# changed by this diff are formatted correctly

set -euo pipefail

VENV=build/venv-docstrings
python3 -mvenv "$VENV"

# regenerate the docstrings
$VENV/bin/python mkdoc.py ./include/bbp/sonata/*.h -o ./python/generated/docstrings.h

# fail if there are diffs in the generated docstrings
git diff --exit-code -- ./python/generated/docstrings.h
