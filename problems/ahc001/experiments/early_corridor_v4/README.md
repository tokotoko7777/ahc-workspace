# Early corridor precedence, AHC001 v4
Preregistered arms: immutable v64 versus exactly one pre-fill two-axis initializer.
Both use 4.8s seed 0 variant 64, CPU 8 locally, alternating paired launch order.
Fresh SHA salt ahc001/early-corridor-v4/{split}/{i}, 8 dev and 12 holdout, check
against all official seeds and earlier development experiments. No final outputs.
Measure candidate pair count, positive feasible pairs on both axes, chosen pair
count, initializer admission relative to exact raw v64 start, chosen axis surviving
in best output, and holdout paired W/T/L + mean and median official score delta.
Useful target is the historical baseline gap (~411,789 score per case).
Keep opt-in unless clear unseen gains.

The variant saves the ordinary initialized board, incumbent, RNG and accepted
count. From points, it considers at most 18 pairs with conflicting largest
anchor-empty boxes, commits at most six disjoint pairs, and protects those
placements only during the usual first fill. For each axis and three seams,
it evaluates the top five empty boxes per endpoint after clipping against
already committed rectangles. The rank combines scaled fit and available
capacity. The alternative is started only when the exact unscaled board
objective improves; rejection restores the ordinary board, incumbent,
accepted count and RNG. The original wall clock still applies, so the
rejected variant can follow a different time-dependent annealing trajectory.
Exclusive surviving precedence means disjoint on the chosen axis but not
on the other; it is a diagnostic, not a proof of causality.

Reproduce from the repository root with official tools and the official
published seed list (the latter is read **only** to assert disjointness):

```sh
D=problems/ahc001/experiments/early_corridor_v4
git show 815b7439185375cd87407c9ec89ebc6e4d2d979c:problems/ahc001/main.cpp > /tmp/ahc001-v64.cpp
g++ -std=c++23 -O3 /tmp/ahc001-v64.cpp -o /tmp/ahc001-v64
python3 scripts/bundle.py problems/ahc001/main.cpp -o /tmp/ahc001-early.cpp
g++ -std=c++23 -O3 -DAHC_EARLY_CORRIDOR /tmp/ahc001-early.cpp -o /tmp/ahc001-early
g++ -std=c++23 -O2 tests/ahc001_early_corridor_test.cpp -o /tmp/ahc001-early-test
/tmp/ahc001-early-test
taskset -c 8 python3 "$D/benchmark.py" "$GEN" "$VIS" /tmp/ahc001-v64 /tmp/ahc001-early "$OFFICIAL_SEEDS" "$WORK"
```

As in the previous studies, the official visualization binary is used solely
for its score. The harness also independently checks board bounds, anchor
containment and pairwise non-overlap. Only authored material and aggregate
development/holdout rows are versioned.

## Result

| Split | v64 mean | Corridor mean | Paired delta (mean / median) | W/T/L | Pairs considered / two axes feasible / selected | Board admissions |
|---|---:|---:|---:|---:|---:|---:|
| Development, 8 | 991,003,733.00 | 990,800,674.88 | −203,058.13 / −192,039 | 3/0/5 | 51/42/48 | 0/8 |
| Untouched, 12 | 995,563,518.67 | 995,333,826.17 | −229,692.50 / −53,821 | 5/0/7 | 73/69/72 | 0/12 |

All 40 outputs were valid; independent and official scores were exact, and
max wall time was **4.819233 s**. The alternative initializer lost
23,536,675–117,648,778 initial score points across all 20 cases, so **no
alternative topology entered annealing**. Local two-axis feasibility was
frequent but not a useful criterion for an entire starting board here.
The paired end scores are reported for the equal-wall experiment, but
cannot be credited to new precedence: both arms started their main search
from the ordinary initializer and extra setup changed elapsed-time annealing.
Do not enable the macro by default or run official final cases for this arm.

Chronological baseline: v64 commit `815b7439185375cd87407c9ec89ebc6e4d2d979c`,
binary SHA-256 `c9689986319fe53ac94c0d5567d708274b1aff13bf8cea22e16e424ca354ed7a`.
Frozen C++23 bundled corridor arm SHA-256
`605e1fe52db34d4bde6171de92959c047ddac3a0cca1cba10de82c87421a1500`;
bundled source SHA-256
`9b4abcf413d2a932eefcf83b4695f03fa4277860ce9683d98cb12101092f0dd0`.
The synthetic two-axis, legality and fallback-identity checks cover 25
mirrored/jittered boards. No new generic library code was justified.

If revisiting this round, the next structurally different experiment is
an **exterior-fixed small-field set solver**, not another greedy precedence
variant. Choose a physically compact group of 5–9 conflicting anchors with
visible score deficits. Enumerate a finite portfolio of legal rectangles per
anchor (including the exact incumbent), using obstacle edges and near-target
aspect alternatives. Select one per anchor by exact branch-and-bound with
bitmask pair conflicts and an optimistic remaining-gain bound. Compare
budgeted same-start results against the same regional edit work. A failing
initializer is not evidence that this set model would fail, but we should not
spend an official run or tune v4 on these data.
