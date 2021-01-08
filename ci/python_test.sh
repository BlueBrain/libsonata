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
$BIN/pip install nose

# run tests with nose
$BIN/pip -v install --force .
$BIN/nosetests -s -v -P python/tests

# run tests through setup.py; also builds documentation
$BIN/python setup.py test
