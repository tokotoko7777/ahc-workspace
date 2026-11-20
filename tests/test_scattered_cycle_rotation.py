"""Differential-test scattered rotations against a simultaneous reference."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


@unittest.skipUnless(shutil.which("g++"), "g++ is required")
class ScatteredCycleTests(unittest.TestCase):
    def test_rotation_matches_copy_and_index(self):
        source = Path(__file__).with_name("scattered_cycle_rotation_test.cpp")
        with tempfile.TemporaryDirectory() as folder:
            exe = Path(folder) / "test"
            subprocess.run(["g++","-std=c++23","-O2","-Wall","-Wextra","-Werror",
                            str(source),"-o",str(exe)],check=True,timeout=30)
            subprocess.run([exe],check=True,timeout=30)
