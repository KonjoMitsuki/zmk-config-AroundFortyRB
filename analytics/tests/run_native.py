#!/usr/bin/env python3
"""Run the audited upstream Combo suite against the actual ZMK event machinery.

Requires a populated v0.3.0 west workspace, host compiler, Python dependencies
and sed. Only synthetic input is logged. No upstream files are modified.
"""
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import re
import subprocess


def run(command, *, cwd, env, timeout=180):
    return subprocess.run(command, cwd=cwd, env=env, text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          timeout=timeout, check=True).stdout


def check_counts(output):
    # Independent reference: the unmodified physical_layouts producer, before
    # raise_zmk_position_state_changed(). Replays do not pass this producer.
    events = re.findall(r"position: (\d+), pressed: (true|false)", output)
    expected = Counter(int(p) for p, state in events if state == "true")
    reports = re.findall(r"ANALYTICS_POC position=(\d+) local=(\d+) peripheral=(\d+)", output)
    if not events or not reports:
        raise AssertionError("missing source events or aggregate report")
    observed = {int(p): int(local) for p, local, _ in reports}
    if len(observed) != len(reports) or set(observed) != set(range(len(reports))):
        raise AssertionError("duplicate or missing position reports")
    if not set(expected).issubset(observed):
        raise AssertionError("source position outside reported array")
    if any(observed[p] != expected[p] for p in observed):
        raise AssertionError(f"source={expected}, collector={observed}")
    if any(int(peripheral) for _, _, peripheral in reports):
        raise AssertionError("local input incorrectly classified as peripheral")
    releases = sum(state == "false" for _, state in events)
    footer = re.findall(r"ANALYTICS_POC releases_local=(\d+) releases_peripheral=(\d+) "
                        r"invalid=(\d+) saturated=(\d+)", output)
    if footer != [(str(releases), "0", "0", "0")]:
        raise AssertionError(f"unexpected footer: {footer}")
    return sum(expected.values())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--workspace", required=True, type=Path)
    parser.add_argument("--zmk", required=True, type=Path, help="ZMK repository root")
    parser.add_argument("--build-dir", required=True, type=Path)
    parser.add_argument("--jobs", type=int, default=2)
    parser.add_argument("--case", action="append", help="Combo directory name; default: all")
    args = parser.parse_args()
    workspace, zmk, output_dir = (p.resolve() for p in
                                  (args.workspace, args.zmk, args.build_dir))
    module = Path(__file__).resolve().parents[1]
    env = dict(os.environ, ZEPHYR_TOOLCHAIN_VARIANT="host", CMAKE_BUILD_PARALLEL_LEVEL="2")
    cases = sorted((zmk / "app/tests/combo").glob("*/native_posix_64.keymap"))
    if args.case:
        cases = [c for c in cases if c.parent.name in args.case]
        if {c.parent.name for c in cases} != set(args.case):
            parser.error("unknown Combo case")
    if not cases or args.jobs < 1:
        parser.error("no cases or invalid job count")
    output_dir.mkdir(parents=True, exist_ok=True)

    def test(keymap):
        case = keymap.parent
        build = output_dir / case.name
        build.mkdir(parents=True, exist_ok=True)
        command = ["west", "build", "-p", "always", "-s", str(zmk / "app"), "-d", str(build),
                   "-b", "native_posix_64", "--",
                   f"-DZephyr_DIR={workspace}/zephyr/share/zephyr-package/cmake",
                   f"-DZMK_CONFIG={case}", f"-DZMK_EXTRA_MODULES={module}",
                   "-DCONFIG_ASSERT=y", "-DCONFIG_ZMK_ANALYTICS=y",
                   "-DCONFIG_ZMK_ANALYTICS_POC_REPORT=y"]
        # Logs live outside the build directory because --pristine removes it.
        build_log = output_dir / f"{case.name}.build.log"
        try:
            build_log.write_text(run(command, cwd=workspace, env=env))
            output = run([str(build / "zephyr/zmk.exe")], cwd=workspace, env=env, timeout=20)
            (build / "synthetic-input.log").write_text(output)
            presses = check_counts(output)
            normalized = re.sub(r"^.*> ", "", output, flags=re.MULTILINE)
            filtered = subprocess.run(["sed", "-n", "-f", str(case / "events.patterns")],
                                      input=normalized, text=True, capture_output=True, check=True).stdout
            expected = (case / "keycode_events.snapshot").read_text()
            (build / "keycode_events.actual").write_text(filtered)
            if filtered.splitlines() != expected.splitlines():
                raise AssertionError("upstream HID/behavior snapshot mismatch")
            result = {"case": case.name, "status": "pass", "source_presses": presses}
        except subprocess.CalledProcessError as error:
            build_log.write_text(error.stdout or str(error))
            result = {"case": case.name, "status": "fail", "error": str(error)}
        except (AssertionError, subprocess.TimeoutExpired) as error:
            result = {"case": case.name, "status": "fail", "error": str(error)}
        print(f"{result['status'].upper()}: {case.name}", flush=True)
        return result

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        results = list(pool.map(test, cases))
    (output_dir / "results.json").write_text(json.dumps(results, indent=2) + "\n")
    return int(any(result["status"] != "pass" for result in results))


if __name__ == "__main__":
    raise SystemExit(main())
