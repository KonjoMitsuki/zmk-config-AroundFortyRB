#!/usr/bin/env python3
"""Compatibility CLI for the canonical physical-layout generator.

Print by default; use --apply to write and save the previous output as .bak.
"""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', required=True)
    parser.add_argument('--out', default='keymap.md')
    parser.add_argument('--layers', default=None, help='comma-separated layer node names')
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    with tempfile.TemporaryDirectory(prefix='aroundforty-keymap-') as directory:
        generated = Path(directory) / 'keymap.md'
        command = [
            sys.executable, str(root / 'tools/zmk_keymap_physical.py'),
            '--src', args.source, '--out', str(generated),
            '--map', str(root / 'tools/around_forty_rb_mapping.json'),
        ]
        if args.layers:
            command.extend(['--layers', args.layers])
        subprocess.run(command, check=True)
        if args.dry_run or not args.apply:
            print(generated.read_text(encoding='utf-8'), end='')
            return
        output = Path(args.out)
        if output.exists():
            shutil.copy2(output, output.with_suffix(output.suffix + '.bak'))
        shutil.copyfile(generated, output)
        print('Wrote', output)


if __name__ == '__main__':
    main()
