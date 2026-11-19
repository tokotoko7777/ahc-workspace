"""Compile and run the counter and AHC004 integration checks."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('g++'), 'g++ is required')
class CoverageTests(unittest.TestCase):
    def check_cpp(self, name):
        with tempfile.TemporaryDirectory() as folder:
            binary = Path(folder) / 'check'
            built = subprocess.run(
                ['g++', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic',
                 '-I', str(ROOT), str(ROOT / 'tests' / name), '-o', str(binary)],
                capture_output=True, text=True,
            )
            self.assertEqual(built.returncode, 0, built.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_counter_against_full_recomputation(self):
        self.check_cpp('coverage_counter_test.cpp')

    def test_ahc004_against_direct_matching(self):
        self.check_cpp('ahc004_coverage_test.cpp')
