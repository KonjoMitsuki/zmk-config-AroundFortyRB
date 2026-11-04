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
    version = doc.get("version")
    if doc.get("protocol") != "zmk-analytics-poc" or type(version) is not int or version not in (1, 2):
        raise ValueError("not a compatible Analytics PoC response")
    if (doc.get("development_unprotected") is not True or type(doc.get("persistent")) is not bool
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
    if version == 1:
        if doc["persistent"]:
            raise ValueError("v1 does not support persistence")
    else:
        storage = doc.get("storage")
        if (not isinstance(storage, dict) or storage.get("state") not in
                ("disabled", "loading", "ok", "error", "no_valid_snapshot", "incompatible_layout")
                or not counter(storage.get("generation")) or not counter(storage.get("error_count_session"))
                or type(storage.get("last_error")) is not int or type(storage.get("dirty")) is not bool
                or type(storage.get("recovered")) is not bool):
            raise ValueError("invalid storage metadata")
        if doc["persistent"]:
            if (not isinstance(doc.get("physical_layout_id"), str) or not doc["physical_layout_id"]
                    or doc.get("layout_tracking") != "manual"):
                raise ValueError("missing layout identity")
    return doc


def read_response(connection):
    deadline = time.monotonic() + 10
    data = bytearray()
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
            return json.loads(data)
    raise ValueError("no complete response. Studio is a separate COM port; "
                     "the USB PoC image must expose TWO USB serial ports.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--list", action="store_true", help="list ports without opening them")
    parser.add_argument("--port", help="Analytics port, e.g. COM7; do not choose Studio")
    parser.add_argument("--out", type=Path, help="explicitly save snapshot as JSON")
    parser.add_argument("--save", action="store_true", help="save to keyboard flash before reading (v2)")
    args = parser.parse_args()
    if not args.list and not args.port:
        parser.error("specify --list or --port COMn")
    if args.save and (args.list or not args.port):
        parser.error("--save requires --port, not --list")
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
            if args.save:
                connection.write(b"\nSAVE\n")
                reply = read_response(connection)
                if not isinstance(reply, dict) or reply.get("saved") is not True or reply.get("error") != 0:
                    raise ValueError(f"save failed: {reply}; -11 means retry after 10 seconds or loading")
                connection.write(b"GET\n")
            else:
                connection.write(b"\nGET\n")
            doc = validate_snapshot(json.dumps(read_response(connection)))
        if args.out:
            # Do not overwrite an earlier trial by accident.
            with args.out.open("x", encoding="utf-8") as output:
                output.write(json.dumps(doc, indent=2) + "\n")
        print("No unlock; source-event counts, not confirmed physical input.")
        if doc["persistent"]:
            print("Storage:", json.dumps(doc["storage"]))
            print("Power loss can discard changes since the last successful save.")
        else:
            print("RAM only; statistics are lost on restart.")
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
