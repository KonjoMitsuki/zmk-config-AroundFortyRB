#!/usr/bin/env python3
"""
Generate physical-layout Markdown tables from a ZMK .keymap file.

Minimal implementation: parses layers and bindings from the keymap file,
formats tokens per rules in pro.md/prompt.md, and prints a unified diff
or writes to `keymap.md` when --apply is provided.

Usage:
  python3 scripts/generate_keymap_table.py --source config/AroundForty-RB.keymap --out keymap.md --dry-run

This is a conservative, dependency-free implementation intended as a starting point.
"""
import argparse
import re
import sys
from pathlib import Path
from textwrap import dedent


SYMBOL_MAP = {
    'COMMA': ',', 'DOT': '.', 'MINUS': '-', 'SLASH': '/', 'SEMICOLON': ';', 'EQUAL': '=',
    'PLUS': '+', 'BACKSLASH': '\\', 'GRAVE': '`', 'FSLH': '/', 'JP_LT': '<', 'JP_GT': '>',
}


def split_bindings_block(block_text):
    # collapse newlines within the bindings block to single spaces
    s = re.sub(r"\n", " ", block_text)
    # split on 2+ whitespace which separates entries in the formatted .keymap
    entries = [e.strip() for e in re.split(r"\s{2,}", s) if e.strip()]
    return entries


def fmt_key(tok: str) -> str:
    tok = tok.strip()
    if not tok:
        return ''
    parts = tok.split()
    head = parts[0]
    name = head.lstrip('&')
    args = parts[1:]

    if name == 'trans' or tok == 'trans':
        return 'trans'

    if name.startswith('mt'):
        # mt-like: &mt_x MOD KEY  -> **MOD**,KEY
        if len(args) >= 2:
            hold, tap = args[0], args[1]
            return f"**{hold}**, {tap}".replace(', ', ',')
        return tok

    if name.startswith('lt'):
        # &lt N KEY  or &lt_num N KEY
        if len(args) >= 2:
            layer, key = args[0], args[1]
            return f"**`{layer}`**,{display_key(key)}"
        return tok

    if name in ('lt',):
        if len(args) >= 2:
            return f"**`{args[0]}`**,{display_key(args[1])}"

    if name in ('mo',):
        if len(args) >= 1:
            return f"mo(`{args[0]}`)"

    if name in ('to',):
        if len(args) >= 1:
            return f"to(`{args[0]}`)"

    if name in ('bt', 'mkp', 'msc', 'kp', 'kt'):
        if args:
            return f"{name}({ ' '.join(args) })"
        return name

    # generic &kp KEY or raw KEY
    if name == 'kp' and args:
        return display_key(args[0])

    # if head contains '(' it's likely a wrapped expression like LC(A)
    if re.search(r"\(.*\)", tok):
        return tok

    # fallback: join parts
    if args:
        joined = ' '.join(args)
        return joined
    return name


def display_key(k: str) -> str:
    # map common tokens to symbols
    if k in SYMBOL_MAP:
        return SYMBOL_MAP[k]
    # preserve tokens like COMMA, DOT if unmapped
    return k


def parse_keymap(path: Path):
    text = path.read_text()
    # find keymap block
    m = re.search(r"keymap\s*\{(.*)\n\s*\};", text, re.S)
    if not m:
        raise RuntimeError('keymap block not found')
    keymap_block = m.group(1)
    # find each layer block: Name { // layer N (optional) ... bindings = < ... >; };
    layer_re = re.compile(r"([A-Za-z0-9_\-]+)\s*\{[^}]*?display-name\s*=\s*\"([^\"]*)\";.*?bindings\s*=\s*<([^>]*)>;.*?\};", re.S)
    layers = []
    for lm in layer_re.finditer(keymap_block):
        name = lm.group(1)
        display = lm.group(2)
        bindings_block = lm.group(3)
        entries = split_bindings_block(bindings_block)
        layers.append((name, display, entries))
    return layers


def make_table_for_layer(display_name, entries):
    # expected program grid counts per row
    counts = [10, 10, 11, 11]
    idx = 0
    rows = []
    for c in counts:
        row_entries = entries[idx:idx+c]
        idx += c
        rows.append(row_entries)

    # build physical 13-column rows per earlier rules
    phys_rows = []
    for r in rows:
        cols = [''] * 13
        # center column index 6 (0-based)
        cols[6] = '中心'
        if len(r) == 10:
            left = r[:5]
            right = r[5:10]
            for i, v in enumerate(left):
                cols[i] = fmt_key(v)
            # leave cols[5] empty
            for i, v in enumerate(right):
                cols[7 + i] = fmt_key(v)
        elif len(r) == 11:
            left = r[:6]
            right = r[6:11]
            for i, v in enumerate(left):
                cols[i] = fmt_key(v)
            for i, v in enumerate(right):
                cols[7 + i] = fmt_key(v)
        else:
            # fallback: fill left to right skipping center
            i = 0
            for v in r:
                if i == 6:
                    i = 7
                if i < 13:
                    cols[i] = fmt_key(v)
                    i += 1
        phys_rows.append(cols)

    # render markdown table
    header = ['小','薬','中','人','人','親','中心','親','人','人','中','薬','小']
    md = []
    md.append(f"### レイヤー: {display_name}")
    md.append('')
    md.append('|' + ' | '.join(header) + ' |')
    md.append('|' + ' --- |' * len(header))
    for cols in phys_rows:
        md.append('|' + ' | '.join(c if c else ' ' for c in cols) + ' |')
    md.append('')
    return '\n'.join(md)


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--source', required=True)
    p.add_argument('--out', default='keymap.md')
    p.add_argument('--layers', default=None, help='comma separated layer display-names to generate')
    p.add_argument('--apply', action='store_true')
    p.add_argument('--dry-run', action='store_true')
    args = p.parse_args()

    src = Path(args.source)
    if not src.exists():
        print('source not found:', src, file=sys.stderr)
        sys.exit(1)

    layers = parse_keymap(src)
    selected = None
    if args.layers:
        want = [s.strip() for s in args.layers.split(',')]
        selected = [l for l in layers if l[1] in want or l[0] in want]
    else:
        selected = layers

    out_md = []
    for name, display, entries in selected:
        out_md.append(make_table_for_layer(display, entries))

    result = '\n'.join(out_md)

    out_path = Path(args.out)
    if args.dry_run or not args.apply:
        print(result)
        sys.exit(0)

    # write backup
    if out_path.exists():
        bak = out_path.with_suffix(out_path.suffix + '.bak')
        out_path.replace(bak)
    out_path.write_text(result)
    print('Wrote', out_path)


if __name__ == '__main__':
    main()
