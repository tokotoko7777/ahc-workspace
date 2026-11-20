"""Differential-test the two-row transfer against exhaustive placements."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


@unittest.skipUnless(shutil.which("g++"), "g++ is required")
class BrickTwoRowTests(unittest.TestCase):
    def test_two_row_dp_matches_all_placements(self):
        source = Path(__file__).with_name("ahc071_two_row_test.cpp")
        with tempfile.TemporaryDirectory() as folder:
            exe = Path(folder) / "test"
            subprocess.run(["g++","-std=c++23","-O2","-Wall","-Wextra","-Werror",
                            str(source),"-o",str(exe)],check=True,timeout=30)
            subprocess.run([exe],check=True,timeout=30)
