# Tactical gates on top of accelerated Flex + terminal

This experiment asks whether a human can redirect one late movement toward an already almost-sealed 1–3-cell connector, then close it next turn. This differs from spending multiple turns preparing a new chamber. All baseline wall actions remain in both turns. The move/redirection can happen during an old planned seam. It forecasts a legal candidate from current positions and pet speed, chooses distinct people, rejects plans that lose more than 46% of the mobile component or gain less than about 1.0m on the current snapshot, and reevaluates exact legality, kept area, and benefit when the next observation arrives. The final turn still uses the exact terminal overlay. This earlier movement forecast is **not** mathematically non-regressing.

The development split came entirely from the official generator with SHA-derived seeds `ahc008/habitat/1/tactical-dev/{i}` for 0–49. Strict variants that never allowed an active seam and only used idle people all scored 788,057,337, exactly equal to the base. Allowing idle preparation during an active seam scored 788,094,374. Preserving build actions but allowing movement redirection first gave 805,782,515 (7 wins / 3 losses). No distributed practice cases or original system seeds were used in this experiment.

First independently generated frozen cohort: baseline 1,456,590,247 → tactical 1,492,655,426 (14 wins, 7 losses, 100/100 legal, max tester wall 0.427851 s). See `tactical100-first.csv` and `tactical100-first.json` for official tester + independent recorded-action replay. The binary used feature flags matching the defaults in `tactical_gate_submission.cpp`; the executable hash may differ from a subsequent rebuild.

A separately tested compound search collects pet-mask-specific leaves, pairs geometrically disjoint leaves, and enforces at most one changed preparatory movement across the pair. An extra max-flow variant locates leaves with vertex-cut capacities constrained to available human moves and short closure length. Neither provided a material development gain over the plain movement redirection: 805,782,515 for composition; 805,879,737 for composition + pet-source cuts; not promoted. Source patch and flags remain reproducible.

```sh
g++ -std=c++23 problems/ahc008/flex_terminal_submission.cpp -o flex_terminal
g++ -std=c++23 problems/ahc008/tactical_gate_submission.cpp -o tactical_gate
python - <<'PY' > seeds.txt
from hashlib import sha256
for i in range(100):
    print(int.from_bytes(sha256(f"ahc008/20261122/tactical-gate-v1/fresh100/{i}".encode()).digest()[:8], "little"))
PY
/path/to/official-tools/target/release/gen seeds.txt
python problems/ahc008/benchmark_cases.py /path/to/official-tools/target/release/tester in tactical100.csv flex_terminal=./flex_terminal tactical=./tactical_gate
```

Compound leaves: copy the standalone source, apply `tactical-composed-leaves.patch` to the copy, then compile with `-DGATE_COMPOSE=1 -DGATE_DURING_LINE=1 -DGATE_REDIRECT=1 -DGATE_MOVEPREEMPT=1 -DGATE_FROM=245 -DGATE_RISK=2 -DGATE_RETAIN=54 -DGATE_GAIN=10`. Add `-DGATE_MINCUT_LEAF=1` for the vertex-cut arm.

The second untouched generated100 cohort uses `second100` in place of `fresh100` in seed strings above. This exact source with default flags independently replayed at **1,669,639,481** versus base **1,637,583,358** (+1.96%). There were 16 wins, 75 ties, and 9 losses, all 100 turns/cases legal and same official scores for the replay, max tester wall 0.537527 s. Across both cohorts, 30 wins / 16 losses and a 68,121,302 cumulative gain (+2.20%). This is a small, reproducible improvement but it remains far below the 6,263,333,336 reference. It is an experimental solver rather than a merge or leaderboard claim.

After freezing and independently checking the source on generated cases, the unchanged second-cohort binary was run on all 100 official distributed practice cases, only as a headline comparison and not for tuning. **1,512,932,629** versus **1,465,133,201** (+3.26%, 22 wins / 66 ties / 12 losses), zero invalid cases, maximum direct tester wall **0.447700 s**. The tester and recorded-action independent replay agree. The generated and distributed scores are not claims about the original online test set; no submission or merge is justified.
