"""Read RAM snapshots from the dedicated Analytics PoC serial port (not Studio)."""
import argparse
import json
from pathlib import Path
import sys
import time

HOLD_BINS = [20, 40, 60, 80, 100, 200, 500]
INTERVAL_BINS = [50, 100, 150, 200, 300, 500, 1000]
PAIR_BINS = [20, 40, 60, 80, 100]
PAIR_CLASSES = ["CC", "PP", "CP"]


def _counter(value):
    return type(value) is int and 0 <= value <= 0xFFFFFFFF


def _counter16(value):
    return type(value) is int and 0 <= value <= 0xFFFF


def _validate_timing(timing, position_count):
    if not isinstance(timing, dict):
        raise ValueError("invalid timing metadata")
    if (timing.get("hold_bins_ms") != HOLD_BINS
            or timing.get("interval_bins_ms") != INTERVAL_BINS
            or timing.get("pair_bins_ms") != PAIR_BINS):
        raise ValueError("unexpected timing bin boundaries")
    for name, width in (("hold", 8), ("same_key", 8), ("idle_before_press", 8)):
        rows = timing.get(name)
        if not isinstance(rows, list) or len(rows) != position_count:
            raise ValueError(f"invalid timing.{name}")
        for row in rows:
            if (not isinstance(row, list) or len(row) != width
                    or not all(map(_counter16, row))):
                raise ValueError(f"invalid timing.{name} row")
    pairs = timing.get("pairs")
    if not isinstance(pairs, list) or len(pairs) > 16:
        raise ValueError("invalid timing.pairs")
    for pair in pairs:
        if not isinstance(pair, dict):
            raise ValueError("invalid pair entry")
        positions = pair.get("positions")
        if (not isinstance(positions, list) or len(positions) != 2
                or not all(type(p) is int and 0 <= p < position_count for p in positions)
                or positions[0] == positions[1]):
            raise ValueError("invalid pair positions")
        for name in ("near", "overlap"):
            rows = pair.get(name)
            if not isinstance(rows, list) or len(rows) != len(PAIR_CLASSES):
                raise ValueError(f"invalid pair.{name}")
            for row in rows:
                if (not isinstance(row, list) or len(row) != len(PAIR_BINS) + 1
                        or not all(map(_counter16, row))):
                    raise ValueError(f"invalid pair.{name} row")


def validate_snapshot(raw):
    doc = json.loads(raw)
    if not isinstance(doc, dict):
        raise ValueError("expected a JSON object")
    version = doc.get("version")
    if doc.get("protocol") != "zmk-analytics-poc" or type(version) is not int or version not in (1, 2, 3):
        raise ValueError("not a compatible Analytics PoC response")
    if (doc.get("development_unprotected") is not True or type(doc.get("persistent")) is not bool
            or doc.get("measurement") != "source_events"):
        raise ValueError("unexpected measurement metadata")

    presses = doc.get("presses")
    if not isinstance(presses, list) or not 1 <= len(presses) <= 256:
        raise ValueError("invalid position array")
    for pair in [*presses, doc.get("releases")]:
        if not isinstance(pair, list) or len(pair) != 2 or not all(map(_counter, pair)):
            raise ValueError("invalid source counters")
    if not _counter(doc.get("invalid_positions")) or type(doc.get("saturated")) is not bool:
        raise ValueError("invalid quality fields")
    if version == 1:
        if doc["persistent"]:
            raise ValueError("v1 does not support persistence")
    else:
        storage = doc.get("storage")
        if (not isinstance(storage, dict) or storage.get("state") not in
                ("disabled", "loading", "ok", "error", "no_valid_snapshot", "incompatible_layout")
                or not _counter(storage.get("generation")) or not _counter(storage.get("error_count_session"))
                or type(storage.get("last_error")) is not int or type(storage.get("dirty")) is not bool
                or type(storage.get("recovered")) is not bool):
            raise ValueError("invalid storage metadata")
        if doc["persistent"]:
            if (not isinstance(doc.get("physical_layout_id"), str) or not doc["physical_layout_id"]
                    or doc.get("layout_tracking") != "manual"):
                raise ValueError("missing layout identity")
    if version == 3:
        quality = doc.get("quality")
        if not isinstance(quality, dict):
            raise ValueError("missing v3 quality fields")
        for name in ("duplicate_downs", "orphan_releases", "timing_discards", "pair_config_errors"):
            if not _counter(quality.get(name)):
                raise ValueError(f"invalid quality.{name}")
        if type(quality.get("histogram_saturated")) is not bool:
            raise ValueError("invalid quality.histogram_saturated")
        _validate_timing(doc.get("timing"), len(presses))
    return doc


