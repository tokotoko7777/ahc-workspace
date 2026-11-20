"""Differential-test rounded multiplicative observations against an independent integral."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


@unittest.skipUnless(shutil.which("g++"), "g++ is required")
class RoundedMultiplicativeTests(unittest.TestCase):
    def test_against_independent_multiplier_integral(self):
        source = Path(__file__).with_name("rounded_multiplicative_normal_test.cpp")
        with tempfile.TemporaryDirectory() as folder:
            exe = Path(folder) / "test"
            subprocess.run(["g++","-std=c++23","-O2","-Wall","-Wextra","-Werror",
                            str(source),"-o",str(exe)],check=True,timeout=30)
            subprocess.run([exe],check=True,timeout=30)
