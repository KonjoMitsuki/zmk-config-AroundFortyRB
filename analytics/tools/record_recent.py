#!/usr/bin/env python3
"""Capture AroundForty RB presses into OneDrive and build a SQLite snapshot."""

import argparse
from contextlib import closing, contextmanager
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import sqlite3
import sys
import tempfile
import time
from uuid import uuid4

from read_poc import read_response, validate_recent

CAPTURE_FORMAT = "aroundforty-recent-capture"
HISTORY_FILENAME = "analytics-dashboard.json"
SCHEMA = """
PRAGMA foreign_keys = ON;
CREATE TABLE segments (
    id INTEGER PRIMARY KEY,
    start_reason TEXT NOT NULL CHECK (start_reason IN ('first', 'unlinked', 'ambiguous'))
);
CREATE TABLE captures (
    id TEXT PRIMARY KEY,
    captured_at TEXT NOT NULL,
    segment_id INTEGER REFERENCES segments(id),
    event_count INTEGER NOT NULL,
    newest_sequence INTEGER NOT NULL,
    sha256 TEXT NOT NULL
);
CREATE TABLE presses (
    segment_id INTEGER NOT NULL REFERENCES segments(id),
    sequence INTEGER NOT NULL,
    uptime_ms INTEGER NOT NULL,
    position INTEGER NOT NULL,
    source INTEGER NOT NULL CHECK (source IN (0, 1)),
    layer_id INTEGER CHECK (layer_id BETWEEN 0 AND 254),
    capture_id TEXT NOT NULL REFERENCES captures(id),
    PRIMARY KEY (segment_id, sequence)
);
CREATE INDEX presses_fingerprint ON presses(sequence, uptime_ms, position, source);
CREATE VIEW gaps AS
SELECT segment_id, previous + 1 AS first_missing, sequence - 1 AS last_missing,
       sequence - previous - 1 AS missing_count
FROM (
    SELECT segment_id, sequence,
           lag(sequence, 1, 0) OVER (PARTITION BY segment_id ORDER BY sequence) AS previous
    FROM presses
)
WHERE sequence > previous + 1;
CREATE TEMP TABLE incoming (
    sequence INTEGER PRIMARY KEY, uptime_ms INTEGER NOT NULL,
    position INTEGER NOT NULL, source INTEGER NOT NULL, layer_id INTEGER
);
"""


def read_recent(port):
    import serial

    with serial.Serial(port, 115200, timeout=0.2, write_timeout=2) as connection:
        connection.dtr = True
        time.sleep(0.4)
        connection.reset_input_buffer()
        connection.write(b"\nRECENT\n")
        return validate_recent(json.dumps(read_response(connection)))


def default_directory():
    for variable in ("OneDrive", "OneDriveConsumer", "OneDriveCommercial"):
        root = os.environ.get(variable)
        if root and Path(root).is_dir():
            return Path(root) / "AroundFortyAnalytics"
    raise ValueError("OneDrive folder not found. Pass --directory with its full path.")


@contextmanager
def atomic_output(path):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temp_name = tempfile.mkstemp(prefix="aroundforty-", suffix=".tmp", dir=path.parent)
    temp = Path(temp_name)
    try:
        with os.fdopen(fd, "wb") as handle:
            yield handle
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp, path)
    finally:
        temp.unlink(missing_ok=True)


def save_capture(directory, recent):
    now = datetime.now(timezone.utc)
    capture_id = uuid4().hex
    doc = {
        "format": CAPTURE_FORMAT,
        "version": 1,
        "id": capture_id,
        "captured_at": now.isoformat(timespec="microseconds"),
        "recent": recent,
    }
    stamp = now.strftime("%Y%m%dT%H%M%S%fZ")
    path = directory / "captures" / f"{stamp}-{capture_id}.json"
    with atomic_output(path) as handle:
        handle.write(json.dumps(doc, separators=(",", ":")).encode("utf-8") + b"\n")
    return path


def load_capture(path):
    with path.open("rb") as handle:
        raw = handle.read(128 * 1024 + 1)
    if len(raw) > 128 * 1024:
        raise ValueError(f"{path.name}: capture is too large")
    try:
        doc = json.loads(raw)
        if (not isinstance(doc, dict) or doc.get("format") != CAPTURE_FORMAT
                or type(doc.get("version")) is not int or doc["version"] != 1
                or not isinstance(doc.get("id"), str) or not re.fullmatch("[a-f0-9]{32}", doc["id"])):
            raise ValueError("invalid capture header")
        timestamp = datetime.fromisoformat(doc["captured_at"])
        if timestamp.utcoffset() != timezone.utc.utcoffset(None):
            raise ValueError("capture time must be UTC")
        recent = validate_recent(json.dumps(doc["recent"]))
        if recent["positions"] != 42:
            raise ValueError("expected the AroundForty RB 42-position layout")
    except (KeyError, TypeError, ValueError) as error:
        raise ValueError(f"{path.name}: {error}") from error
    return doc["id"], timestamp.isoformat(timespec="microseconds"), recent, hashlib.sha256(raw).hexdigest()


