#!/usr/bin/env bash

set -euxo pipefail

VENV=build/venv-python-test

if [[ ! -d $VENV ]]; then
    python3 -mvenv "$VENV"
fi

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

pip install --upgrade pip

# install
pip install .
pip install nose
nosetests python
