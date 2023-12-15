#!/usr/bin/env bash

# build the sdist file, and try installing it

set -euo pipefail

BASE=$(pwd)/build/

mkdir -p "$BASE"

VENV="$BASE/venv-python-sdist/"
if [[ ! -d "$VENV" ]]; then
    python3 -mvenv "$VENV"
fi
BIN="$VENV/bin/"

set +u  # ignore missing variables in activation script
source "$BIN/activate"
set -u

"$BIN/pip" install -U setuptools pip
"$BIN/pip" install -U twine build

"$BIN/python" -m build

ls -al dist

cd "$VENV"
"$BIN/pip" --verbose install ../../dist/*.tar.gz
