#!/usr/bin/env python3
"""Expand literal quoted C++ includes into one submission file (stdlib only)."""

import argparse
import hashlib
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
# Mask comments and literals before looking for directives. In particular, a line
# resembling #include inside a raw string or block comment is ordinary text.
TOKENS = re.compile(
    r'R"(?P<delimiter>[^ ()\\\t\r\n]{0,16})\(.*?\)(?P=delimiter)"'
    r'|//(?:\\\n|[^\n])*|/\*.*?\*/'
    r'|"(?:\\.|[^"\\])*"'
    r'|(?<![\w])(?:u8|u|U|L)?\'(?:\\.|[^\'\\\n])*\'',
    re.DOTALL,
)
DIRECTIVE = re.compile(r'^\s*#\s*(\w+)')


class BundleError(Exception):
    pass


class Bundler:
    def __init__(self, include_dirs=()):
        self.include_dirs = [Path(p).resolve() for p in include_dirs] + [ROOT]
        self.dependencies = set()
        self.stack = []

    @staticmethod
    def label(path):
        try:
            return path.relative_to(ROOT).as_posix()
        except ValueError:
            return path.name

    def resolve_include(self, name, parent, line):
        for directory in [parent.parent, *self.include_dirs]:
            candidate = (directory / name).resolve()
            if candidate.is_file():
                return candidate
        raise BundleError(f'{parent}:{line}: include not found: "{name}" (add -I DIR)')

    def expand(self, path, header=False):
        path = Path(path).resolve()
        if path in self.stack:
            chain = ' -> '.join(self.label(p) for p in [*self.stack, path])
            raise BundleError(f'cyclic include: {chain}')
        self.dependencies.add(path)
        source = path.read_text(encoding='utf-8-sig')
        masked = TOKENS.sub(lambda m: re.sub(r'[^\n]', ' ', m[0]), source)
        source_lines = source.split('\n')
        lines = []
        self.stack.append(path)
        try:
            for number, (original, visible) in enumerate(
                    zip(source_lines, masked.split('\n')), 1):
                if number < len(source_lines):
                    original += '\n'
                directive = DIRECTIVE.match(visible)
                if not directive:
                    lines.append(original)
                    continue
                name = directive[1]
                argument = TOKENS.sub(
                    lambda m: ' ' if m[0].startswith(('//', '/*')) else m[0],
                    original[directive.end():],
                ).strip()
                if name == 'pragma' and argument == 'once':
                    lines.append(original[:visible.index('#')] + '\n')
                    continue  # Our guards below also work after inlining.
                if name != 'include':
                    lines.append(original)
                    continue
                quoted = re.fullmatch(r'"([^"\n]+)"', argument)
                if quoted:
                    dependency = self.resolve_include(quoted[1], path, number)
                    prefix = original[:visible.index('#')]
                    directive_text = original[visible.index('#'):].rstrip('\n')
                    lines.append(prefix + '// ' + directive_text + ' (expanded)\n')
                    lines.append(self.expand(dependency, header=True))
                elif re.fullmatch(r'<[^>\n]+>', argument):
                    lines.append(original)  # Standard/compiler-provided headers remain includes.
                else:
                    raise BundleError(
                        f'{path}:{number}: use a single-line literal #include "path" or <header>')
        finally:
            self.stack.pop()
        body = ''.join(lines)
        if body and not body.endswith('\n'):
            body += '\n'
        if not header:
            return body
        # Expand every occurrence under the SAME guard: a header first included
        # inside #if 0 must still be available at a later active include site.
        # Repeated textual bodies are intentional; definitions compile only once.
        identity = str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)
        guard = 'AHC_BUNDLE_' + hashlib.sha256(identity.encode()).hexdigest().upper()
        label = self.label(path)
        return (f'// begin: {label}\n#ifndef {guard}\n#define {guard}\n'
                f'{body}#endif  // {guard}\n// end: {label}\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path, help='main.cpp to bundle')
    parser.add_argument('-I', dest='include_dirs', action='append', default=[], metavar='DIR',
                        help='additional quoted-include directory (repeatable)')
    parser.add_argument('-o', '--output', type=Path, help='output file (default: stdout)')
    args = parser.parse_args()
    try:
        bundler = Bundler(args.include_dirs)
        body = bundler.expand(args.source)
        # Keep this repository's MIT notice in copies submitted as a single file.
        license_text = (ROOT / 'LICENSE').read_text(encoding='utf-8')
        result = '// Generated by scripts/bundle.py; edit the original source.\n'
        result += ''.join('// ' + line + '\n' for line in license_text.splitlines())
        result += '\n' + body
        if args.output:
            if args.output.resolve() in bundler.dependencies:
                raise BundleError('output would overwrite an input source/header; choose another path')
            args.output.parent.mkdir(parents=True, exist_ok=True)
            # Expansion/validation finishes before opening the destination.
            args.output.write_text(result, encoding='utf-8')
        else:
            sys.stdout.write(result)
    except (BundleError, OSError, UnicodeError) as error:
        print(f'bundle: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
