# AHC034 physical walk shortcuts + exact cargo

[Task, scoring and archive status](../../README.md): unproven 150-case final
reproduction, rank one yosupo 9,486,350,384. This study compares fresh
official-generator cases only (10 development, 16 untouched; fixed
SHA-256 salt `ahc034/physical-shortcuts-v3/{split}/{i}`, disjoint from
earlier published runs and the 100 starter seeds). No final case scores
or case-level results used.

The frozen fixed-walk-flow result is the incumbent in both new arms;
the harness checks its travel score and step count and the pre-flow walk's
travel score. Leaving the original route in place is always legal.

- Control reanneals the original compressed visit sequence for two extra
  seeded epochs and tries flow on each expanded walk. It may retain the
  candidate's legal original allocation if exact flow times out.
- The shortcut arm tries two deterministic contiguous walk replacements.
  For each available subwalk improvement it checks each Manhattan L
  orientation, changing the physical walk only when every original
  nonzero cell retains a visit. The two modes use independent span
  limits 24 and 56 and distinct orientation ordering; each starts at
  the same incumbent. Run existing exact forward-time cargo flow on
  both and retain only cheaper complete solutions.
- Both arms attempt exactly two additional flow solves and share a
  1.85 s internal cutoff (<2 s externally). Actual geometry/route-search
  work and timing differ, so report time as well as counts.

| Split | Fixed flow mean score | Longer route+flow | Shortcuts+flow | Shortcuts vs longer W/T/L |
|---|---:|---:|---:|---:|
| Development 10 | 32,415,489.50 | 32,415,489.50 | 33,914,991.60 | 1/9/0 |
| Untouched 16 | 36,778,709.75 | 36,843,697.44 | 38,443,379.38 | 3/12/1 |

All 78 layouts fully leveled the board, respected the action/load
constraints, and matched independent and official scores. Untouched
mean steps 1434.25 → 1308.625 and carried cost 48,834.63 → 53,798.88
against frozen; total travel improved from 192,259.63 to 184,661.38
with unchanged handling. Both tested sets favored a few cases; median
paired improvement was zero. Candidate wall mean/peak untouched
0.2723/0.403914 s, control 0.4315/0.588930 s (peak on development 0.610657).
Keep both variants optional, not worth a historical rerun.

## Why route coverage alone fails

The shortest geometry per untouched case retains all nonzero cells and
averages 617.625 moves. But 43 of the 52 candidates over both sets fail
an independent exact cargo-feasibility test. Each positive cell can be
fully collected on its first visit, and each negative cell's deposit
can be delayed until its last visit. This maximizes the available load
at **every** prefix simultaneously: no other complete allocation can
collect more or deposit less yet. If that prefix profile is ever
negative, no cargo allocation can fix the walk. If it stays nonnegative,
the same first/last schedule is itself a witness. Only 9 candidates had
this property, and three improved untouched scores.

The next distinct change is to preserve prefix feasibility **during**
geometry edits, not merely retry aggressive coverage-only cut sizes:
build first/last event sums with a minimum-prefix summary, update events
only for cells touched by a splice, and reject any negative minimum.
That would let profitable later cuts survive when an earlier aggressive
cut would destroy chronological supply.

## Reproduce

```sh
p=problems/ahc034
d="$p/experiments/physical_shortcuts_v3"
python scripts/bundle.py "$p/main.cpp" -o /tmp/034-submission.cpp
g++ -std=c++23 -O3 -DAHC034_SHORTCUTS=0 /tmp/034-submission.cpp -o /tmp/034-base
g++ -std=c++23 -O3 -DAHC034_SHORTCUTS=1 /tmp/034-submission.cpp -o /tmp/034-long
g++ -std=c++23 -O3 -DAHC034_SHORTCUTS=2 /tmp/034-submission.cpp -o /tmp/034-shortcuts
g++ -std=c++23 -O2 "$d/feasibility_probe.cpp" -o /tmp/034-prefix
/tmp/034-prefix
taskset -c 8 python "$d/benchmark.py" "$GEN" "$VIS" /tmp/034-base /tmp/034-long /tmp/034-shortcuts "$STARTER_SEEDS" "$WORK"
# For each generated case, e.g. dev/0000:
 /tmp/034-prefix "$WORK/dev/in/0000.txt" "$WORK/dev/0000-baseline.txt"
```

Baseline binary was the existing C++23 bundle from local `37552b8`
(SHA `0d4cf4d9c516b16cb140f226bc10447579a8c3e0802365c4e9218a7c10c5c4db`).
Experiment bundle SHA `e355f46a3bc4536377a54705b78ca2538d74f2b805f562af7ec50982108d5cbc`;
long binary `f0d62b1ede421a22e9efc2b813268ef64745bdd0f080f669830bfe2ca0c5759f`;
shortcut binary `1d4bfe4e2113ff85b1204f0feb1a431c7c61d8ed8009bd445d4a62b7bec21c38`.
The checker reused existing flow and prefix-route components; the new
shortcut and feasibility proof are problem-specific.
