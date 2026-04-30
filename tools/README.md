# ZMK Physical Layout Generator

This tool generates Markdown tables for the physical keyboard layout from a ZMK keymap file.

## Requirements

- Python 3.8+

## Usage

```bash
python3 tools/zmk_keymap_physical.py \
  --src config/AroundForty-RB.keymap \
  --out keymap.md \
  --map tools/around_forty_rb_mapping.json
```

## Options

- `--layers Win-Base,Mac-Base` Only generate the listed layers.
- `--append` Append instead of overwrite.
- `--confirm-line generated` Confirmation line to add at the end.
- `--dry-run` Print output to stdout without writing.

## Mapping

Edit the layout or symbol behavior in:

- tools/around_forty_rb_mapping.json