def validate_bigrams(raw):
    doc = json.loads(raw)
    if (not isinstance(doc, dict) or doc.get("protocol") != "zmk-analytics-poc"
            or doc.get("version") != 3 or doc.get("kind") != "bigrams"):
        raise ValueError("not a compatible bigram response")
    positions = doc.get("positions")
    if not type(positions) is int or not 1 <= positions <= 256:
        raise ValueError("invalid position count")
    if type(doc.get("saturated")) is not bool:
        raise ValueError("invalid bigram saturation flag")
    counts = doc.get("counts")
    if not isinstance(counts, list) or len(counts) != positions:
        raise ValueError("invalid bigram matrix")
    for row in counts:
        if (not isinstance(row, list) or len(row) != positions
                or not all(map(_counter16, row))):
            raise ValueError("invalid bigram row")
    return doc


def validate_recent(raw):
    doc = json.loads(raw)
    if (not isinstance(doc, dict) or doc.get("protocol") != "zmk-analytics-poc"
            or doc.get("version") != 3 or doc.get("kind") != "recent"):
        raise ValueError("not a compatible recent-order response")
    positions = doc.get("positions")
    capacity = doc.get("capacity")
    count = doc.get("count")
    if (not type(positions) is int or not 1 <= positions <= 256
            or not type(capacity) is int or not 1 <= capacity <= 65535
            or not type(count) is int or not 0 <= count <= capacity
            or not _counter(doc.get("next_sequence"))):
        raise ValueError("invalid recent metadata")
    events = doc.get("events")
    if not isinstance(events, list) or len(events) != count:
        raise ValueError("invalid recent event count")
    previous = 0
    for event in events:
        if not isinstance(event, list) or len(event) != 4:
            raise ValueError("invalid recent event")
        sequence, timestamp, position, source = event
        if (not _counter(sequence) or not _counter(timestamp) or not type(position) is int
                or not 0 <= position < positions or source not in (0, 1)
                or sequence <= previous):
            raise ValueError("invalid recent event fields")
        previous = sequence
    return doc


def read_response(connection):
    deadline = time.monotonic() + 30
    data = bytearray()
    while time.monotonic() < deadline:
        byte = connection.read(1)
        if not byte:
            continue
        data.extend(byte)
        if len(data) > 65536:
            raise ValueError("response exceeds size limit")
        if byte == b"\n":
            if data.strip() == b'{"error":"unknown_command"}':
                data.clear()
                continue
            return json.loads(data)
    raise ValueError("no complete response. Studio is a separate COM port; "
                     "the USB PoC image must expose TWO USB serial ports.")


def _labels(bounds):
    return [f"<{bounds[0]}"] + [f"{bounds[i]}-{bounds[i + 1] - 1}"
                                for i in range(len(bounds) - 1)] + [f">={bounds[-1]}"]


def _column_sums(rows):
    return [sum(row[i] for row in rows) for i in range(len(rows[0]))]


