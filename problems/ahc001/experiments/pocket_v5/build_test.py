"""Append the differential test to the exact bundled model, renaming its CLI only."""
from pathlib import Path
import sys

source = Path(sys.argv[1]).read_text()
entry = 'int main(int argc,char** argv){'
assert source.count(entry) == 1
print(source.replace(entry, 'int pocket_entry(int argc,char** argv){\n'))
print(Path(__file__).with_name('pocket_test.inc').read_text())
