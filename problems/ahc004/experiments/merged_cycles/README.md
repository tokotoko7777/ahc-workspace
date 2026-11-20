# Fallback: complete toroidal lines from merged partial paths

This candidate preserves the original standalone solver when its observed-cycle gate
passes. If the gate fails and a read of length at least nine exists, run at most
18 ms of additional preprocessing. Branch-cut unitigs of order four and five
and direct reads contribute candidate stretches. Each shared five-letter substring
proposes a mod-20 relative displacement. Merge in decreasing overlap-vote order,
retaining exact assignments in a packed circular 60-bit word and rejecting every
known-letter conflict. A resulting complete line need not have an observed
de Bruijn closing edge: independent reads may between them pin all 20 letters
without having observed that transition. Holes remain wildcards in the assembly
but are not yet passed to the geometry solver.

New complete lines must survive the original 28-line / 75% aggregate exact
read-coverage gate. Geometry still produces distinct exact intersection
assignments and uses the original final-score check and fallback. The existing
geometry timer and solver timeout controls are unchanged. Telemetry for the new
fallback is a separate `partial_` line to preserve old parsers.

```sh
python3 scripts/bundle.py problems/ahc004/experiments/merged_cycles/submission_entry.cpp -o merged.single.cpp
g++ -std=c++23 merged.single.cpp -o merged
```

Synthetic-only checks (no official inputs or their generating seeds used):

```sh
cd problems/ahc004/experiments/merged_cycles
g++ -std=c++23 driver.cpp -o driver
PYTHONHASHSEED=0 python3 differential.py
g++ -std=c++23 aug-driver.cpp -o aug-driver
PYTHONHASHSEED=0 python3 cyclecheck.py
```

`differential.py` checks the candidate sets against an independently written
string-based implementation on 144 small runs. `synthetic-output.txt` uses forty
independent uniform tori per (length, read count), reused for overlaps five and
six, and counts distinct authentic full lines up to toroidal rotation. Fast
C++ timing is assembly-only. Gate estimates emulate exact enumeration and the
28/75 logic without live solver timers; for the baseline evidence tie-break
they give all observed candidates the five-point bonus rather than just those
observed with the longer k-mer. They are not predicted solver scores. Overlap 5
was retained because the modest precision tradeoff buys many more full lines in
the synthetic instances. L5–6 seldom meet the original geometry gate even with
this supplement; a later partial-pattern realization is separate work.