def match_segment(db):
    # Require at least one matching event and reject any conflicting shared sequence.
    candidates = db.execute("""
        WITH candidates AS (
            SELECT DISTINCT p.segment_id
            FROM incoming AS i
            JOIN presses AS p USING (sequence, uptime_ms, position, source)
        )
        SELECT candidate.segment_id
        FROM candidates AS candidate
        WHERE NOT EXISTS (
            SELECT 1 FROM incoming AS other
            JOIN presses AS old ON old.segment_id = candidate.segment_id AND old.sequence = other.sequence
            WHERE (old.uptime_ms, old.position, old.source) !=
                  (other.uptime_ms, other.position, other.source)
               OR (old.layer_id IS NOT NULL AND other.layer_id IS NOT NULL
                   AND old.layer_id != other.layer_id)
        )
        LIMIT 2
    """).fetchall()
    if len(candidates) == 1:
        return candidates[0][0]
    has_segments = db.execute("SELECT 1 FROM segments LIMIT 1").fetchone()
    reason = "ambiguous" if candidates else "unlinked" if has_segments else "first"
    return db.execute("INSERT INTO segments(start_reason) VALUES (?)", (reason,)).lastrowid


def ingest(db, capture):
    capture_id, captured_at, recent, digest = capture
    previous = db.execute("SELECT sha256 FROM captures WHERE id = ?", (capture_id,)).fetchone()
    if previous:
        if previous[0] != digest:
            raise ValueError(f"conflicting copies of capture {capture_id}")
        return

    events = recent["events"]
    segment_id = None
    if events:
        db.execute("DELETE FROM incoming")
        layers = recent.get("layers", [None] * len(events))
        db.executemany("INSERT INTO incoming VALUES (?, ?, ?, ?, ?)",
                       ((*event, layer) for event, layer in zip(events, layers)))
        segment_id = match_segment(db)
    db.execute("INSERT INTO captures VALUES (?, ?, ?, ?, ?, ?)", (
        capture_id, captured_at, segment_id, len(events), recent["next_sequence"], digest,
    ))
    if events:
        db.execute("""
            INSERT INTO presses (segment_id, sequence, uptime_ms, position, source, layer_id, capture_id)
            SELECT ?, sequence, uptime_ms, position, source, layer_id, ? FROM incoming WHERE true
            ON CONFLICT(segment_id, sequence) DO UPDATE SET layer_id = excluded.layer_id
            WHERE presses.layer_id IS NULL AND excluded.layer_id IS NOT NULL
        """, (segment_id, capture_id))


def check_previous_publish(db, output):
    if not output.exists():
        return
    with closing(sqlite3.connect(output.resolve().as_uri() + "?mode=ro", uri=True)) as previous:
        present = dict(db.execute("SELECT id, sha256 FROM captures"))
        for capture_id, digest in previous.execute("SELECT id, sha256 FROM captures"):
            if present.get(capture_id) != digest:
                raise ValueError("Previously published records are missing or changed. "
                                 "Check OneDrive sync before publishing again.")


def build_database(directory, output, *, publishing=False):
    capture_directory = directory / "captures"
    if capture_directory.resolve() in output.resolve().parents:
        raise ValueError("Choose an output path outside the captures folder.")
    paths = sorted(capture_directory.glob("*.json"))
    if not paths:
        raise ValueError(f"No captures found under {capture_directory}.")
    # Build outside the synced folder and publish only the closed, standalone database.
    with tempfile.TemporaryDirectory(prefix="aroundforty-db-") as scratch:
        generated = Path(scratch) / "analytics.sqlite3"
        with closing(sqlite3.connect(generated)) as db:
            db.executescript(SCHEMA)
            with db:
                for path in paths:
                    ingest(db, load_capture(path))
            if publishing:
                check_previous_publish(db, output)
            summary = tuple(db.execute(f"SELECT count(*) FROM {table}").fetchone()[0]
                            for table in ("captures", "presses", "segments", "gaps"))
        with generated.open("rb") as source, atomic_output(output) as destination:
            shutil.copyfileobj(source, destination)
    return summary


