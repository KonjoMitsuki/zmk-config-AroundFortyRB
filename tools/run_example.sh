#!/usr/bin/env bash
set -euo pipefail

python3 tools/zmk_keymap_physical.py \
  --src config/AroundForty-RB.keymap \
  --out keymap.md \
  --map tools/around_forty_rb_mapping.json
