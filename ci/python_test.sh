#!/usr/bin/env bash

set -euxo pipefail

VENV=env

python -m pip install virtualenv --upgrade
rm -rf "$VENV"
virtualenv "$VENV" --python=python3 --seeder=pip

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

python -m pip install --upgrade pip

# install
python -m pip install .
python -m pip install nose
nosetests python
