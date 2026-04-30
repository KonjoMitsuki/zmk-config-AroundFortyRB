#!/usr/bin/env python3
"""Generate physical-layout Markdown tables from a ZMK keymap."""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Dict, List, Tuple

DEFAULT_SYMBOL_MAP = {
    "COMMA": ",",
    "DOT": ".",
    "SLASH": "/",
    "MINUS": "-",
    "EQUAL": "=",
    "LEFT_BRACKET": "[",
    "RIGHT_BRACKET": "]",
    "SEMICOLON": ";",
    "BACKSLASH": "\\",
    "GRAVE": "`",
}

DEFAULT_SHIFT_MAP = {
    "LS(COMMA)": "<",
    "LS(DOT)": ">",
    "LS(SLASH)": "?",
    "LS(N1)": "!",
    "LS(N2)": "\"",
    "LS(N3)": "#",
    "LS(N4)": "$",
    "LS(N5)": "%",
    "LS(N6)": "&",
    "LS(N7)": "'",
    "LS(N8)": "(",
    "LS(N9)": ")",
    "LS(N0)": "0",
    "LS(LEFT_BRACKET)": "{",
    "LS(RIGHT_BRACKET)": "}",
    "LS(BACKSLASH)": "|",
    "LS(MINUS)": "_",
    "LS(EQUAL)": "+",
    "LS(SEMICOLON)": "+",
    "LS(SINGLE_QUOTE)": "*",
}

ARG_COUNTS = {
    "kp": 1,
    "lt": 2,
    "mt": 2,
    "mo": 1,
    "to": 1,
    "mkp": 1,
    "msc": 1,
    "bt": 2,
    "trans": 0,
    "mac_ime": 0,
    "swapper": 0,
    "sys_reset": 0,
    "studio_unlock": 0,
    "bootloader": 0,
}


def parse_defines(text: str) -> Dict[str, str]:
    macro_map: Dict[str, str] = {}
    for line in text.splitlines():
        match = re.match(r"#define\s+(\w+)\s+.+?//\s*(.+)$", line)
        if not match:
            continue
        macro, comment = match.group(1), match.group(2).strip()
        macro_map[macro] = comment
    return macro_map


