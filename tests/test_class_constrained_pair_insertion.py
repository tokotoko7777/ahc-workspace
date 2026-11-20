"""Differential-test the classified pair insertion helper against full rescoring."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


@unittest.skipUnless(shutil.which("g++"), "g++ is required")
class ClassPairInsertionTests(unittest.TestCase):
    def test_against_all_admissible_gaps(self):
        source = Path(__file__).with_name("class_constrained_pair_insertion_test.cpp")
        with tempfile.TemporaryDirectory() as folder:
            exe = Path(folder) / "test"
            subprocess.run(["g++","-std=c++23","-O2","-Wall","-Wextra","-Werror",
                            str(source),"-o",str(exe)],check=True,timeout=30)
            subprocess.run([exe],check=True,timeout=30)
