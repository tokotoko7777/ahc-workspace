"""Score and legality regressions for simultaneous relocation."""
from pathlib import Path
import runpy
import unittest

MODULE=Path(__file__).resolve().parents[1]/"problems/ahc069/benchmark.py"
replay=runpy.run_path(str(MODULE))["replay"]


class ParkReplayTests(unittest.TestCase):
    def test_exchange_regions_preserves_worst_compactness(self):
        inp=b"4 3 0.100\n....\n....\n....\n....\n0 0 30 4 100\n1 1 20 4 100\n2 2 40 4 120\n"
        square="0 0 0 1 1 0 1 1"
        line="3 0 3 1 3 2 3 3"
        right="0 2 0 3 1 2 1 3"
        out=f"0 Yes {square} 1 0 {line} Yes {square} 2 0 {square} 1 {line} Yes {right}"
        self.assertEqual(replay(inp,out.encode()),{"score":250,"moved":3,"accepted":3})

    def test_half_cent_fees_are_exact_and_add_across_moves(self):
        # float(0.009)*1500 is 13.499999999999998; official cost rounds
        # exact 13.5 to 14 on each move, so two such edges can differ by 2.
        inp=b"4 3 0.009\n....\n....\n....\n....\n0 0 30 4 1500\n1 1 20 4 100\n2 2 40 4 120\n"
        square="0 0 0 1 1 0 1 1"
        right="0 2 0 3 1 2 1 3"
        out=f"0 Yes {square} 1 0 {right} No 1 0 {square} No"
        self.assertEqual(replay(inp,out.encode()),{"score":1472,"moved":2,"accepted":1})

    def test_disconnected_region_is_rejected(self):
        inp=b"4 1 0.100\n....\n....\n....\n....\n0 0 30 4 100\n"
        with self.assertRaisesRegex(ValueError,"disconnected"):
            replay(inp,b"0 Yes 0 0 0 1 3 2 3 3")
