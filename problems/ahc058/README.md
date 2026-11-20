# AHC058 — Apple Incremental Game

[Official statement and scoring](https://atcoder.jp/contests/ahc058/tasks/ahc058_a) ·
[Official generator and visualizer](https://img.atcoder.jp/ahc058/UpvAVdx6.zip) ·
[Official ranking metadata](https://atcoder.jp/contests/ahc058/results/json).

Score is the sum of individually rounded `100000 * log2(final apples)`
over the 150 fixed contest cases, larger better. There is no post-contest
system rerun. The public result identifies **fishylene** in first, but does
not include the score. Final standings redirects to sign-in, both direct
official score/input CSV URLs returned 403, and the possible rank-only
ALE archive path was unavailable. Neither the numeric first-place score
nor exact historical cases are verified; historical comparison is **unjudged**.

## Existing solver and alternative first calendar

The repository's own [prefix-SA example](../../examples/search/ahc058_prefix_sa.cpp)
is the reference; its source attribution is retained in the one-file
problem implementation. It warms up with deterministic 3-step rollout,
stores only purchase order, waits the shortest legal time between purchases,
then edits the calendar with prefix-replay annealing. The variant also
tries ten focal-machine start calendars. Within each it can commit a
contiguous ladder of one to four consecutive levels, from line 0 or the
focal line. Each ladder is evaluated under exact earliest affordability,
including the complete terminal production with no further upgrades. This
can in principle see the value of expensive high-level investment even when
one upgrade by itself has no immediate payoff. Only an alternative with
higher exact final apples can replace the inherited annealing start.

Both arms use the same 3,400 annealing iterations and independent generated
seed sets. The count cap is useful for reproduction; the deployed solver
uses a 1,720 ms cutoff with margin for judge variance.

| Set | Cases | Inherited score ↑ | Ladder warm starts ↑ | Better / tie / worse |
|---|---:|---:|---:|---:|
| Development | 24 | 5,187,945.79 | 5,187,945.79 | 0 / 24 / 0 |
| Holdout | 24 | 5,187,704.58 | 5,187,704.58 | 0 / 24 / 0 |

All 48 pairs had identical initial rounded scores, final scores and exact
final apple totals. The variant was not useful on these cases and remains
opt-in. All 96 runs had 500 legal actions and agreed with the official
visualizer and independent exact-integer replay. Max observed solver wall
0.763907 s at the benchmark cap. [Case logs](benchmark-results.csv).

The repository's existing deterministic rollout, prefix replay and
time-based annealing fit directly. The hypothesis uses the existing
problem-specific exact income polynomial and does not warrant another
library component.

**Next hypothesis:** stop searching new warm starts if they never replace
the inherited calendar. Keep the inherited early purchases exactly and
fix a late cut where affordability is less constrained. Replace one window
of purchases with a same-multiset block reordering that jointly lets high
levels compound before lower-level production upgrades. Evaluate the full
suffix with exact affordability and integer production, against equal
work on the existing local moves; preserve current behavior when it fails.

```sh
p=problems/ahc058
d=$(mktemp -d)
for i in 0 1; do
  g++ -std=c++23 -O3 -DAHC058_SA_ITERATIONS=3400 \
      -DAHC058_MACRO=$i "$p/main.cpp" -o "$d/arm$i"
done
python "$p/benchmark.py" /path/to/gen /path/to/vis "$d/arm0" "$d/arm1" "$d/results.csv"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/submission"
```
