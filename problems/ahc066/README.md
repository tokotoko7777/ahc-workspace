# AHC066 — Macro Controller

[Official problem and scoring](https://atcoder.jp/contests/ahc066/tasks/ahc066_a) ·
[Official generator and scorer](https://img.atcoder.jp/ahc066/O25rQjiK.zip) ·
[Official rank metadata](https://atcoder.jp/contests/ahc066/results/json).

Fully solved raw score is printed button count, lower better; incomplete
is `T * (M - V)`. Printed count is at most T and expanded basics are
truncated at T; the solver actively keeps both inside the bound for fully
solved outputs. The final score is the sum of per-case
`round(10^9 * minimum_legal_abs / own_abs)` over 2000 system cases.
[Official score matrix](https://img.atcoder.jp/ahc_standings/ahc066/result.csv)
(1268 contestant rows × 2000 cases, SHA-256
`edde18d960f3f47b2fad0ffa8e539274ce2d18797b73dca7d30c49d179866a82`)
reconstructs **kurakura, 1,893,330,847,366** with positive minima and
integer rounding. The winning user matches ranked results. The archive
header's visualization URL contains an `ahc063` path, so use the `ahc066`
official tools above instead. Official
[input metadata](https://img.atcoder.jp/ahc_standings/ahc066/input.csv)
fields file, seed, N, M, T, W; SHA-256
`50bfe8fa4be3e043f924c9bb6797fa78db9d0fc28c4db3f5a270934d77ecf7d1`.
[Published final seeds](https://img.atcoder.jp/ahc066/seeds.txt) expected hash
`d88ec61a00aa2dfb0be89801a7320e56550cf9c71cc2e6959f79c5eb057cf9ae`.
Development did not use historical cases. See the [frozen audit](OFFICIAL_AUDIT.md).
Historical raw mean 180.1755 is across different cases than the table below.

## Shared literal trace, then macro routing

Use shortest movement/rotation to each ball and matching basket, compare
the generator's original index order with a deterministic nearest-ball
order. Keep a full legal literal trace. Search its movement-only
substrings of length 4–12 for potential repeated use and keep the top ten.
Each selected macro is registered at its first literal occurrence, so
begin/end registration actions really execute that chosen path.

The control only replaces later matching substrings by P, preserving the
exact expanded trace. The candidate also maps registered macro playback
to a new pose graph edge for every starting cell and heading. It reroutes
to the same remaining pickup and basket positions, minimizing printed
buttons and then expanded basic operations per leg. It checks cumulative
expanded cost against T and retains the original control otherwise.
Bumping a wall during a macro is legal and included in pose computation.

| Fresh cases | Literal buttons ↓ | Trace-only ↓ | Reroute ↓ | Better / tie / worse |
|---|---:|---:|---:|---:|
| Dev (28) | 595.393 | 368.571 | 315.893 | 28 / 0 / 0 |
| Holdout (28) | 443.179 | 288.750 | 248.679 | 28 / 0 / 0 |

Holdout median saving 24.5 printed buttons over exact trace compression.
All 112 outputs fully solve the task and agree with the official scorer
and independent macro expansion, wall collision, held-ball exchange,
and both operation-budget checks. Max solver wall 0.030318 s. Reroute is
the branch default. [Logs](benchmark-results.csv).

The existing graph BFS cannot break equal button counts by separate
expanded usage. Added [unit-edge lexicographic BFS](../../library/unit-lexicographic-bfs.hpp)
with 1,800 random differential tests against pair-cost Dijkstra.

**Next hypothesis:** the top substrings of a fixed route may not span
effective repeated macro geometry. Compare the same number of macro
evaluations using legal templates containing straight repeats and
alternating turns, allowing registration at the first suitable route
position even if it detours from the control trace. Preserve legality by
rerouting the remaining fixed same ball order and enforcing expanded T,
then alternatively optimize ball order on macro-aware pose distances.

```sh
p=problems/ahc066
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC066_TEMPLATES=0 -DAHC066_REPLAN=0 "$p/main.cpp" -o "$d/trace"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 -DAHC066_TEMPLATES=0 "$d/submission.cpp" -o "$d/reroute"
python "$p/benchmark.py" /path/to/gen /path/to/vis \
  "$d/trace" "$d/reroute" "$d/results.csv"
g++ -std=c++23 -O3 tests/unit_lexicographic_bfs_test.cpp -o "$d/bfs-test"
"$d/bfs-test"
```

## Follow-up: geometric macro templates

Both arms preserve the frozen plan and evaluate ten additional one-macro
options. Control tries more substrings of the literal route. Variant
registers a predetermined choice of ten straight or turning templates
at the starting pose; its recorded actions execute immediately. It
then reconstructs the same waypoint order with that single selected
macro. All plans still have both printed and expanded costs capped by T.

| Fresh cases | Frozen buttons ↓ | More substrings ↓ | Templates ↓ | Templates vs more |
|---|---:|---:|---:|---:|
| Dev (28) | 259.464 | 255.143 | 248.286 | 17 / 6 / 5 |
| Holdout (28) | 226.679 | 221.536 | 212.321 | 18 / 7 / 3 |

The holdout median paired improvement is 3.5 buttons vs extra substrings.
All 168 complete solutions agreed with official vis and independent
replay. Max measured solver 0.070245 s; held-out median .0121 s vs .0134 s.
Keep templates as branch default. [Logs](benchmark-template-results.csv).
Next optimize ball order on macro-aware orientation distances at the same
macro candidate count, or compare a second legal re-registration phase
against spending the same exact work on one-macro route order.

```sh
p=problems/ahc066
d=$(mktemp -d)
for i in 0 1 2; do
  g++ -std=c++23 -O3 -DAHC066_TEMPLATES=$i "$p/main.cpp" -o "$d/arm$i"
done
python "$p/benchmark_macros.py" /path/to/gen /path/to/vis \
  "$d/arm0" "$d/arm1" "$d/arm2" "$d/results.csv"
```

[Joint one-macro / delivery-order follow-up](experiments/joint_order_v2/README.md):
six equal graph-build trials per arm; the joint candidate improved the
untouched mean only from 224.375 to 223.500 keys against the fixed-order
control. It remains opt-in and did not repeat the historical audit.
