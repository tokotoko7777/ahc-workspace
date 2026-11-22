"""Produce five controlled AHC008 fixtures, one for each corner pet species.

Setup deliberately station four builders outside a 2x2 board corner; nine
other pets stay away from its initial walls. These are mechanism witnesses,
not samples of contest input distribution.
"""
from pathlib import Path
import sys

base = Path(sys.argv[1])
base.mkdir(parents=True, exist_ok=True)
for i, name in enumerate("cow pig rabbit dog cat".split(), 1):
    lines = ["10", f"1 1 {i}"] + [f"{20+j//5} {20+j%5} {j%5+1}" for j in range(9)]
    lines += ["5", "1 4", "2 4", "4 1", "4 2", "16 16", str(96002219+i)]
    (base / (name+".txt")).write_text("\n".join(lines)+"\n")
