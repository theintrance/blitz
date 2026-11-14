#!/bin/bash

set -euo pipefail

if ! command -v uvx > /dev/null; then
    echo "uvx is not installed: https://docs.astral.sh/uv/"
    exit 1
fi

cd "$(git rev-parse --show-toplevel)"

uvx cpplint@2.0.2 --quiet --recursive --config=CPPLINT.cfg .
