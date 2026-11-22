"""Lossless gzip-chunk the two-row-per-case CSV (header copied per file)."""
import gzip,sys
from pathlib import Path
file, out=map(Path,sys.argv[1:])
data=file.read_bytes().splitlines(keepends=True)
assert len(data)==201, len(data)
out.mkdir(parents=True,exist_ok=True)
for i in range(5):
 p=out/f'{file.stem}-{i*20:02d}-{i*20+19:02d}.csv.gz'
 p.write_bytes(gzip.compress(b''.join(data[:1]+data[1+40*i:1+40*(i+1)]),mtime=0))
# All fields (including provenance hashes and actual telemetry) reconstruct exactly.
got=[gzip.decompress(f.read_bytes()).splitlines(keepends=True)[1:] for f in sorted(out.glob(file.stem+'-*.csv.gz'))]
assert [x for g in got for x in g]==data[1:]
