"""Differential-test bridges against endpoint connectivity after removing each edge."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


@unittest.skipUnless(shutil.which("g++"), "g++ is required")
class UndirectedBridgeTests(unittest.TestCase):
    def test_each_edge_against_removal(self):
        source = Path(__file__).with_name("undirected_bridges_test.cpp")
        with tempfile.TemporaryDirectory() as folder:
            exe = Path(folder) / "test"
            subprocess.run(["g++","-std=c++23","-O2","-Wall","-Wextra","-Werror",
                            str(source),"-o",str(exe)],check=True,timeout=30)
            subprocess.run([exe],check=True,timeout=30)
