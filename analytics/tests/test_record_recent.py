"""Host-only tests for merging independent capture files into a SQLite snapshot."""

from concurrent.futures import ThreadPoolExecutor
from contextlib import closing
import json
from pathlib import Path
import shutil
import sqlite3
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import record_recent as recorder


def recent(first, last, timestamp_offset=0):
    events = [[sequence, timestamp_offset + 10 * sequence, sequence % 42, sequence % 2]
              for sequence in range(first, last + 1)]
    return {"protocol": "zmk-analytics-poc", "version": 3, "kind": "recent",
            "positions": 42, "capacity": 1024, "count": len(events),
            "next_sequence": last, "events": events}


class CaptureDatabaseTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.output = self.root / "analytics.sqlite3"

    def test_merges_overlapping_writes_and_duplicate_sync_copy(self):
        with ThreadPoolExecutor(max_workers=2) as workers:
            first, second = list(workers.map(
                lambda doc: recorder.save_capture(self.root, doc),
                [recent(1, 5), recent(4, 8)],
            ))
        self.assertNotEqual(first, second)
        shutil.copyfile(first, first.with_name("OneDrive conflict copy.json"))
        self.assertEqual(recorder.build_database(self.root, self.output), (2, 8, 1, 0))
        with closing(sqlite3.connect(self.output)) as db:
            self.assertEqual(db.execute("PRAGMA integrity_check").fetchone(), ("ok",))
            self.assertEqual(db.execute("SELECT sequence FROM presses ORDER BY sequence").fetchall(),
                             [(number,) for number in range(1, 9)])

    def test_retains_distinct_runs_and_reports_unobserved_prefix(self):
        recorder.save_capture(self.root, recent(7, 12))
        recorder.save_capture(self.root, recent(1, 9, timestamp_offset=3))
        self.assertEqual(recorder.build_database(self.root, self.output), (2, 15, 2, 1))
        with closing(sqlite3.connect(self.output)) as db:
            self.assertEqual(db.execute("SELECT start_reason FROM segments ORDER BY id").fetchall(),
                             [("first",), ("unlinked",)])
            self.assertEqual(db.execute("SELECT first_missing, last_missing FROM gaps").fetchall(),
                             [(1, 6)])

    def test_conflicting_sync_copy_does_not_replace_published_database(self):
        original = recorder.save_capture(self.root, recent(1, 5))
        recorder.build_database(self.root, self.output)
        before = self.output.read_bytes()
        copied = json.loads(original.read_text(encoding="utf-8"))
        copied["recent"]["events"][0][2] = 41
        original.with_name("conflict.json").write_text(json.dumps(copied), encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "conflicting copies"):
            recorder.build_database(self.root, self.output)
        self.assertEqual(self.output.read_bytes(), before)

    def test_publish_keeps_the_previous_snapshot_when_sync_is_incomplete(self):
        recorder.save_capture(self.root, recent(1, 5))
        unavailable = recorder.save_capture(self.root, recent(4, 8))
        recorder.build_database(self.root, self.output, publishing=True)
        before = self.output.read_bytes()
        unavailable.rename(self.root / unavailable.name)
        with self.assertRaisesRegex(ValueError, "records are missing"):
            recorder.build_database(self.root, self.output, publishing=True)
        self.assertEqual(self.output.read_bytes(), before)

    def test_dashboard_export_omits_transitions_across_gaps_and_segments(self):
        first = recent(1, 5)
        del first["events"][2]
        first["count"] = len(first["events"])
        recorder.save_capture(self.root, first)
        recorder.save_capture(self.root, recent(4, 6))
        recorder.save_capture(self.root, recent(1, 2, timestamp_offset=9))
        recorder.build_database(self.root, self.output)
        export = self.root / recorder.HISTORY_FILENAME
        recorder.export_dashboard(self.output, export)
        doc = json.loads(export.read_text(encoding="utf-8"))
        self.assertEqual(sum(map(sum, doc["presses"])), 7)
        self.assertEqual((doc["captures"], doc["segments"], doc["gaps"]), (3, 2, 1))
        self.assertEqual(doc["transitions"][1][2], 2)
        self.assertEqual(doc["transitions"][2][4], 0)
        self.assertEqual(doc["transitions"][6][1], 0)
        self.assertEqual(sum(map(sum, doc["transitions"])), 4)

    def test_layered_captures_fill_unknown_values_without_counting_twice(self):
        recorder.save_capture(self.root, recent(1, 3))
        recorder.save_capture(self.root, {**recent(2, 4), "layers": [0, 6, None]})
        self.assertEqual(recorder.build_database(self.root, self.output), (2, 4, 1, 0))
        with closing(sqlite3.connect(self.output)) as db:
            self.assertEqual(db.execute("SELECT layer_id FROM presses ORDER BY sequence").fetchall(),
                             [(None,), (0,), (6,), (None,)])
        export = self.root / recorder.HISTORY_FILENAME
        recorder.export_dashboard(self.output, export)
        doc = json.loads(export.read_text(encoding="utf-8"))
        self.assertEqual(doc["position_counts"], {"2": {"L0": 1}, "3": {"L6": 1}})
        self.assertEqual(doc["event_layers"], [None, 6, 0, None])


if __name__ == "__main__":
    unittest.main()
