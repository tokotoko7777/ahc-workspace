# AHC034: Leveling with a Dump Truck

[Official statement](https://atcoder.jp/contests/ahc034/tasks/ahc034_a) and
[generator / scorer](https://img.atcoder.jp/ahc034/vImT4eac.zip). 2 seconds,
1024 MiB, at most 100,000 commands. Maximum score is sought: on each of
150 contest cases, round `1e9 * base / (cost + diff)`, sum these scores;
`base` is original absolute height sum and `diff` is final-height penalty.
There is no post-contest rerun. These solutions leave every height zero.

Final #1 **9,486,350,384**, yosupo by
[official results](https://atcoder.jp/contests/ahc034/results/json). The
official per-case archive endpoints returned HTTP 403; score cross-reference
is the rank/score CSV supplied by AtCoder in the
[ALE-Bench archive](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc034.zip),
SHA-256 matching LFS pointer
`a6c2666e596dee5a743d80cfcfac8cf1d90d80696a2c11acfd4891b7467ce889`.
The official generator bundle has 100 starter cases, not proven identical to
the 150 contest cases. **Official comparison undetermined.**

## Initial route and exact order edits

Each nonzero cell is served once in a route: pick up the full positive
height or deposit its full deficit. For a candidate snake through all cells,
omit zero-height events, then rotate after a minimum cumulative height.
Because total height is zero, the rotation guarantees that the carried
load is nonnegative. Check eight snakes with the true travel score,
including initial empty travel from (0,0).

Both arms start with that same route. The annealing arm proposes reversals,
swaps, and relocations and rejects any path with a negative-load prefix.
A [load-aware route summary](../../library/load-aware-route.hpp) tracks net
load, min/max prefix, geometric length and load-weighted travel. Restoring
base handling cost is trivial because each cell is served fully once.
The existing [segment tree](../../library/segment-tree.hpp) joins a small
number of range summaries to score the move exactly. It has no existing
in-place all-leaf rebuild call, so the experiment refreshes the exposed
nodes linearly the moment an edit is accepted rather than call O(log n)
point updates or reallocate a tree for each accepted edit. The complexity of
a proposed edit before acceptance is O(log n).

| 24 fresh cases each | Snake mean score | Annealed mean score | W/T/L | Paired travel-cost ratio |
|---|---:|---:|---:|---:|
| Development | 23,160,596.71 | 30,677,647.17 | 23/1/0 | 0.73632 |
| Untouched holdout | 25,131,542.38 | 33,027,861.63 | 24/0/0 | 0.74009 |

All 96 official scores matched full independent replay including load,
terrain, handling and score. Worst measured wall time was 0.159890 s under
the 1.55 s internal guard. Maximum output 2,429 commands. Branch defaults
annealing. [Paired data](benchmark-results.csv) do not use final seeds.

Next broaden the route representation: permit multiple visits to one
source/destination and optimize pickup/drop quantities for a fixed walk.
A time-expanded flow graph has forward arcs with distance costs and
one source or sink supply node per cell, linked to all its visits.
Use exact mass balance and integral flow to recover legal partial service,
then test the trade-off of additional empty movement on fresh instances.
The current prefix summary is still useful for validating a stitched route
or exact work in candidate fixed walks.

## Reproduction

From repository root (`ATCODER_TOOLS` is the extracted official tools'
`target/release` directory, built with `cargo build --release --locked`):

    g++ -std=c++23 -O3 -DAHC034_SHORTCUTS=0 -DAHC034_ANNEAL=0 -DAHC034_FLOW=0 problems/ahc034/main.cpp -o /tmp/ahc034-snake
    g++ -std=c++23 -O3 -DAHC034_SHORTCUTS=0 -DAHC034_FLOW=0 problems/ahc034/main.cpp -o /tmp/ahc034-anneal
    python problems/ahc034/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc034-snake /tmp/ahc034-anneal /tmp/ahc034.csv
    g++ -std=c++23 -O2 tests/load_aware_route_test.cpp -o /tmp/ahc034-summary-test
    /tmp/ahc034-summary-test
    python scripts/bundle.py problems/ahc034/main.cpp -o /tmp/ahc034-single.cpp
    g++ -std=c++23 -O3 -DAHC034_SHORTCUTS=0 /tmp/ahc034-single.cpp -o /tmp/ahc034-single

Optional `-DAHC034_VERIFY` compares every accepted move to full route
recomputation, intended only for local diagnostics.

## Fixed-walk integral flow follow-up

Keep **every** emitted movement command from the two-epoch annealed
incumbent. The truck can load/unload during a traversal of a nonzero cell,
not just its designated visit. Each time step becomes a graph node,
with next-step arcs of cost1 per carried unit. A positive cell connects to
all its visits on the source side; all visits to a negative cell connect
to its demand node. Capacities enforce exact per-cell height requirements.
Integral flow yields legal soil quantities, with total edge cost equal
to the carried load-distance. Movement overhead and handling are fixed.
Only accept a lower-cost **full** flow; the time cutoff keeps the incumbent.

| 24 new cases each | Same itinerary incumbent | Split flow | W/T/L | Paired travel-cost ratio |
|---|---:|---:|---:|---:|
| Development | 29,830,615.04 | 33,266,808.71 | 24/0/0 | 0.89379 |
| Untouched holdout | 30,949,419.92 | 35,727,578.04 | 24/0/0 | 0.86627 |

Both arms had identical motion bytes, no detours, and the same annealing
schedule. All 96 traces were legal, completely flattened the grid, and
matched official scoring. Maximum measured wall time 0.273714 s and
maximum output 2523 commands. The same-itinerary comparison isolates the
allocation effect but the extra flow uses more time; a separate run
compares it with more iterations using a new seed split.

On an independent 24+24 resource-control split, the flow variant won every
case versus five annealing epochs (500k move proposals minus early skips, each
arm under the same overall time limit). Holdout score averaged 31,226,877.96
→34,773,539.50 and the median wall times were 0.152 vs 0.164 s; development
32,179,630.42→36,336,346.83 (median 0.156 vs 0.160 s). The lengths can differ
in this control because the longer search changes the route. Thus this is
an approximately time-matched budget comparison, not the isolation test.
All 96 additional outputs matched exact replay and the official scorer.
The branch defaults to flow. See [same walk logs](fixed-flow-results.csv)
and [resource control logs](flow-resource-results.csv).

The reusable flow implementation previously could not stop before completing
all augmentations, even when a legal incumbent already existed.
[`flow_until`](../../library/min-cost-flow.hpp) adds a templated cooperative
stop callback with [partial-flow tests](../../tests/min_cost_flow_stop_test.cpp).
It polls between shortest path augmentations; AHC034 stops flow at 1.73 s.

    g++ -std=c++23 -O3 -DAHC034_SHORTCUTS=0 problems/ahc034/main.cpp -o /tmp/ahc034-flow
    python problems/ahc034/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc034-anneal /tmp/ahc034-flow /tmp/ahc034-flow.csv --flow
    g++ -std=c++23 -O3 -DAHC034_SHORTCUTS=0 -DAHC034_FLOW=0 -DAHC034_EPOCHS=5 problems/ahc034/main.cpp -o /tmp/ahc034-long
    python problems/ahc034/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc034-long /tmp/ahc034-flow /tmp/ahc034-control.csv --resource

Next try **constructing several resource-matched short walks** optimized for
local positive/negative exchanges rather than one snake-derived route.
Score full fixed-walk flow only on a shortlist (the unconditional
baseline remains a valid choice), and compare selected walks to local
path changes that keep step count unchanged.

## 開発診断: 移動と積載移動の理論下限

元盤面の高さの絶対値和を (B) とする。完全整地する任意の出力の
費用は (B+100L+W) 以上。(L) は原点と初期の非ゼロセルすべてを
訪れるのに必要な移動の下限で、原点を含む格子の偶奇別点数を
(e,o) として `max(2*(e-1), 2*o-1, Manhattan MST)` を使う。
(W) は切断を横切る土量の下限で、行と列それぞれの高さ合計の
累積和の絶対値を足す。切断ごとの正味移動量は初期高さの和で決まる
ため、回り道や仮置きもこの下限を破らない。

この下限は未整地の出力にも使える。残りセル数を (R)、残り高さの
絶対値和を (A) とすると、積荷は高々 (A)。現在地に積荷を下ろすと
残りは収支ゼロの高々 (R+1) セルで、移動すべき正の土量も高々 (A)。
正負セルを1組ずつ結び、各組で少なくとも一方を空にすると (R) 組以下。
20×20の格子で1組あたり空荷と積載移動がそれぞれ高々38手なので、
残作業の費用は `7600*R+41*A` 以下（固定移動、荷重移動、積み降ろし、
最初の積荷下ろしを含む）。公式の未整地ペナルティ
`10000*R+100*A` はこれを上回る。下限は操作数制限を外した整地にも
成立するので、この仮想の残作業を実際に出力する必要はない。

```bash
g++ -std=c++23 -O3 -DAHC034_SHORTCUTS=0 problems/ahc034/main.cpp -o /tmp/ahc034-flow
python3 problems/ahc034/diagnose_ceiling.py "$gen" "$vis" /tmp/ahc034-flow /tmp/ahc034-diagnostic
```

公式ジェネレータの独立した開発 seed 20 件で、固定経路の流量版を
公式採点と二重計測。平均得点 33,596,853.25、上限 89,685,469.85。
移動数は平均 1,549.15、下限 392.55。積載量×移動数は平均 44,573.35、
下限 30,097.65。下限までの費用差の内訳は固定移動費 115,660、
積載費 14,475.70、操作費 0。積載費だけでは理論上も改善幅が限られる
ので、次は同じ再配分を保ちながら経路を短縮する。移動の下限は緩く、
この差すべてを削れるとは限らない。本番ケースや順位には使わない。

[物理経路のショートカット実験](experiments/physical_shortcuts_v3/README.md)では、
未調整16件中3件を改善しましたが、全非ゼロセルを残しても52候補中43件は
訪問順の関係で積載量が不足しました。既定版は変えず、次は経路変更時に
最速積載・最遅荷下ろしの接頭辞を検査します。

[積荷の接頭辞を守るショートカット](experiments/chronological_shortcuts_v4/README.md)
では、未調整16件で固定経路の流量版に15勝1分、平均得点が35,225,693.81から
43,686,637.44に上がりました。全104出力で公式採点と独立再生が一致し、
最大0.391493秒のため問題別ブランチの既定版とします。過去実験には
`-DAHC034_SHORTCUTS=0` を指定します。本番と同じケースでの比較は未判定です。
