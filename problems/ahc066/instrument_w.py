"""Add only a seed/W diagnostic to a PRIVATE COPY of the official generator."""
import argparse
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('source',type=Path,help='private scratch copy of tools/src/lib.rs')
a=p.parse_args()
s=a.source.read_bytes()
lines=s.splitlines(keepends=True)
candidates=[i for i,line in enumerate(lines)
            if b'let W =' in line and b'gen_range' in line and b'0..N as i32' in line]
assert len(candidates)==1
i=candidates[0]
assert any(b'pub fn gen(seed: u64)' in row for row in lines[max(0,i-6):i])
nl=b'\r\n' if lines[i].endswith(b'\r\n') else b'\n'
lines.insert(i+1,b'    eprintln!("AHC066_W,{seed},{W}");'+nl)
a.source.write_bytes(b''.join(lines))
