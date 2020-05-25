#!/usr/bin/env bash

# upload the sdist files to PyPI

set -euo pipefail

if [[ -z "$TRAVIS" ]]; then
    echo "This script is meant to be run on travis-ci.com"
    exit -1
fi

pip install setuptools pip twine
python setup.py sdist
ls -al dist
twine upload --verbose -u bbp.opensource -p "$PYPI_PASSWORD" dist/*
