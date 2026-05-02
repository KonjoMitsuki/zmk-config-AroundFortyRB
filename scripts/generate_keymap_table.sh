#!/usr/bin/env bash
# Wrapper to run the Python generator
set -euo pipefail
PY=${PY:-python3}
DIR=$(cd "$(dirname "$0")" && pwd)
"$PY" "$DIR/generate_keymap_table.py" --source config/AroundForty-RB.keymap --out keymap.md "$@"