def export_dashboard(database, output):
    with closing(sqlite3.connect(database.resolve().as_uri() + "?mode=ro", uri=True)) as db:
        # Keep all query results on the same database snapshot.
        db.execute("BEGIN")
        presses = [[0, 0] for _ in range(42)]
        for position, source, count in db.execute("""
            SELECT position, source, count(*) FROM presses GROUP BY position, source
        """):
            presses[position][source] = count
        transitions = [[0] * 42 for _ in range(42)]
        for origin, target, count in db.execute("""
            WITH ordered AS (
                SELECT position, sequence,
                       lag(position) OVER history AS origin,
                       lag(sequence) OVER history AS previous
                FROM presses
                WINDOW history AS (PARTITION BY segment_id ORDER BY sequence)
            )
            SELECT origin, position, count(*) FROM ordered
            WHERE previous + 1 = sequence GROUP BY origin, position
        """):
            transitions[origin][target] = count
        captures, first, last = db.execute("""
            SELECT count(*), min(captured_at), max(captured_at) FROM captures
        """).fetchone()
        segments = db.execute("SELECT count(*) FROM segments").fetchone()[0]
        gaps = db.execute("SELECT count(*) FROM gaps").fetchone()[0]
        position_counts = {}
        has_layers = any(column[1] == "layer_id" for column in db.execute("PRAGMA table_info(presses)"))
        if has_layers:
            for position, layer, count in db.execute("""
                SELECT position, layer_id, count(*) FROM presses WHERE layer_id IS NOT NULL
                GROUP BY position, layer_id
            """):
                position_counts.setdefault(position, {})[f"L{layer}"] = count
        layer_column = "presses.layer_id" if has_layers else "NULL"
        latest = db.execute(f"""
            SELECT captures.captured_at, presses.position, presses.source, {layer_column}
            FROM presses JOIN captures ON captures.id = presses.capture_id
            ORDER BY captures.captured_at DESC, presses.capture_id DESC, presses.sequence DESC
            LIMIT 50
        """).fetchall()
    doc = {
        "format": "aroundforty-analytics-history",
        "version": 1,
        "physical_layout_id": "aroundforty-rb:matrix-v1",
        "first_capture": first,
        "last_capture": last,
        "captures": captures,
        "segments": segments,
        "gaps": gaps,
        "presses": presses,
        "transitions": transitions,
        "events": [event[:3] for event in latest],
        "event_layers": [event[3] for event in latest],
        "position_counts": position_counts,
    }
    with atomic_output(output) as handle:
        handle.write(json.dumps(doc, separators=(",", ":")).encode("utf-8") + b"\n")


def port_file():
    root = Path(os.environ.get("LOCALAPPDATA", Path.home() / ".config"))
    return root / "AroundFortyAnalytics" / "port.txt"


def capture(args, directory):
    import serial

    port = args.port
    if not port and args.interactive:
        if port_file().is_file():
            port = port_file().read_text(encoding="utf-8").strip()
        if not port:
            port = input("Analytics COM port (e.g. COM7): ").strip()
    if not port:
        raise ValueError("Specify your Analytics port with --port COM7.")
    previous = None
    remember = args.interactive
    while True:
        started = time.monotonic()
        try:
            recent = read_recent(port)
            if recent["positions"] != 42:
                raise ValueError("expected the AroundForty RB 42-position layout")
            if recent != previous:
                path = save_capture(directory, recent)
                print(f"Saved {recent['count']} retained presses: {path}", flush=True)
                previous = recent
            if remember:
                with atomic_output(port_file()) as handle:
                    handle.write(port.encode("utf-8") + b"\n")
                remember = False
        except (OSError, ValueError, serial.SerialException) as error:
            if args.interval is None:
                raise ValueError(f"{port}: {error}") from error
            print(f"{port}: {error} (retrying)", file=sys.stderr, flush=True)
        if args.interval is None:
            return
        time.sleep(max(0, args.interval - (time.monotonic() - started)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    collect = sub.add_parser("capture", help="save one USB capture, or repeat with --interval")
    collect.add_argument("--port", help="Analytics port, e.g. COM7")
    collect.add_argument("--interval", type=int, metavar="SECONDS")
    collect.add_argument("--interactive", action="store_true", help="prompt for and remember a port on this PC")
    build = sub.add_parser("build", help="rebuild a closed SQLite snapshot from synced captures")
    target = build.add_mutually_exclusive_group(required=True)
    target.add_argument("--publish", action="store_true", help="replace analytics.sqlite3 in OneDrive (use one PC)")
    target.add_argument("--output", type=Path, help="build a separate SQLite file")
    export = sub.add_parser("export", help="export web dashboard JSON from an existing analytics.sqlite3")
    for command in (collect, build, export):
        command.add_argument("--directory", type=Path, help="shared capture folder (default OneDrive/AroundFortyAnalytics)")
    args = parser.parse_args()
    if args.command == "capture" and args.interval is not None and args.interval < 1:
        parser.error("--interval must be at least 1 second")
    try:
        directory = args.directory or default_directory()
        if args.command == "capture":
            capture(args, directory)
        elif args.command == "build":
            output = directory / "analytics.sqlite3" if args.publish else args.output
            captures, presses, segments, gaps = build_database(directory, output, publishing=args.publish)
            print(f"{output}: {captures} captures, {presses} presses, {segments} segments, {gaps} gap ranges.")
        if args.command == "export" or (args.command == "build" and args.publish):
            dashboard = directory / HISTORY_FILENAME
            export_dashboard(directory / "analytics.sqlite3", dashboard)
            print(f"Open {dashboard} in the web dashboard.")
    except ModuleNotFoundError as error:
        if error.name != "serial":
            raise
        parser.exit(1, "Install pyserial first: py -3 -m pip install pyserial\n")
    except (OSError, ValueError, sqlite3.Error) as error:
        parser.exit(1, f"{error}\n")
    except KeyboardInterrupt:
        print("\nStopped.")


if __name__ == "__main__":
    main()
