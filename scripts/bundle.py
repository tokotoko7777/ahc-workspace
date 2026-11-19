#!/usr/bin/env python3
"""Expand literal quoted C++ includes into one submission file (stdlib only)."""

import argparse
import hashlib
from pathlib import Path
import re
import subprocess
import sys
from urllib.parse import quote


ROOT = Path(__file__).resolve().parents[1]
PROJECT_URL = 'https://github.com/tokotoko7777/ahc-workspace'
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


class SourceReference:
    """Link to a local Git revision; this does NOT verify when it became public."""

    def __init__(self, ref):
        result = self.git('rev-parse', '--verify', '--end-of-options', ref + '^{commit}')
        self.commit = result.stdout.decode().strip()
        if result.returncode or not re.fullmatch(r'[0-9a-f]{40}', self.commit):
            raise BundleError('source-ref must resolve to a commit in this checkout')
        self.blobs = {}

    @staticmethod
    def git(*args):
        return subprocess.run(['git', '-C', str(ROOT), *args], capture_output=True)

    def blob(self, relative):
        name = relative.as_posix()
        if name not in self.blobs:
            result = self.git('cat-file', 'blob', self.commit + ':' + name)
            self.blobs[name] = result.stdout if result.returncode == 0 else None
        return self.blobs[name]

    def url(self, relative):
        return PROJECT_URL + '/blob/' + self.commit + '/' + quote(relative.as_posix(), safe='/')

    def comment(self, path, source, header):
        relative = path.relative_to(ROOT) if path.is_relative_to(ROOT) else None
        if header:
            # Only this repository's library headers get automatic source claims.
            if relative is None or relative.parts[0] != 'library':
                return ''
            original = self.blob(relative)
            if original is None:
                raise BundleError(f'{relative}: not present at source-ref {self.commit}')
            if original != path.read_bytes():
                raise BundleError(f'{relative}: differs from source-ref; publish/select the matching version')
            return '// Source: ' + self.url(relative) + '\n'

        # Copying a template keeps this marker. The final problem-specific source
        # is expected to differ from the template: label it as a basis, not a match.
        for line in source.split('\n'):
            if line.startswith('// AHC_SOURCE: '):
                basis = (ROOT / line.removeprefix('// AHC_SOURCE: ').strip()).resolve()
                if not basis.is_relative_to(ROOT):
                    raise BundleError('AHC_SOURCE must name a file inside this repository')
                basis = basis.relative_to(ROOT)
                if self.blob(basis) is None:
                    raise BundleError(f'{basis}: source basis not present at source-ref {self.commit}')
                return '// Source basis (before problem-specific edits): ' + self.url(basis) + '\n'
            if line.strip() and not line.lstrip().startswith('//'):
                break
        # A complete, unchanged example can be identified without a copy marker.
        if relative is not None and self.blob(relative) == path.read_bytes():
            return '// Source: ' + self.url(relative) + '\n'
        return ''


class Bundler:
    def __init__(self, include_dirs=(), source_reference=None):
        self.include_dirs = [Path(p).resolve() for p in include_dirs] + [ROOT]
        self.source_reference = source_reference
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
        reference = self.source_reference.comment(path, source, header) if self.source_reference else ''
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
            return reference + body
        # Expand every occurrence under the SAME guard: a header first included
        # inside #if 0 must still be available at a later active include site.
        # Repeated textual bodies are intentional; definitions compile only once.
        identity = str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)
        guard = 'AHC_BUNDLE_' + hashlib.sha256(identity.encode()).hexdigest().upper()
        label = self.label(path)
        if not reference and path.is_relative_to(ROOT / 'library'):
            reference = '// Source repository: ' + PROJECT_URL + '\n'
        return (f'// begin: {label}\n#ifndef {guard}\n#define {guard}\n'
                f'{reference}{body}#endif  // {guard}\n// end: {label}\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path, help='main.cpp to bundle')
    parser.add_argument('-I', dest='include_dirs', action='append', default=[], metavar='DIR',
                        help='additional quoted-include directory (repeatable)')
    parser.add_argument('-o', '--output', type=Path, help='output file (default: stdout)')
    parser.add_argument('--source-ref', metavar='REF',
                        help='Git commit/tag for immutable source URLs; library headers must match')
    args = parser.parse_args()
    try:
        reference = SourceReference(args.source_ref) if args.source_ref else None
        bundler = Bundler(args.include_dirs, reference)
        body = bundler.expand(args.source)
        # Keep this repository's MIT notice in copies submitted as a single file.
        license_text = (ROOT / 'LICENSE').read_text(encoding='utf-8')
        result = '// Generated by scripts/bundle.py; edit the original source.\n'
        result += '// Library repository: ' + PROJECT_URL + '\n'
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
