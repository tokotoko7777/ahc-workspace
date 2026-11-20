"""Regression cases for tail-overlap and bite/food accounting."""
from pathlib import Path
import runpy
import unittest

SOURCE=Path(__file__).resolve().parents[1]/"problems/ahc063/benchmark.py"
replay=runpy.run_path(str(SOURCE))["replay"]


def fixture(food):
    cell=[0]*64
    for row,col,value in food:
        cell[8*row+col]=value
    return ("8 6 3\n1 1 1 1 1 2\n"+" ".join(map(str,cell))).encode()


class SnakeReplayTests(unittest.TestCase):
    def test_food_appends_color(self):
        x=replay(fixture([(4,1,2)]),b"R\n")
        self.assertTrue(x["perfect"])
        self.assertEqual(x["cost"],1)

    def test_overlap_body_drops_suffix(self):
        x=replay(fixture([(4,1,2)]),b"R\nU\nL\n")
        self.assertEqual(x["bites"],1)
        self.assertEqual(x["cost"],20003)

    def test_overlap_only_tail_does_not_cut(self):
        x=replay(fixture([(7,7,2)]),b"R\nU\nL\n")
        self.assertEqual(x["bites"],0)
        self.assertEqual(x["cost"],20003)
