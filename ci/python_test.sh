#!/usr/bin/env bash

set -euxo pipefail

VENV=env

rm -rf "$VENV"
python3 -m venv "$VENV"

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

pip install --upgrade pip

# install
pip install .
pip install nose
nosetests python
