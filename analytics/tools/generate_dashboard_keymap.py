#!/usr/bin/env python3
"""Generate analytics/dashboard/keymap-data.js from physical mapping and keymap."""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
import sys

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "tools"))

import zmk_keymap_physical as zkp  # noqa: E402

# Pixel coordinates matching Data/Firmware/aroundforty_rb_keymap.html (780x232 canvas)
COL_X = [0, 60, 120, 180, 240, 300, 360, 420, 480, 540, 600, 660, 720]
ROW_Y = [0, 58, 116, 174]

LAYER_NAMES = {
    "0": "Win-Base",
    "1": "Mac-Base",
    "2": "Win-Fnc",
    "3": "Mac-Fnc",
    "4": "Win-Common",
    "5": "Mac-Common",
    "6": "Num",
    "7": "Mac-Num",
    "8": "V_Scroll",
    "9": "CLIP",
    "10": "Settings",
    "11": "AML",
    "12": "AML-On",
    "13": "Mac-Set",
}


def resolve_defines(text: str) -> dict[str, str]:
    constants: dict[str, str] = {}
    for line in text.splitlines():
        m = re.match(r"^\s*#define\s+(L_[A-Z0-9_]+)\s+(\d+)\b", line)
        if m:
            constants[m.group(1)] = m.group(2)
    return constants


def parse_hold_tap_behaviors(text: str) -> dict[str, dict[str, object]]:
    """Extract hold-tap and tap-dance timing settings from keymap devicetree."""
    behaviors: dict[str, dict[str, object]] = {
        "mt": {
            "kind": "mod-tap",
            "tapping_term_ms": 200,
            "quick_tap_ms": 0,
            "require_prior_idle_ms": None,
            "flavor": "balanced",
        },
        "lt": {
            "kind": "layer-tap",
            "tapping_term_ms": 200,
            "quick_tap_ms": 0,
            "require_prior_idle_ms": None,
            "flavor": "tap-preferred",
        },
    }

    mt_override = re.search(r"&mt\s*\{([^}]*)\};", text, re.S)
    if mt_override:
        body = mt_override.group(1)
        flavor = re.search(r'flavor\s*=\s*"([^"]+)"', body)
        qt = re.search(r"quick-tap-ms\s*=\s*<(\d+)>", body)
        tt = re.search(r"tapping-term-ms\s*=\s*<(\d+)>", body)
        if flavor:
            behaviors["mt"]["flavor"] = flavor.group(1)
        if qt:
            behaviors["mt"]["quick_tap_ms"] = int(qt.group(1))
        if tt:
            behaviors["mt"]["tapping_term_ms"] = int(tt.group(1))

    for m in re.finditer(r"(\w+)\s*:\s*\w+\s*\{([^{}]*)\};", text, re.S):
        name, body = m.group(1), m.group(2)
        compat = re.search(r'compatible\s*=\s*"([^"]+)"', body)
        if not compat:
            continue
        compat_val = compat.group(1)
        if compat_val == "zmk,behavior-hold-tap":
            tt = re.search(r"tapping-term-ms\s*=\s*<(\d+)>", body)
            qt = re.search(r"quick-tap-ms\s*=\s*<(\d+)>", body)
            idle = re.search(r"require-prior-idle-ms\s*=\s*<(\d+)>", body)
            flavor = re.search(r'flavor\s*=\s*"([^"]+)"', body)
            binds = re.search(r"bindings\s*=\s*<([^>]+)>", body)
            bind_str = binds.group(1) if binds else ""
            kind = "layer-tap" if ("&mo" in bind_str or "&tog" in bind_str) else "mod-tap"
            behaviors[name] = {
                "kind": kind,
                "tapping_term_ms": int(tt.group(1)) if tt else 200,
                "quick_tap_ms": int(qt.group(1)) if qt else 0,
                "require_prior_idle_ms": int(idle.group(1)) if idle else None,
                "flavor": flavor.group(1) if flavor else "hold-preferred",
            }
        elif compat_val == "zmk,behavior-tap-dance":
            tt = re.search(r"tapping-term-ms\s*=\s*<(\d+)>", body)
            behaviors[name] = {
                "kind": "tap-dance",
                "tapping_term_ms": int(tt.group(1)) if tt else 200,
                "quick_tap_ms": None,
                "require_prior_idle_ms": None,
                "flavor": "tap-dance",
            }
    return behaviors


