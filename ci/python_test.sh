#!/usr/bin/env bash

set -euxo pipefail

VENV=$(pwd)/build/venv-python-test/

if [[ ! -d "$VENV" ]]; then
    # We use virtualenv instead of `python3 -mvenv` because of python2 tests
    pip install virtualenv
    virtualenv "$VENV"
fi

BIN=$VENV/bin/

source $BIN/activate
pip -v install --upgrade pip setuptools wheel

# install
pip -v install --force .
pip install nose

nosetests -s -v -P python/tests

PYTHON_MAJOR_VERSION=$(python -c 'import sys; print(sys.version_info[0])')
if [[ $PYTHON_MAJOR_VERSION -ge 3 ]]; then
    python setup.py test
fi
