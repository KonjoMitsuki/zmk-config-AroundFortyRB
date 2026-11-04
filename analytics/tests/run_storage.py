#!/usr/bin/env python3
"""Exercise the real Zephyr settings/NVS backend across native process restarts.

Uses fresh simulator files in a new output directory, never a real device.
"""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--workspace", required=True, type=Path)
    parser.add_argument("--zmk", required=True, type=Path)
    parser.add_argument("--build-dir", required=True, type=Path)
    args = parser.parse_args()
    workspace, zmk, root = (p.resolve() for p in (args.workspace, args.zmk, args.build_dir))
    if root.exists():
        parser.error("use a new --build-dir to avoid reusing or overwriting test flash")
    root.mkdir(parents=True)
    module = Path(__file__).resolve().parents[1]
    env = dict(os.environ, ZEPHYR_TOOLCHAIN_VARIANT="host", CMAKE_BUILD_PARALLEL_LEVEL="2")

    def build(name, identity):
        target = root / name
        command = ["west", "build", "-s", str(zmk / "app"), "-d", str(target),
                   "-b", "native_posix_64", "--",
                   f"-DZephyr_DIR={workspace}/zephyr/share/zephyr-package/cmake",
                   f"-DZMK_CONFIG={module}/tests/storage-probe",
                   f"-DZMK_EXTRA_MODULES={module};{module}/tests/storage-probe",
                   f'-DCONFIG_ZMK_ANALYTICS_LAYOUT_ID="{identity}"']
        with (root / f"{name}.build.log").open("w") as log:
            subprocess.run(command, cwd=workspace, env=env, stdout=log, stderr=subprocess.STDOUT,
                           timeout=240, check=True)
        return target / "zephyr/zmk.exe"

    executable = build("normal", "native-storage:test-v1")
    alternate = build("other-layout", "native-storage:test-v2")
    results = []

    def trial(step, image, exe=executable):
        result = subprocess.run([str(exe), f"--flash={image}"], cwd=root,
                                env=dict(env, ANALYTICS_TEST_STEP=step),
                                text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
        output = result.stdout
        (root / f"{image.stem}.{step}.log").write_text(output)
        result.check_returncode()
        if f"PASS: storage {step}" not in output:
            raise AssertionError(f"storage {step}: missing success marker")
        print(f"PASS: storage {step}", flush=True)
        results.append(dict(step=step, flash=image.name, status="pass"))

    base = root / "base.flash"
    trial("save", base)
    trial("verify", base)
    # Verification and restoration do not implicitly save startup events.
    trial("verify", base)
    layout = root / "layout.flash"
    shutil.copyfile(base, layout)
    trial("layout", layout, alternate)
    damaged = root / "damaged.flash"
    shutil.copyfile(base, damaged)
    trial("corrupt", damaged)
    trial("recover", damaged)
    trial("corrupt-both", damaged)
    trial("blocked", damaged)
    (root / "results.json").write_text(json.dumps(results, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
