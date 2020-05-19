#!/usr/bin/env bash

# upload the sdist files to PyPI

set -euo pipefail

if [[ -z "$TRAVIS" ]]; then
    echo "This script is meant to be run on travis-ci.com"
    exit -1
fi

if [[ -z "$TRAVIS" ]]; then
    exit -1
fi

env

pip3 install setuptools pip twine

python3 setup.py sdist

ls -al dist


export TWINE_REPOSITORY_URL=https://test.pypi.org/legacy/
twine upload --verbose -u bbp.opensource -p "$PYPI_PASSWORD" dist/*
