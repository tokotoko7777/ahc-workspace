# AHC060 — Ice Cream Collection

[Official statement / scoring](https://atcoder.jp/contests/ahc060/tasks/ahc060_a) ·
[Official generator / visualizer](https://img.atcoder.jp/ahc060/wX0NuJxV.zip) ·
[Official results JSON](https://atcoder.jp/contests/ahc060/results/json).

Absolute score is the total number of unique flavor strings per shop.
The final contest ranks the sum across the same 150 official cases (larger
better); no system rerun. The official results metadata identifies
**Sweet_Sweet_Soul** as rank 1, but does not include their score. Both
direct case/score archive endpoints returned 403 and the possible ALE
rank-only archive returned 404. No proof of historical cases or numeric
winning score; strict comparison is **unjudged**.

## Delivery frontier

Both arms share exactly the same deterministic 360-action exploratory
prefix up to the next shop, flipping roughly half of visited trees while
favoring unexplored vertices. Thereafter tree flavors are fixed. Starting
with an empty cone at a shop, both propose paths of at most 20 moves and
count at most 1000 edge extensions per dispatch. Non-backtracking condition
persists across deliveries and flips.

The control samples non-backtracking walks. The new arm explores them by
length, stops extending on entering a shop, and deduplicates frontier
states by exact packed word, current and previous vertex. These three
values fully determine the set of possible future completions and resulting
inventories while the flavors are fixed. A width cap of 56 balances coverage;
lower bound distance-to-shop and seeded priorities settle ties. Both prefer
short novel deliveries and a legal fallback route when none are found.

| Fresh cases | Sample ↑ | Frontier ↑ | Win / tie / loss |
|---|---:|---:|---:|
| Dev (24) | 851.750 | 867.958 | 21 / 0 / 3 |
| Holdout (24) | 882.750 | 894.875 | 19 / 0 / 5 |

Holdout median gain 14. Held-out frontier mean edge checks 451,105 vs
969,708 for samples, despite the same cap per dispatch, and median solver
wall time 0.0305 s vs 0.0407 s. Max observed 0.087508 s across all
96 runs. All 96 official scores, move counts, permanent flavor changes
and no-backtracking conditions match an independent exact replay. Keep
frontier as branch default. [Logs](benchmark-results.csv).

The fallback cannot use `graph_bfs.hpp` without explicitly adding the last
vertex to state. A naive queue copied whole path vectors, so
[`shortest_nonbacktracking_path`](../../library/nonbacktracking-path.hpp)
uses parent reconstruction in directed-edge state space instead.
Its differential test compares with exact layer-by-layer state reachability.

**Next hypothesis:** let the frontier evaluate at most one legal flavor flip
along a sampled delivery, not just a frozen coloring. A flip should count
one extra action and change all following emissions from that tree without
changing its first harvest. Use an extra flag/vertex in the dedup key;
compare against a higher-budget frozen-color frontier with the same
untouched-case protocol. Avoid allocating an irreversible flip to a single
novel string if it destroys many cheap still-unseen future strings.

```sh
p=problems/ahc060
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC060_RICH=0 -DAHC060_FRONTIER=0 "$p/main.cpp" -o "$d/base"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 -DAHC060_RICH=0 "$d/submission.cpp" -o "$d/frontier"
python "$p/benchmark.py" /path/to/gen /path/to/vis "$d/base" "$d/frontier" "$d/results.csv"
g++ -std=c++23 -O3 tests/nonbacktracking_path_test.cpp -o "$d/path-test"
"$d/path-test"
```

## Follow-up: one flip within a delivery macro

A macro may apply one irreversible flip immediately after harvesting from
a white tree, at one out of every twelve dispatch opportunities. The
already-harvested scoop stays white. Subsequent visits to that tree in
the same path produce red. Count the flip as one real action and
include its identity in the exact frontier key. Prefer not to spend
a flip on the same delivery result; prohibit fallback duplicates with
flips. The fixed wider control and flip arm both cap width at 104 and
extensions at 1800 per dispatch, versus width 56 and extensions 1000 in
the inherited mode. Flips also count against the extension budget.

| Fresh cases | Frozen ↑ | Wider frozen ↑ | Flip macros ↑ | vs wider |
|---|---:|---:|---:|---:|
| Dev (24) | 872.333 | 870.875 | 980.708 | 24 / 0 / 0 |
| Holdout (24) | 888.833 | 887.042 | 992.333 | 24 / 0 / 0 |

Held-out median gain 105, with 488,729 actual probes versus 586,539 in
the wider control. Median wall .03355 s vs .03875 s, max .111667 s
across all arms. All 144 official and independent scores matched full
legality replay with 10,000 operations per case. Flip macros are the
branch default. Final comparisons remain unavailable.
[Raw log](benchmark-flip-results.csv).

Next estimate future coverage preserved on each proposed flip before
committing: compare the short unseen closed paths around that tree with
and without its color change, then account both immediate novelty and
regional destruction of opportunities.

```sh
p=problems/ahc060
d=$(mktemp -d)
for i in 0 1 2; do
  g++ -std=c++23 -O3 -DAHC060_RICH=$i "$p/main.cpp" -o "$d/arm$i"
done
python "$p/benchmark_flips.py" /path/to/gen /path/to/vis \
  "$d/arm0" "$d/arm1" "$d/arm2" "$d/results.csv"
```

## 固定経路で反転時刻だけを調整する（オフライン実験）

既定ソルバーの移動列・反転対象の木・反転回数は変えず、すでに反転している木ごとに、
反転を置く訪問直後の位置を1回ずつ全探索する後処理。新しく木を反転することはない。
訪問で採取された直後に反転するので、その訪問の味は変更されず、次の訪問から赤になる。
境界を訪問1回分だけ動かすと、納品済みなら対応する文字列の1文字だけが変化する。
店ごとに完全な文字列の出現回数を保存し、旧語の最後の出現を消した場合に減点、新語の最初の
出現に加点する。衝突判定には文字列全体の一致を使い、空文字の納品も保持する。
最後の未納品分は得点に含めない。候補評価後は最良位置（同点なら元の位置を優先）に戻す。
出力に実在する訪問直後だけを候補にするため、行動上限・後戻り禁止は元の合法な出力から不変。

1回の座標上昇であり、大域最適化ではない。正確な文字列キー処理量を制限するため、
各木の訪問が属する納品語の長さの和を基に、予算に入らない木は掃引しない。
既定ソルバー・提出コードには組み込んでいない。

```sh
g++ -std=c++23 -O2 problems/ahc060/relocate_flips.cpp -o /tmp/ahc060-relocate
/tmp/ahc060-relocate instance.txt < original-answer.txt > relocated-answer.txt
g++ -std=c++23 -O2 problems/ahc060/fixed_walk_relocation_test.cpp -o /tmp/ahc060-relocate-test
/tmp/ahc060-relocate-test
```

検証は、4頂点環で旧語が重複する衝突例、88文字の納品語と未納品末尾、
乱択で生成した合法な2,000経路の独立全再生（木・候補位置ごと）との照合。
ペア評価の再現には `benchmark_relocation.py GEN VIS SOLVER RELOCATE CSV` を使う。
1ケースのソルバー出力を両群で固定し、元と後処理後の両方を公式visと独立再生で検証する。

| 新規生成セット | 件数 | 元の合計 | 後処理後 | 改善 / 同点 / 悪化 |
| --- | ---: | ---: | ---: | ---: |
| 開発 | 6 | 6,005 | 6,012 | 5 / 1 / 0 |
| 未調整 | 12 | 11,554 | 11,572 | 9 / 3 / 0 |

全18対の移動列、反転対象、総行動数は一致し、元と後処理後の全36出力で公式visと
独立再生の得点が一致した。後処理の実測最大約19ms（別プロセス起動込み）。
比較の元はローカル `e4a5e2b13abeb3fe70b84a87c91fa23d939fefe9` で、公開版
`6e7f42ee36c537681b0db68526aba19077582ad0` と対象ディレクトリおよび経路ヘッダが同一。
本番データ・本番順位との比較はしていない。
