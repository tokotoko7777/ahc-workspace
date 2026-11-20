# AHC001 exterior-fixed exact pocket diagnostic

This is a standalone repair experiment; `main.cpp` and the historical v64
baseline are unchanged. Freeze v64 once at 4.8 seconds on each fresh input;
all arms start from the identical saved legal layout. The official list is
used only to verify disjoint seeds. Seeds were preregistered before the first
run: SHA-256 salt `ahc001/pocket-v5/{split}/{i}`, 6 development and 10
untouched cases. No official final outputs or rank test.

Each of at most three windows has six IDs: start with the biggest current
quality deficits and use rectangle gap then anchor Manhattan distance to
select neighbors. Every exterior rectangle must remain identical. Up to
24 options per ID include the exact original, clipped smaller options, and
near-target alternative aspects fitted inside expanded boxes cut against
fixed exterior rectangles. Each rectangle independently includes its anchor
and misses all exterior rectangles, including other anchors. The portfolio
may miss good continuous rectangles; nothing here proves an optimum
outside the discrete portfolio.

- **None:** identical frozen board.
- **Local:** best improving one-/two-ID coordinate change (a two-ID change
  can leave either rectangle unchanged), with repeated scans from the same
  start.
- **Exact:** one option per ID, propagate incompatible option bitmasks,
  choose the most constrained unassigned ID, and use the sum of each
  remaining ID's highest still-feasible quality as an optimistic bound.
  The incumbent contributes zero gain, all returned assignments are checked,
  and a noncapped search exhausts or bounds all assignments.

Both repair arms receive the same shared portfolio, three windows, exactly
the same precomputed incompatibility masks, maximum 9,000 candidate
feasibility checks per window, and an external one-second guard. A check
extends one exact-search branch or tests one local pair proposal, so the
raw counts are not instruction-for-instruction equivalent; actual wall
times are also recorded. Each window is independently started at the unchanged
original board; only the highest exact-scoring returned whole layout is kept.
This is diagnostic standalone wall only, not an integrated 4.8s + repair
timing claim.

## Reproduction

From the repository root, with the public AtCoder `gen`/`vis` and
official seed list:

```sh
D=problems/ahc001/experiments/pocket_v5
git show 815b7439185375cd87407c9ec89ebc6e4d2d979c:problems/ahc001/main.cpp > /tmp/v64.cpp
g++ -std=c++23 -O3 /tmp/v64.cpp -o /tmp/v64
python3 scripts/bundle.py "$D/pocket.cpp" -o /tmp/ahc001-pocket.cpp
g++ -std=c++23 -O3 /tmp/ahc001-pocket.cpp -o /tmp/ahc001-pocket
python3 "$D/build_test.py" /tmp/ahc001-pocket.cpp > /tmp/ahc001-pocket-test.cpp
g++ -std=c++23 -O2 /tmp/ahc001-pocket-test.cpp -o /tmp/ahc001-pocket-test
/tmp/ahc001-pocket-test
taskset -c 8 python3 "$D/benchmark.py" "$GEN" "$VIS" /tmp/v64 /tmp/ahc001-pocket "$OFFICIAL_SEEDS" "$WORK"
```

## Result

| Split | Local mean official gain | Exact mean official gain | Exact vs frozen (W/T/L) | Exact vs local (W/T/L) |
|---|---:|---:|---:|---:|
| Development 6 | 0 | 0 | 0/6/0 | 0/6/0 |
| Untouched 10 | +14.20 | +14.20 | 1/9/0 | 0/10/0 |

All 48 official scores matched independent geometry and score checking;
the only rounded change was +142 points on one untouched case in both
arms. The median gain was zero. Each repair arm measured raw positive
gain in 10/48 windows before whole-case score rounding. All 48 exact
window searches completed below the cap; observed exact branch checks
6,216 vs 70,590 local pair checks (different units under shared ceilings).
Peak standalone wall was 0.007402 s exact, 0.007011 s local, and v64
frozen preparation peak 4.863238 s. Synthetic brute force matched the exact
assignment on 600 portfolios; exact beat local on 42, so the lack of
official metric gains here is not because the solvers are equivalent.
The bounded portfolio around v64 was the limitation, not the cap.

Frozen repair binary SHA-256:
`372a9731b4e8573290ace7bd1f44e5d830af85f34c30ee9a86784b4377a8ff15`.
Bundle SHA-256:
`14f60a1dd72bc07a9baa0bd383ffa4886e7198c2f4747c4683cd4ec8eda124cc`.
The v64 default remains unchanged. No full final replay or reuse of final
case outputs; stop increasing late repair variants. This improvement is
far short of the ~411,789-point historical gap per case. If reopening
AHC001, use stronger large-scale initial layout diversity instead of
expanding this same late search budget; otherwise prioritize another round.
