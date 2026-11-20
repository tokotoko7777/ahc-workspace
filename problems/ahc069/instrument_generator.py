#!/usr/bin/env python3
"""Log metadata without changing the random stream or serialized input."""
from pathlib import Path
import sys

source=Path(sys.argv[1])/"src/lib.rs"
text=source.read_text()
anchor='    let num_pond = rng.gen_range(0..=(900 - num_cluster) as i32) as usize;'
marker='    eprintln!("{},{:.3},{},{}",seed,r,num_cluster,num_pond);'
if marker in text:
    raise ValueError("expected an isolated fresh copy")
if text.count(anchor)!=1:
    raise ValueError("generator variant not recognized")
source.write_text(text.replace(anchor,anchor+"\n"+marker))
