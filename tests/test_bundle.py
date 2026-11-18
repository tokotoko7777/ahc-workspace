"""Behavioral checks for the optional single-file submission command."""

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / 'scripts' / 'bundle.py'


class BundleTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.work = Path(self.temp.name)

    def write(self, name, content):
        path = self.work / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding='utf-8')
        return path

    def bundle(self, source='main.cpp', *args):
        return subprocess.run(
            [sys.executable, str(SCRIPT), str(self.work / source), *map(str, args)],
            cwd=self.work, capture_output=True, text=True,
        )

    def check_runs(self, bundled, expected):
        if not shutil.which('g++'):
            self.skipTest('g++ is needed for the single-file compile check')
        # Compile outside the include tree and without -I flags.
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'submit.cpp'
            executable = Path(directory) / 'solver'
            source.write_text(bundled, encoding='utf-8')
            compiled = subprocess.run(
                ['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', str(source), '-o', str(executable)],
                cwd=directory, capture_output=True, text=True,
            )
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, expected)

    def test_nested_shared_headers_and_path_aliases(self):
        self.write('value.hpp', 'int value() { return 42; }\n')  # Deliberately no guard.
        self.write('sub/first.hpp', '#include "../value.hpp"\n')
        self.write('second.hpp', '#pragma once\n#include "sub/../value.hpp"\n')
        self.write('main.cpp', '#include <iostream>\n#include "sub/first.hpp"\n'
                   '#include "second.hpp"\nint main() { std::cout << value(); }\n')
        result = self.bundle()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn('#pragma once', result.stdout)
        self.assertIn('Copyright (c) 2026 tokotoko7777 and contributors', result.stdout)
        self.check_runs(result.stdout, '42')

    def test_first_include_in_inactive_branch_does_not_hide_later_include(self):
        self.write('value.hpp', 'int value() { return 7; }\n')
        self.write('main.cpp', '#include <iostream>\n#if 0\n#include "value.hpp"\n#endif\n'
                   '#include "value.hpp"\nint main() { std::cout << value(); }\n')
        result = self.bundle()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.check_runs(result.stdout, '7')

    def test_comments_and_raw_strings_are_not_includes(self):
        self.write('value.hpp', 'constexpr int value = 3;\n')
        self.write('main.cpp', '#include <iostream>\n'
                   '/*\n#include "missing_in_comment.hpp"\n*/ #include "value.hpp" // kept\n'
                   'const char* text = R"tag(\n#include "missing_in_string.hpp"\n)tag";\n'
                   '// #include "missing_on_comment_line.hpp"\n'
                   'int main() { std::cout << value; }\n')
        result = self.bundle()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('missing_in_string.hpp', result.stdout)
        self.assertIn('// kept', result.stdout)
        self.check_runs(result.stdout, '3')

    def test_cpp_digit_separators_are_not_character_literals(self):
        self.write('value.hpp', 'constexpr int value = 3;\n')
        self.write('main.cpp', '#include <iostream>\nconstexpr int first = 1\'000;\n'
                   '#include "value.hpp"\nconstexpr int second = 2\'000;\n'
                   'int main() { std::cout << first + second + value; }\n')
        result = self.bundle()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.check_runs(result.stdout, '3003')

    def test_local_directory_precedes_include_directories(self):
        self.write('src/pick.hpp', 'constexpr int value = 1;\n')
        self.write('extra/pick.hpp', 'constexpr int value = 2;\n')
        self.write('extra/other.hpp', 'constexpr int other = 4;\n')
        self.write('src/main.cpp', '#include <iostream>\n#include "pick.hpp"\n'
                   '#include "other.hpp"\nint main() { std::cout << value + other; }\n')
        result = self.bundle('src/main.cpp', '-I', self.work / 'extra')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.check_runs(result.stdout, '5')

    def test_repository_include_path_and_output_directory(self):
        self.write('main.cpp', '#include "library/timer.hpp"\nint main() { Timer timer; }\n')
        output = self.work / 'generated' / 'submit.cpp'
        result = self.bundle('main.cpp', '-o', output)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, '')
        self.check_runs(output.read_text(), '')

    def test_missing_include_does_not_truncate_existing_output(self):
        self.write('main.cpp', '#include "missing.hpp"\n')
        output = self.write('submit.cpp', 'previous submission\n')
        result = self.bundle('main.cpp', '-o', output)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('include not found', result.stderr)
        self.assertEqual(output.read_text(), 'previous submission\n')
        self.assertEqual(result.stdout, '')

    def test_cyclic_include_reports_the_chain(self):
        self.write('main.cpp', '#include "a.hpp"\n')
        self.write('a.hpp', '#include "b.hpp"\n')
        self.write('b.hpp', '#include "a.hpp"\n')
        result = self.bundle()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('main.cpp -> a.hpp -> b.hpp -> a.hpp', result.stderr)
        self.assertEqual(result.stdout, '')

    def test_macro_include_is_rejected(self):
        self.write('main.cpp', '#define HEADER "a.hpp"\n#include HEADER\n')
        result = self.bundle()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('single-line literal', result.stderr)

    def test_multiline_include_is_rejected(self):
        self.write('main.cpp', '#include \\\n"a.hpp"\n')
        result = self.bundle()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('single-line literal', result.stderr)

    def test_output_cannot_overwrite_source_or_dependency(self):
        source = self.write('main.cpp', '#include "value.hpp"\n')
        header = self.write('value.hpp', 'constexpr int value = 0;\n')
        for path in (source, header):
            with self.subTest(path=path.name):
                original = path.read_text()
                result = self.bundle('main.cpp', '-o', path)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('overwrite', result.stderr)
                self.assertEqual(path.read_text(), original)

    def test_output_is_reproducible(self):
        self.write('main.cpp', '#include "library/timer.hpp"\n')
        first = self.bundle()
        second = self.bundle()
        self.assertEqual(first.returncode, 0, first.stderr)
        self.assertEqual(second.returncode, 0, second.stderr)
        self.assertEqual(first.stdout, second.stdout)


if __name__ == '__main__':
    unittest.main()
