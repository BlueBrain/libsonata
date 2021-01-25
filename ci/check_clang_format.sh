#!/usr/bin/env bash

# Install the desired clang-format, check the lines
# changed by this diff are formatted correctly

set -euo pipefail

VENV=venv-clang-format
CLANG_FORMAT_VERSION=9.0.0

if [[ ! -d $VENV ]]; then
    python3 -mvenv "$VENV"
    # pinned to 19.3.1 b/c of: https://github.com/pypa/pip/issues/7629
    "$VENV/bin/pip" install --upgrade 'pip==19.3.1'
    "$VENV/bin/pip" install clang-format=="$CLANG_FORMAT_VERSION"
fi

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

changes=$(git-clang-format 'HEAD~1')
if [[ $(echo "$changes" | grep -n1 'changed files') ]]; then
    echo "The following files require changes to pass the current clang-format"
    echo "$changes"
    exit 1
fi