def extract_block(text: str, token: str) -> str:
    idx = text.find(token)
    if idx == -1:
        return ""
    brace_start = text.find("{", idx)
    if brace_start == -1:
        return ""
    depth = 0
    for i in range(brace_start, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[brace_start + 1 : i]
    return ""


def parse_layers(keymap_block: str) -> List[Tuple[str, str]]:
    layers: List[Tuple[str, str]] = []
    lines = keymap_block.splitlines()
    i = 0
    while i < len(lines):
        line = lines[i]
        match = re.match(r"\s*([A-Za-z0-9_-]+)\s*\{", line)
        if not match:
            i += 1
            continue
        name = match.group(1)
        depth = 0
        block_lines = []
        while i < len(lines):
            line = lines[i]
            depth += line.count("{")
            depth -= line.count("}")
            block_lines.append(line)
            if depth == 0:
                break
            i += 1
        block_text = "\n".join(block_lines)
        bind_match = re.search(r"bindings\s*=\s*<([\s\S]*?)>;", block_text)
        if bind_match:
            layers.append((name, bind_match.group(1)))
        i += 1
    return layers


def tokenize_line(line: str) -> List[str]:
    line = re.sub(r"//.*$", "", line).strip()
    return line.split()


def parse_bindings(bind_body: str) -> List[List[Tuple[str, List[str]]]]:
    rows: List[List[Tuple[str, List[str]]]] = []
    for line in bind_body.splitlines():
        tokens = tokenize_line(line)
        if not tokens:
            continue
        row: List[Tuple[str, List[str]]] = []
        i = 0
        while i < len(tokens):
            tok = tokens[i]
            if not tok.startswith("&"):
                i += 1
                continue
            beh = tok[1:]
            argc = ARG_COUNTS.get(beh)
            if beh == "bt":
                if i + 1 < len(tokens) and tokens[i + 1] == "BT_SEL":
                    argc = 2
                else:
                    argc = 1
            if argc is None:
                if beh.startswith("mt_"):
                    argc = 2
                elif beh.startswith("lt_"):
                    argc = 2
                else:
                    argc = 0
            args = tokens[i + 1 : i + 1 + argc]
            row.append((beh, args))
            i += 1 + argc
        rows.append(row)
    return rows


def html_escape_cell(value: str) -> str:
    value = value.replace("|", "&#124;")
    value = value.replace("`", "&#96;")
    return value


def comment_to_symbol(comment: str) -> str | None:
    if not comment:
        return None
    comment = comment.strip()
    if len(comment) == 1:
        return comment
    first = comment.split()[0]
    if len(first) == 1:
        return first
    return None


def render_key(
    token: str,
    macro_comments: Dict[str, str],
    macro_overrides: Dict[str, str],
    symbol_map: Dict[str, str],
    shift_map: Dict[str, str],
) -> str:
    if token in macro_overrides:
        return html_escape_cell(macro_overrides[token])
    if token in macro_comments:
        sym = comment_to_symbol(macro_comments[token])
        if sym:
            return html_escape_cell(sym)
    if token in shift_map:
        return html_escape_cell(shift_map[token])
    if token in symbol_map:
        return html_escape_cell(symbol_map[token])
    return html_escape_cell(token)


def format_binding(
    beh: str,
    args: List[str],
    macro_comments: Dict[str, str],
    macro_overrides: Dict[str, str],
    symbol_map: Dict[str, str],
    shift_map: Dict[str, str],
    custom_behaviors: Dict[str, str],
    hold_only: List[str],
) -> str:
    beh_type = custom_behaviors.get(beh)
    if beh_type == "mt":
        hold = render_key(args[0], macro_comments, macro_overrides, symbol_map, shift_map)
        tap = render_key(args[1], macro_comments, macro_overrides, symbol_map, shift_map)
        return f"**{hold}**,{tap}"
    if beh_type == "lt":
        layer = f"`{args[0]}`"
        tap = render_key(args[1], macro_comments, macro_overrides, symbol_map, shift_map)
        return f"**{layer}**,{tap}"

    if beh == "mt":
        hold = render_key(args[0], macro_comments, macro_overrides, symbol_map, shift_map)
        tap = render_key(args[1], macro_comments, macro_overrides, symbol_map, shift_map)
        return f"**{hold}**,{tap}"
    if beh == "lt":
        layer = f"`{args[0]}`"
        tap = render_key(args[1], macro_comments, macro_overrides, symbol_map, shift_map)
        return f"**{layer}**,{tap}"
    if beh == "mo":
        layer = f"`{args[0]}`"
        if beh in hold_only:
            return f"**{layer}**"
        return layer
    if beh == "to":
        return f"to(`{args[0]}`)"
    if beh == "kp":
        return render_key(args[0], macro_comments, macro_overrides, symbol_map, shift_map)
    if beh == "mkp":
        return f"mkp({args[0]})"
    if beh == "msc":
        return f"msc({args[0]})"
    if beh == "bt":
        if len(args) >= 2:
            return f"bt({args[0]} {args[1]})"
        if args:
            return f"bt({args[0]})"
        return "bt"
    if beh == "trans":
        return "trans"

    if args:
        return f"{beh}({', '.join(args)})"
    return beh


def build_grid(
    rows: List[List[Tuple[str, List[str]]]],
    formatter,
) -> List[List[str]]:
    grid: List[List[str]] = []
    for row in rows:
        grid.append([formatter(beh, args) for beh, args in row])
    return grid


def apply_mapping(
    grid: List[List[str]],
    layout_rows: List[List[str]],
    center_label: str,
) -> List[List[str]]:
    out_rows: List[List[str]] = []
    for phys_row in layout_rows:
        row_out: List[str] = []
        for cell in phys_row:
            if cell == "CENTER":
                row_out.append(center_label)
                continue
            if not cell:
                row_out.append("")
                continue
            match = re.match(r"P(\d+)C(\d+)", cell)
            if not match:
                row_out.append(cell)
                continue
            r = int(match.group(1)) - 1
            c = int(match.group(2)) - 1
            if r < 0 or r >= len(grid) or c < 0 or c >= len(grid[r]):
                row_out.append("")
            else:
                row_out.append(grid[r][c])
        out_rows.append(row_out)
    return out_rows


def render_table(header: List[str], rows: List[List[str]]) -> str:
    align = ["---"] * len(header)
    for i, h in enumerate(header):
        if h == "中心":
            align[i] = ":--:"
    lines = []
    lines.append("| " + " | ".join(header) + " |")
    lines.append("| " + " | ".join(align) + " |")
    for row in rows:
        lines.append("| " + " | ".join(row) + " |")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--src", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--map", required=True)
    parser.add_argument("--layers", default="")
    parser.add_argument("--append", action="store_true")
    parser.add_argument("--confirm-line", default="generated")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    src_text = Path(args.src).read_text(encoding="utf-8")
    mapping = json.loads(Path(args.map).read_text(encoding="utf-8"))

    macro_comments = parse_defines(src_text)
    macro_overrides = mapping.get("macro_overrides", {})
    symbol_map = {**DEFAULT_SYMBOL_MAP, **mapping.get("symbol_map", {})}
    shift_map = {**DEFAULT_SHIFT_MAP, **mapping.get("shift_map", {})}
    custom_behaviors = mapping.get("custom_behaviors", {})
    hold_only = mapping.get("hold_only_behaviors", [])

    header = mapping["header"]
    center_label = mapping.get("center_label", "中心")
    layout_rows = mapping["rows"]

    keymap_block = extract_block(src_text, "keymap")
    if not keymap_block:
        raise SystemExit("keymap block not found")

    layers = parse_layers(keymap_block)
    if args.layers:
        allowed = {x.strip() for x in args.layers.split(",")}
        layers = [layer for layer in layers if layer[0] in allowed]

    output_lines: List[str] = []
    for name, body in layers:
        rows = parse_bindings(body)
        formatter = lambda b, a: format_binding(
            b,
            a,
            macro_comments,
            macro_overrides,
            symbol_map,
            shift_map,
            custom_behaviors,
            hold_only,
        )
        grid = build_grid(rows, formatter)
        phys_rows = apply_mapping(grid, layout_rows, center_label)
        output_lines.append(f"### {name} Physical Layout\n")
        output_lines.append(render_table(header, phys_rows))
        output_lines.append("")

    if args.confirm_line:
        output_lines.append(args.confirm_line)

    output = "\n".join(output_lines).rstrip() + "\n"

    if args.dry_run:
        print(output)
        return 0

    mode = "a" if args.append else "w"
    Path(args.out).write_text(output, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
