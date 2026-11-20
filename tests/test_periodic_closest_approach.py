"""Differential-test the periodic motion helper against a whole time scan."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


@unittest.skipUnless(shutil.which("g++"), "g++ is required")
class PeriodicApproachTests(unittest.TestCase):
    def test_against_all_integer_times(self):
        source = Path(__file__).with_name("periodic_closest_approach_test.cpp")
        with tempfile.TemporaryDirectory() as folder:
            exe = Path(folder) / "test"
            subprocess.run(["g++","-std=c++23","-O2","-Wall","-Wextra","-Werror",
                            str(source),"-o",str(exe)],check=True,timeout=30)
            subprocess.run([exe],check=True,timeout=30)
