#!/usr/bin/env bash

set -euxo pipefail

VENV=$(pwd)/build/venv-python-test/

if [[ ! -d "$VENV" ]]; then
    python3 -mvenv "$VENV"
fi

BIN=$VENV/bin/

set +u  # ignore missing variables in activation script
source $BIN/activate
set -u

$BIN/pip -v install --upgrade pip setuptools wheel

which python
which pip
pip debug

$BIN/pip -v install --force .

pushd python/tests
$BIN/python -m unittest -v
popd
