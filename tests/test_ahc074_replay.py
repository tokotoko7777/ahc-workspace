"""Independent protocol checking for visible sheets and post-removal depths."""
from pathlib import Path
import runpy
import unittest

SOURCE=Path(__file__).resolve().parents[1]/"problems/ahc074/benchmark.py"
replay=runpy.run_path(str(SOURCE))["replay"]


class BarcodeReplayTests(unittest.TestCase):
    def test_sheet_can_be_visible_at_one_right_anchor_only(self):
        data=b"3 3 3 3\n000\n101\n001\n2 1 0\n"
        out=b"0..\n1.1\n..0\n0\n1\n1 0\n1\n1 2\n"
        self.assertEqual(replay(data,out),{"score":2,"turns":2,"errors":0})

    def test_invisible_sheet_is_invalid_even_if_move_would_help(self):
        data=b"3 3 1 1\n000\n0\n"
        with self.assertRaisesRegex(ValueError,"not visible"):
            replay(data,b"0..\n1.1\n..0\n1\n2 0\n")

    def test_wrong_order_is_charged_once_for_multiple_bits(self):
        data=b"3 3 1 1\n111\n0\n"
        self.assertEqual(replay(data,b"0..\n1.1\n..0\n0\n"),
                         {"score":6,"turns":0,"errors":1})
