"""Small independent joint-action replay regressions."""
from pathlib import Path
import runpy
import unittest

RUNNER=Path(__file__).resolve().parents[1]/"problems/ahc061/benchmark.py"
replay=runpy.run_path(str(RUNNER))["replay"]


class TerritoryReplayTests(unittest.TestCase):
    def test_neutral_collision_restores_both(self):
        # All equal values. Both initially in opposite corners.
        data=b"2 2 1 2 100 100 100 100 0 0 1 1"
        self.assertEqual(replay(data,[[1,1]]),100000)

    def test_forbidden_target_on_opposing_piece(self):
        data=b"2 2 1 2 100 100 100 100 0 0 0 1"
        with self.assertRaisesRegex(ValueError,"wrong target"):
            replay(data,[[1,1]])

    def test_level_one_capture_and_occupied_old_position(self):
        data=b"2 2 3 2 100 100 100 100 0 0 0 1"
        # Each player goes down, making the previous positions attackable.
        # First capture changes owner, next turn both hold and reinforce.
        self.assertEqual(replay(data,[[2,3],[1,0],[1,0]]),100000)
