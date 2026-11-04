"""Read a RAM snapshot from the dedicated Analytics PoC serial port (not Studio)."""
import argparse
import json
from pathlib import Path
import sys
import time


def validate_snapshot(raw):
    doc = json.loads(raw)
    if not isinstance(doc, dict):
        raise ValueError("expected a JSON object")
    if doc.get("protocol") != "zmk-analytics-poc" or doc.get("version") != 1:
        raise ValueError("not a compatible Analytics PoC response")
    if (doc.get("development_unprotected") is not True or doc.get("persistent") is not False
            or doc.get("measurement") != "source_events"):
        raise ValueError("unexpected measurement metadata")

    def counter(value):
        return type(value) is int and 0 <= value <= 0xFFFFFFFF

    presses = doc.get("presses")
    if not isinstance(presses, list) or not 1 <= len(presses) <= 256:
        raise ValueError("invalid position array")
    for pair in [*presses, doc.get("releases")]:
        if not isinstance(pair, list) or len(pair) != 2 or not all(map(counter, pair)):
            raise ValueError("invalid source counters")
    if not counter(doc.get("invalid_positions")) or type(doc.get("saturated")) is not bool:
        raise ValueError("invalid quality fields")
    return doc


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--list", action="store_true", help="list ports without opening them")
    parser.add_argument("--port", help="Analytics port, e.g. COM7; do not choose Studio")
    parser.add_argument("--out", type=Path, help="explicitly save snapshot as JSON")
    args = parser.parse_args()
    if not args.list and not args.port:
        parser.error("specify --list or --port COMn")
    try:
        import serial
        from serial.tools import list_ports
    except ImportError as error:
        parser.exit(1, f"Python: {sys.executable}\nImport failed: {error}\n"
                    f'Install for this Python: "{sys.executable}" -m pip install pyserial\n')

    if args.list:
        for port in list_ports.comports():
            print(f"{port.device}: {port.description} | interface={port.interface} | {port.hwid}")
        return 0
    try:
        with serial.Serial(args.port, 115200, timeout=0.2, write_timeout=2) as connection:
            connection.dtr = True
            time.sleep(0.4)
            connection.reset_input_buffer()
            # Leading newline terminates any incomplete command from an earlier open.
            connection.write(b"\nGET\n")
            deadline = time.monotonic() + 5
            data = bytearray()
            doc = None
            while time.monotonic() < deadline:
                byte = connection.read(1)
                if not byte:
                    continue
                data.extend(byte)
                if len(data) > 16384:
                    raise ValueError("response exceeds size limit")
                if byte == b"\n":
                    if data.strip() == b'{"error":"unknown_command"}':
                        data.clear()
                        continue
                    doc = validate_snapshot(data)
                    break
            if doc is None:
                raise ValueError("no complete snapshot. Studio is a separate COM port; "
                                 "the USB PoC image must expose TWO USB serial ports. "
                                 "If only one appears, check which firmware is running.")
        if args.out:
            # Do not overwrite an earlier trial by accident.
            with args.out.open("x", encoding="utf-8") as output:
                output.write(json.dumps(doc, indent=2) + "\n")
        print("RAM only; no unlock; source-event counts, not confirmed physical input.")
        print("position  right/local  left/peripheral")
        for position, (local, peripheral) in enumerate(doc["presses"]):
            print(f"{position:8} {local:12} {peripheral:16}")
        print(f"invalid_positions={doc['invalid_positions']} saturated={doc['saturated']}")
        return 0
    except (serial.SerialException, OSError, ValueError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