def parse_combos(text: str, binding_order: list[str]) -> list[dict[str, object]]:
    combos_block = zkp.extract_block(text, "combos")
    combos: list[dict[str, object]] = []
    if not combos_block:
        return combos
    for m in re.finditer(r"(\w+)\s*\{([^{}]*)\};", combos_block, re.S):
        name, body = m.group(1), m.group(2)
        pos_m = re.search(r"key-positions\s*=\s*<([^>]+)>", body)
        if not pos_m:
            continue
        positions = [int(x) for x in pos_m.group(1).split()]
        timeout_m = re.search(r"timeout-ms\s*=\s*<(\d+)>", body)
        idle_m = re.search(r"require-prior-idle-ms\s*=\s*<(\d+)>", body)
        bind_m = re.search(r"bindings\s*=\s*<([^>]+)>", body)
        combos.append({
            "name": name,
            "positions": positions,
            "coords": [binding_order[p] for p in positions if 0 <= p < len(binding_order)],
            "timeout_ms": int(timeout_m.group(1)) if timeout_m else 50,
            "require_prior_idle_ms": int(idle_m.group(1)) if idle_m else None,
            "binding": bind_m.group(1).strip() if bind_m else name,
        })
    return combos


def short_key_label(token: str, defines: dict[str, str], symbol_map: dict[str, str]) -> str:
    if token in defines:
        return defines[token]
    if token in symbol_map:
        return symbol_map[token]
    short = {
        "LANGUAGE_1": "LANG1",
        "LANGUAGE_2": "LANG2",
        "PRINTSCREEN": "PrtSc",
        "LEFT_SHIFT": "LShift",
        "LSHFT": "LShift",
        "LCTRL": "LCtrl",
        "LGUI": "LGui",
        "LEFT_GUI": "LGui",
        "LALT": "LAlt",
        "LEFT_ALT": "LAlt",
        "ENTER": "Enter",
        "SPACE": "Space",
        "BSPC": "BSpc",
        "BACKSPACE": "BSpc",
        "DEL": "Del",
        "DELETE": "Del",
        "mac_ime": "IME",
        "ime_tog": "IME",
    }
    return short.get(token, token)


def describe_binding(
    behavior: str,
    args: list[str],
    constants: dict[str, str],
    defines: dict[str, str],
    symbol_map: dict[str, str],
    ht_defs: dict[str, dict[str, object]],
) -> dict[str, object]:
    resolved_args = [constants.get(a, a) for a in args]
    info: dict[str, object] = {
        "behavior": behavior,
        "tap": behavior,
        "hold": None,
        "category": "key",
        "timing": None,
    }
    if behavior == "kp" and resolved_args:
        info["tap"] = short_key_label(resolved_args[0], defines, symbol_map)
    elif behavior in ("mt", "mt_a_ctrl", "mt_z_shift") and len(resolved_args) >= 2:
        info["hold"] = short_key_label(resolved_args[0], defines, symbol_map)
        info["tap"] = short_key_label(resolved_args[1], defines, symbol_map)
        info["category"] = "mod-tap"
        info["timing"] = ht_defs.get(behavior, ht_defs["mt"])
    elif behavior in ("lt", "lt_num", "lt_typ") and len(resolved_args) >= 2:
        layer_id = resolved_args[0]
        layer_label = LAYER_NAMES.get(layer_id, f"L{layer_id}")
        info["hold"] = f"L{layer_id}:{layer_label}"
        info["tap"] = short_key_label(resolved_args[1], defines, symbol_map)
        info["category"] = "layer-tap"
        info["timing"] = ht_defs.get(behavior, ht_defs["lt"])
    elif behavior == "lt_clip_toggle" and len(resolved_args) >= 2:
        layer_id = resolved_args[0]
        info["hold"] = f"tog({LAYER_NAMES.get(layer_id, layer_id)})"
        info["tap"] = short_key_label(resolved_args[1], defines, symbol_map)
        info["category"] = "layer-tap"
        info["timing"] = ht_defs.get(behavior)
    elif behavior in ("settings_hold_tap", "mac_settings_hold_tap") and len(resolved_args) >= 1:
        layer_id = resolved_args[0]
        td_name = "mac_aml_toggle_td" if behavior.startswith("mac_") else "aml_toggle_td"
        td_term = ht_defs.get(td_name, {}).get("tapping_term_ms", 300)
        base_timing = dict(ht_defs.get(behavior, ht_defs["lt"]))
        base_timing["tap_dance_term_ms"] = td_term
        info["hold"] = LAYER_NAMES.get(layer_id, f"L{layer_id}")
        info["tap"] = "AML切替"
        info["category"] = "settings-ht"
        info["timing"] = base_timing
    elif behavior == "mac_ime":
        info["tap"] = "IME"
        info["category"] = "macro"
    return info


