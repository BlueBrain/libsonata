#!/usr/bin/env bash

set -euxo pipefail

VENV=$(pwd)/build/venv-python-test/

if [[ ! -d "$VENV" ]]; then
    # We use virtualenv instead of venv for python2 tests
    pip install virtualenv
    virtualenv "$VENV"
fi

BIN=$VENV/bin/

$BIN/pip -v install --upgrade pip setuptools wheel

# install
$BIN/pip -v install --force .
$BIN/pip install nose

# prevent the $REPO_ROOT/libsonata/ directory from interfering with the tests
cd python
$BIN/nosetests -s -v
