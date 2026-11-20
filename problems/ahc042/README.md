# AHC042: Oni wa Soto, Fuku wa Uchi

[Official problem](https://atcoder.jp/contests/ahc042/tasks/ahc042_a) /
[official generator and scorer](https://img.atcoder.jp/ahc042/cnhLtdRT.zip).
2 s, 1024 MiB, at most 1600 unit shifts. With no remaining Oni and no
lost Fuku, the score is `3200 - moves`; otherwise it is
`1600 - 20*(leftOni+lostFuku)`. The sum over 150 contest cases
decides the rank; no post-contest rerun.

Final #1 **468,807**, Shun_PI per
[official results](https://atcoder.jp/contests/ahc042/results/json).
Direct case archive returns HTTP403. Rank/score from an official CSV
provided by AtCoder in the
[ALE-Bench archive](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc042.zip),
SHA-256 matching LFS pointer
`cff8191b01d93c1790597f50ed8359c15ad172b52d82253b74d857b319d6f6ae`.
The official tools' 100 starter cases are not proven to be the 150
contest cases. **Official comparison undetermined.**

## Greedy full return vs beam partial return

The control uses the official existence observation: select a row/column
and eject one or more Oni using pushes in a Fuku-free segment, then push
back by the original distance. Remaining pieces return to their original
cells. Greedily minimize steps per removed Oni. This guarantees complete
recovery on legal inputs and immediately gives a fallback incumbent.

The variant permits no return, half return or full return per safe
ejection. A return no longer than the outbound shift only ejects empty
cells. Each candidate must also preserve the invariant that every
remaining Oni has at least one unobstructed ray to an edge. A complete
return-only continuation therefore exists after every accepted move.
Buckets by remaining count expand a bounded beam, guided by paid
steps and sum of accessible distances. Only complete solutions can
replace the incumbent. The 1.45 s timeout returns the best complete
answer without needing to finish a partial branch.

The two piece types use independent [packed bit boards](../../library/packed-line-shift.hpp).
The helper extracts and replaces lines and performs clipped multi-cell
shifts; it doesn't know which bits may be dropped. This avoided copying
and shifting character grids repeatedly for each beam candidate.
[2,600 differential tests](../../tests/ahc042_shift_test.cpp) compare
two-way macro updates to cell-by-cell replay.

| 32 fresh cases each | Greedy mean moves | Beam mean moves | Beam W/T/L | Median score gain |
|---|---:|---:|---:|---:|
| Development | 281.125 | 111.469 | 32/0/0 | 166.5 |
| Untouched holdout | 282.875 | 111.125 | 32/0/0 | 164.5 |

All 128 outputs removed all Oni without losing Fuku and matched the
official scorer and independent replay. Maximum wall 0.188275 s in
that measurement. The branch defaults to beam. [Paired results](benchmark-results.csv).

Next search a limited set of perpendicular preparations that shift a
blocking Fuku out of a useful ray (without crossing any edge Fuku),
then require an actual Oni ejection and keep the same safety invariant.
Compare to the same beam with extra direct ejections at similar work.
Always keep the complete direct baseline; no finals for tuning.

## Reproduce

From repository root, build the official tools using
`cargo build --release --locked` and let `ATCODER_TOOLS` be their
`target/release` directory.

    g++ -std=c++23 -O3 -DAHC042_BEAM=0 problems/ahc042/main.cpp -o /tmp/ahc042-restore
    g++ -std=c++23 -O3 problems/ahc042/main.cpp -o /tmp/ahc042-beam
    python problems/ahc042/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc042-restore /tmp/ahc042-beam /tmp/ahc042.csv
    g++ -std=c++23 -O2 tests/ahc042_shift_test.cpp -o /tmp/ahc042-test
    /tmp/ahc042-test
    python scripts/bundle.py problems/ahc042/main.cpp -o /tmp/ahc042-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc042-single.cpp -o /tmp/ahc042-single
