"""Host-only protocol validation; no real keyboard required."""
import importlib.util
import json
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location(
    "read_poc", Path(__file__).resolve().parents[1] / "tools/read_poc.py")
reader = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reader)


class SnapshotTests(unittest.TestCase):
    def document(self):
        return dict(protocol="zmk-analytics-poc", version=1, development_unprotected=True,
                    persistent=False, measurement="source_events", presses=[[0, 0]] * 42,
                    releases=[0, 0], invalid_positions=0, saturated=False)

    def test_valid(self):
        doc = self.document()
        doc["presses"][0] = [0xFFFFFFFF, 4]
        doc["saturated"] = True
        self.assertEqual(reader.validate_snapshot(json.dumps(doc)), doc)

    def test_metadata(self):
        for field, value in (("protocol", "studio"), ("version", 2), ("persistent", True),
                             ("development_unprotected", False), ("measurement", "physical")):
            with self.subTest(field=field):
                doc = self.document()
                doc[field] = value
                with self.assertRaises(ValueError):
                    reader.validate_snapshot(json.dumps(doc))

    def test_counters(self):
        for value in (-1, 0x100000000, True, "3", 1.5):
            doc = self.document()
            doc["presses"][0] = [value, 0]
            with self.subTest(value=value), self.assertRaises(ValueError):
                reader.validate_snapshot(json.dumps(doc))

    def test_incomplete_or_wrong_shape(self):
        for raw in ('{"protocol":', 'null', '[]', '{"error":"unknown_command"}'):
            with self.subTest(raw=raw), self.assertRaises(ValueError):
                reader.validate_snapshot(raw)
        for presses in ([], [[1]], [[0, 0]] * 257):
            doc = self.document()
            doc["presses"] = presses
            with self.assertRaises(ValueError):
                reader.validate_snapshot(json.dumps(doc))


if __name__ == "__main__":
    unittest.main()