def print_timing(doc):
    timing = doc["timing"]
    quality = doc["quality"]
    print(f"duplicate_downs={quality['duplicate_downs']} "
          f"orphan_releases={quality['orphan_releases']} "
          f"timing_discards={quality['timing_discards']} "
          f"pair_config_errors={quality['pair_config_errors']} "
          f"histogram_saturated={quality['histogram_saturated']}")
    for name, bounds in (("hold", HOLD_BINS), ("same_key", INTERVAL_BINS),
                         ("idle_before_press", INTERVAL_BINS)):
        print(f"{name} (all positions, ms bins {_labels(bounds)}): "
              f"{_column_sums(timing[name])}")
    for pair in timing["pairs"]:
        print(f"pair {pair['positions']} near "
              f"{ {name: _column_sums(pair['near'][i]) for i, name in enumerate(PAIR_CLASSES)} }")
        print(f"pair {pair['positions']} overlap "
              f"{ {name: _column_sums(pair['overlap'][i]) for i, name in enumerate(PAIR_CLASSES)} }")


def print_bigrams(doc):
    counts = doc["counts"]
    edges = []
    for a, row in enumerate(counts):
        row_total = sum(row)
        for b, count in enumerate(row):
            if count:
                edges.append((count, a, b, 100.0 * count / row_total if row_total else 0.0))
    edges.sort(reverse=True)
    print("from -> to  count  row%")
    for count, a, b, percentage in edges[:30]:
        print(f"{a:4} -> {b:<4} {count:6}  {percentage:5.1f}")
    if not edges:
        print("(no transitions yet)")
    print(f"saturated={doc['saturated']} positions={doc['positions']}")


def print_recent(doc, limit):
    events = doc["events"][-limit:]
    print("sequence  time_ms  position  source")
    for sequence, timestamp, position, source in events:
        print(f"{sequence:8} {timestamp:9} {position:8}  "
              f"{'local' if source == 0 else 'peripheral'}")
    if not events:
        print("(no press events yet)")
    print(f"retained={doc['count']} capacity={doc['capacity']}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--list", action="store_true", help="list ports without opening them")
    parser.add_argument("--port", help="Analytics port, e.g. COM7; do not choose Studio")
    parser.add_argument("--out", type=Path, help="explicitly save the fetched document as JSON")
    parser.add_argument("--save", action="store_true", help="save counters to keyboard flash before GET")
    parser.add_argument("--bigrams", action="store_true",
                        help="fetch all directed consecutive-press counts")
    parser.add_argument("--recent", action="store_true",
                        help="fetch the RAM-only recent press order")
    parser.add_argument("--recent-limit", type=int, default=50,
                        help="events to print for --recent (default 50)")
    args = parser.parse_args()
    if not args.list and not args.port:
        parser.error("specify --list or --port COMn")
    if args.save and (args.list or not args.port or args.bigrams or args.recent):
        parser.error("--save requires --port and cannot be combined with --bigrams/--recent")
    if args.bigrams and args.recent:
        parser.error("choose --bigrams or --recent")
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
                doc = validate_snapshot(json.dumps(read_response(connection)))
            elif args.bigrams:
                connection.write(b"\nBIGRAMS\n")
                doc = validate_bigrams(json.dumps(read_response(connection)))
            elif args.recent:
                connection.write(b"\nRECENT\n")
                doc = validate_recent(json.dumps(read_response(connection)))
            else:
                connection.write(b"\nGET\n")
                doc = validate_snapshot(json.dumps(read_response(connection)))
        if args.out:
            # Do not overwrite an earlier trial by accident.
            with args.out.open("x", encoding="utf-8") as output:
                output.write(json.dumps(doc, indent=2) + "\n")
        if args.bigrams:
            print_bigrams(doc)
            return 0
        if args.recent:
            print_recent(doc, args.recent_limit)
            return 0
        print("No unlock; source-event counts and arrival-time histograms, "
              "not confirmed physical timing.")
        if doc["persistent"]:
            print("Storage:", json.dumps(doc["storage"]))
            print("Power loss can discard changes since the last successful save.")
        else:
            print("RAM only; statistics are lost on restart.")
        print("position  right/local  left/peripheral")
        for position, (local, peripheral) in enumerate(doc["presses"]):
            print(f"{position:8} {local:12} {peripheral:16}")
        print(f"invalid_positions={doc['invalid_positions']} saturated={doc['saturated']}")
        if doc["version"] >= 3:
            print_timing(doc)
        return 0
    except (serial.SerialException, OSError, ValueError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
