# AHC008 v7: use an empty pen to reach an understaffed aisle

Current all-head-first research version, not a contest submission or main
merge. In the same article-based 1×4 and irregular-cleanup policy, allow
one unassigned worker to cross a truly empty open tube to rebalance an
otherwise unstaffed service lane. The worker is already at that tube's
outside stance; a different worker remains available on the source
lane; another usable tube serves the destination. Wait for every brace
to be installed and keep the usual immediate seals and already
assigned/holding pairs ahead of transfers. Reserve both gates of the
specific transit tube until the worker exits. The window t101–152 keeps
each seven-move transit out of the t160 fallback. No new walls, no
per-problem geometry or cleanup change.

| Same official tester v3 input | Distributed practice100, v4→v7 | Frozen generated100, v4→v7 |
|---|---:|---:|
| All individual animals fully isolated from every human | **33→36** | **41→44** |
| Gained / lost full-clear boards | 5 / 2 | 7 / 4 |
| Full-clear and ≥300 mean reachable floor cells for people | 22→25 | 24→26 |
| Full-clear and ≥100 mean reachable floor cells for people | 33→35 | 38→42 |
| True final exact-four animal count | 814→822 | 830→837 |
| Still human-reachable animal count | 166→165 | 152→139 |
| Score | 2,598,735,635→2,655,369,894 | 2,519,824,436→2,688,102,365 |
| Active / action-identical inactive boards | 21 / 79 | 21 / 79 |
| Started / finished actual transfers | 24 / 24 | 27 / 27 |

All 400 official tester executions were valid, independently scored
from the exact logs. All 200 historical v4 action hashes match again.
In the two cohorts, each pair is action-identical through t100; no
inactive execution reports any action changes. The new maximum measured
wall times, official tester included, were .946s and .870s. This uses
the ordinary v4 forecasting implementation; the previously published
passive-memo alternate has identical v4 actions but was not silently
combined with this experiment.

Newly clear cases were not all compact: one practice case leaves just
36 human cells and one generated case has a 441-cell pet pen. Count
raw clear and useful space separately. In a constructive generated
example case0023, all 12 animals are in exact-four pockets and the
people retain 666 connected cells. No input/case-specific rule uses
these retrospective cases.

```sh
D=problems/ahc008/experiments/20261122
A="$D/article-capture"
g++ -std=c++23 "$A/solve.cpp" -o /tmp/article-v4
g++ -std=c++23 "$A/solve-v7.cpp" -o /tmp/article-v7
PYTHONPATH="$D" python3 "$A/bench_lane_transfer.py" problems/ahc008 "$TESTER" /tmp/article-v4 /tmp/article-v7 "$PRACTICE100" /tmp/article-v7.csv --n 100
```

Use the same adjacent `independent100-seeds.txt` with official generator
for the generated validation. No command-line options to the solver
itself are needed. v7 source SHA256
`65ef9c66a898ccd564d3874de5bcc422962ffe444da2511d057bf9a73ada364b`.