def build_keymap_data(mapping_path: Path, keymap_path: Path) -> dict[str, object]:
    mapping = json.loads(mapping_path.read_text(encoding="utf-8"))
    keymap_text = keymap_path.read_text(encoding="utf-8")

    defines = {**zkp.parse_defines(keymap_text), **mapping.get("macro_overrides", {})}
    symbol_map = {**zkp.DEFAULT_SYMBOL_MAP, **mapping.get("symbol_map", {})}
    constants = resolve_defines(keymap_text)
    ht_defs = parse_hold_tap_behaviors(keymap_text)

    arg_counts = dict(zkp.ARG_COUNTS)
    for custom_name, kind in mapping.get("custom_behaviors", {}).items():
        if kind in ("mt", "lt", "toggle_tap", "settings"):
            arg_counts[custom_name] = 2

    keymap_block = zkp.extract_block(keymap_text, "keymap")
    layers_raw = dict(zkp.parse_layers(keymap_block))

    def parse_flat_layer(layer_name: str) -> list[tuple[str, list[str]]]:
        body = layers_raw.get(layer_name, "")
        body = re.sub(r"/\*[\s\S]*?\*/", "", body)
        tokens: list[str] = []
        for line in body.splitlines():
            tokens.extend(zkp.tokenize_line(line))
        items: list[tuple[str, list[str]]] = []
        i = 0
        while i < len(tokens):
            tok = tokens[i]
            if tok.startswith("&"):
                beh = tok[1:]
                argc = arg_counts.get(beh, 0)
                args = tokens[i + 1 : i + 1 + argc]
                items.append((beh, args))
                i += 1 + argc
            else:
                i += 1
        return items

    win_items = parse_flat_layer("Win-Base")
    mac_items = parse_flat_layer("Mac-Base")

    grid_lookup: dict[str, tuple[int, int, str]] = {}
    headers = mapping["header"]
    for r_idx, row in enumerate(mapping["rows"]):
        for c_idx, cell in enumerate(row):
            if cell and cell != "CENTER":
                grid_lookup[cell] = (r_idx, c_idx, headers[c_idx])

    binding_order: list[str] = mapping["binding_order"]
    keys: list[dict[str, object]] = []
    for pos, coord in enumerate(binding_order):
        r_idx, c_idx, finger = grid_lookup[coord]
        hand = "L" if coord.startswith("L") else "R"
        col_num = int(coord.split("C")[1])
        is_thumb = finger == "親"
        is_home_row = (r_idx == 1 and not is_thumb) or (is_thumb and coord in ("L4C1", "R4C1", "R4C0", "L4C0"))

        win_beh, win_args = win_items[pos] if pos < len(win_items) else ("none", [])
        mac_beh, mac_args = mac_items[pos] if pos < len(mac_items) else ("none", [])
        win_desc = describe_binding(win_beh, win_args, constants, defines, symbol_map, ht_defs)
        mac_desc = describe_binding(mac_beh, mac_args, constants, defines, symbol_map, ht_defs)

        keys.append({
            "position": pos,
            "coord": coord,
            "hand": hand,
            "expected_source": "peripheral" if hand == "L" else "local",
            "row": r_idx + 1,
            "col_grid": c_idx,
            "col_center": col_num,
            "finger": finger,
            "finger_id": f"{hand}:{finger}",
            "is_thumb": is_thumb,
            "is_prime": is_home_row,
            "x": COL_X[c_idx],
            "y": ROW_Y[r_idx],
            "win": win_desc,
            "mac": mac_desc,
        })

    return {
        "keyboard_name": "AroundForty-RB",
        "position_count": len(keys),
        "canvas": {"width": 780, "height": 232, "key_width": 54, "key_height": 52},
        "fingers_order": ["小", "薬", "中", "人", "親"],
        "layer_names": list(layers_raw),
        "combos": parse_combos(keymap_text, binding_order),
        "keys": keys,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--map", type=Path, default=REPO_ROOT / "tools/around_forty_rb_mapping.json")
    parser.add_argument("--keymap", type=Path, default=REPO_ROOT / "config/AroundForty-RB.keymap")
    parser.add_argument("--out", type=Path, default=REPO_ROOT / "analytics/dashboard/keymap-data.js")
    args = parser.parse_args()

    data = build_keymap_data(args.map, args.keymap)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    payload = (
        "// Generated by analytics/tools/generate_dashboard_keymap.py — do not edit manually.\n"
        "(function (root) {\n"
        '  "use strict";\n'
        f"  const data = {json.dumps(data, ensure_ascii=False, indent=2)};\n"
        '  if (typeof module === "object" && module.exports) {\n'
        "    module.exports = data;\n"
        "  }\n"
        "  root.AROUNDFORTY_KEYMAP_DATA = data;\n"
        '})(typeof globalThis !== "undefined" ? globalThis : window);\n'
    )
    args.out.write_text(payload, encoding="utf-8")
    print(f"Wrote {args.out} ({data['position_count']} keys, {len(data['combos'])} combos)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
