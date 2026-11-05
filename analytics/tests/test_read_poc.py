"""Host-only protocol validation; no real keyboard required."""
import importlib.util
import json
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location(
    "read_poc", Path(__file__).resolve().parents[1] / "tools/read_poc.py")
reader = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reader)


def zeros(rows, columns):
    return [[0] * columns for _ in range(rows)]


class SnapshotTests(unittest.TestCase):
    def v1_document(self):
        return dict(protocol="zmk-analytics-poc", version=1, development_unprotected=True,
                    persistent=False, measurement="source_events", presses=[[0, 0]] * 42,
                    releases=[0, 0], invalid_positions=0, saturated=False)

    def v2_document(self):
        doc = self.v1_document()
        doc.update(version=2, persistent=True, physical_layout_id="test:1",
                   layout_tracking="manual",
                   storage=dict(state="ok", generation=2, error_count_session=0, last_error=0,
                                dirty=False, recovered=False))
        return doc

    def v3_document(self):
        doc = self.v2_document()
        doc.update(
            version=3,
            quality=dict(duplicate_downs=1, orphan_releases=2, timing_discards=3,
                         pair_config_errors=4, histogram_saturated=False),
            timing=dict(
                hold_bins_ms=reader.HOLD_BINS,
                interval_bins_ms=reader.INTERVAL_BINS,
                pair_bins_ms=reader.PAIR_BINS,
                hold=zeros(42, 8),
                same_key=zeros(42, 8),
                idle_before_press=zeros(42, 8),
                pairs=[dict(positions=[0, 1], near=zeros(3, 6), overlap=zeros(3, 6))],
            ),
        )
        return doc

    def test_valid_v1(self):
        doc = self.v1_document()
        doc["presses"][0] = [0xFFFFFFFF, 4]
        doc["saturated"] = True
        self.assertEqual(reader.validate_snapshot(json.dumps(doc)), doc)

    def test_metadata(self):
        for field, value in (("protocol", "studio"), ("version", 4), ("persistent", True),
                             ("development_unprotected", False), ("measurement", "physical")):
            with self.subTest(field=field):
                doc = self.v1_document()
                doc[field] = value
                with self.assertRaises(ValueError):
                    reader.validate_snapshot(json.dumps(doc))

    def test_counters(self):
        for value in (-1, 0x100000000, True, "3", 1.5):
            doc = self.v1_document()
            doc["presses"][0] = [value, 0]
            with self.subTest(value=value), self.assertRaises(ValueError):
                reader.validate_snapshot(json.dumps(doc))

    def test_incomplete_or_wrong_shape(self):
        for raw in ('{"protocol":', 'null', '[]', '{"error":"unknown_command"}'):
            with self.subTest(raw=raw), self.assertRaises(ValueError):
                reader.validate_snapshot(raw)
        for presses in ([], [[1]], [[0, 0]] * 257):
            doc = self.v1_document()
            doc["presses"] = presses
            with self.assertRaises(ValueError):
                reader.validate_snapshot(json.dumps(doc))

    def test_persistent_v2(self):
        doc = self.v2_document()
        self.assertEqual(reader.validate_snapshot(json.dumps(doc)), doc)
        for field, value in (("generation", -1), ("dirty", 1), ("state", "made_up")):
            bad = {**doc, "storage": {**doc["storage"], field: value}}
            with self.subTest(field=field), self.assertRaises(ValueError):
                reader.validate_snapshot(json.dumps(bad))

    def test_v3_timing(self):
        doc = self.v3_document()
        self.assertEqual(reader.validate_snapshot(json.dumps(doc)), doc)
        mutations = [
            ("timing", {**doc["timing"], "hold_bins_ms": [1]}),
            ("timing", {**doc["timing"], "same_key": zeros(41, 8)}),
            ("timing", {**doc["timing"], "hold": zeros(42, 7)}),
            ("timing", {**doc["timing"], "pairs": [
                dict(positions=[1, 1], near=zeros(3, 6), overlap=zeros(3, 6))]}),
            ("timing", {**doc["timing"], "pairs": [
                dict(positions=[0, 1], near=zeros(2, 6), overlap=zeros(3, 6))]}),
            ("quality", {**doc["quality"], "duplicate_downs": -1}),
            ("quality", {**doc["quality"], "histogram_saturated": 1}),
        ]
        for field, value in mutations:
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                reader.validate_snapshot(json.dumps({**doc, field: value}))

    def test_bigrams(self):
        doc = dict(protocol="zmk-analytics-poc", version=3, kind="bigrams", positions=42,
                   saturated=False, counts=zeros(42, 42))
        doc["counts"][0][1] = 7
        self.assertEqual(reader.validate_bigrams(json.dumps(doc)), doc)
        for field, value in (("positions", 0), ("saturated", 1), ("counts", zeros(41, 42))):
            with self.subTest(field=field), self.assertRaises(ValueError):
                reader.validate_bigrams(json.dumps({**doc, field: value}))
        bad = {**doc, "counts": zeros(42, 42)}
        bad["counts"][0][0] = 65536
        with self.assertRaises(ValueError):
            reader.validate_bigrams(json.dumps(bad))

    def test_recent(self):
        doc = dict(protocol="zmk-analytics-poc", version=3, kind="recent", positions=42,
                   capacity=1024, next_sequence=3, count=2,
                   events=[[1, 100, 0, 0], [3, 120, 1, 1]])
        self.assertEqual(reader.validate_recent(json.dumps(doc)), doc)
        mutations = [
            ("count", 3),
            ("events", [[2, 100, 0, 0], [1, 120, 1, 1]]),
            ("events", [[1, 100, 42, 0], [3, 120, 1, 1]]),
            ("events", [[1, 100, 0, 2], [3, 120, 1, 1]]),
            ("next_sequence", -1),
        ]
        for field, value in mutations:
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                reader.validate_recent(json.dumps({**doc, field: value}))

    def test_stream_response(self):
        import io
        reply = reader.read_response(io.BytesIO(b'{"error":"unknown_command"}\n{"saved":true,"error":0}\n'))
        self.assertEqual(reply, dict(saved=True, error=0))
        with self.assertRaises(ValueError):
            reader.read_response(io.BytesIO(b'x' * 65537))


if __name__ == "__main__":
    unittest.main()
