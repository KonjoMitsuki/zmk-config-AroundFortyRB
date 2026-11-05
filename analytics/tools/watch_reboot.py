#!/usr/bin/env python3
"""Save a baseline, wait for the Analytics USB port to disappear and return,
then compare the restored counters with the baseline. Do not type on the
keyboard while this runs: keyboard input would change the counters."""
import argparse
import json
from pathlib import Path
import sys
import time

import serial
from serial.tools import list_ports

import read_poc


def get_snapshot(connection):
    connection.write(b"\nGET\n")
    return read_poc.validate_snapshot(json.dumps(read_poc.read_response(connection)))


def get_bigrams(connection):
    connection.write(b"\nBIGRAMS\n")
    return read_poc.validate_bigrams(json.dumps(read_poc.read_response(connection)))


def save_baseline(port):
    with serial.Serial(port, 115200, timeout=0.5, write_timeout=2) as connection:
        connection.dtr = True
        time.sleep(0.4)
        connection.reset_input_buffer()
        connection.write(b"\nSAVE\n")
        reply = read_poc.read_response(connection)
        if not isinstance(reply, dict) or reply.get("saved") is not True or reply.get("error") != 0:
            raise ValueError(f"save failed: {reply}; -11 means retry after 10 seconds")
        return get_snapshot(connection), get_bigrams(connection)


def wait_and_read(port, timeout):
    deadline = time.monotonic() + timeout
    print(f"WAITING_FOR_DISCONNECT {port}", flush=True)
    down = False
    while time.monotonic() < deadline:
        present = port in {item.device for item in list_ports.comports()}
        if not present:
            down = True
        elif down:
            try:
                with serial.Serial(port, 115200, timeout=0.5, write_timeout=2) as connection:
                    connection.dtr = True
                    time.sleep(1.0)
                    connection.reset_input_buffer()
                    return get_snapshot(connection), get_bigrams(connection)
            except (serial.SerialException, OSError, ValueError):
                pass
        time.sleep(0.2)
    raise TimeoutError("port did not disconnect and return; power-cycle the keyboard")


def same(a, b):
    fields = ("presses", "releases", "invalid_positions", "saturated", "quality", "timing")
    return all(a.get(field) == b.get(field) for field in fields)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="Analytics COM port, e.g. COM7")
    parser.add_argument("--timeout", type=int, default=600, help="seconds to wait for the reboot")
    parser.add_argument("--out", type=Path, help="write baseline and restored JSON here")
    args = parser.parse_args()

    baseline, baseline_bigrams = save_baseline(args.port)
    print("BASELINE " + json.dumps(baseline["storage"], sort_keys=True), flush=True)
    print(f"position 0: right/local={baseline['presses'][0][0]} "
          f"left/peripheral={baseline['presses'][0][1]}", flush=True)
    restored, restored_bigrams = wait_and_read(args.port, args.timeout)
    print("RESTORED " + json.dumps(restored["storage"], sort_keys=True), flush=True)
    print(f"position 0: right/local={restored['presses'][0][0]} "
          f"left/peripheral={restored['presses'][0][1]}", flush=True)

    if args.out:
        args.out.write_text(json.dumps({"baseline": baseline, "restored": restored,
                                        "baseline_bigrams": baseline_bigrams,
                                        "restored_bigrams": restored_bigrams}, indent=2) + "\n",
                            encoding="utf-8")
    if not restored["persistent"] or restored["storage"]["state"] != "ok":
        print("FAIL: persistence is not active after reboot", file=sys.stderr)
        return 1
    if restored["storage"]["generation"] != baseline["storage"]["generation"]:
        print("FAIL: generation changed", file=sys.stderr)
        return 1
    if restored["storage"]["dirty"]:
        print("FAIL: restored counters differ from the saved generation "
              "(keyboard input during the test?)", file=sys.stderr)
        return 1
    if not same(baseline, restored):
        print("FAIL: statistics differ", file=sys.stderr)
        return 1
    if (baseline_bigrams["counts"] != restored_bigrams["counts"]
            or baseline_bigrams["saturated"] != restored_bigrams["saturated"]):
        print("FAIL: directed bigrams differ", file=sys.stderr)
        return 1
    print("PASS: statistics and bigrams restored exactly at the same generation", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
