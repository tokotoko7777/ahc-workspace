# AHC032: Mod Stamp

[Official statement](https://atcoder.jp/contests/ahc032/tasks/ahc032_a) and
[generator / scorer](https://img.atcoder.jp/ahc032/e2weanqa.zip). 2 s,
1024 MiB; place at most 81 nonrotated stamps. Absolute score is sum of the
81 final entries modulo 998244353, added across 150 cases. No post-contest
system test. The upper bound per case is 80,857,792,512.

Final #1 **11,854,207,127,046**, heno239 by
[official results](https://atcoder.jp/contests/ahc032/results/json).
Direct input and case-score CSV access both returned HTTP 403. The score
comes from the rank-score CSV supplied by AtCoder in the
[ALE-Bench archive](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc032.zip)
(zip SHA-256
`6d8a7a6831a00dfc7771a537e5417d2873d5f70bd6f35f84f4a60b20e0497f2a`,
matching the publisher LFS pointer). The local tools' 100 starter cases
are not proof of the 150 actual cases. **No official comparison yet.**

## Inherited beam and coupled coordinate repair

Main includes the repository owner's existing
[action beam model](../../examples/search/ahc032_action_beam.cpp) unchanged;
the source credit remains there. Its bare width-9000 executable took
2.308–2.477 s on three official starter inputs on this host, so both
measured arms use width3500. If a 1.46 s guard is reached between beam
steps, greedily finish a complete legal schedule using the original score.
The repository also has an [existing Chokudai solver](../../examples/search/ahc032_chokudai.cpp);
it was inspected but not changed and is not the paired control.

The repair starts from the exact same completed beam sequence. Pick two
overlapping placed stamps, temporarily remove both, and try all 20×20 stamp
types at the proposed two anchors. One anchor may be shifted by one cell
to allow local allocation changes. The board update computes the exact
signed modular score delta on touched cells; all nonchosen alterations
are rolled back. Accept only strictly improving pairs. No added actions;
a 1.78 s overall guard ends repair.

No generic API was needed. The inherited model's modular addition is
private and the full-score delta requires straight integer residues;
a tiny signed add/sub on normalized residues avoids converting each hot
cell to the existing modular integer object. The differential unit test
compares against full-board replay using the mathematical modulo, checks
reversal, and replays complete repaired schedules.

| 16 fresh cases each | Control mean | Repair mean | Repair W/T/L |
|---|---:|---:|---:|
| Development | 79,075,642,076.81 | 79,075,642,076.81 | 0 / 16 / 0 |
| Untouched holdout | 79,106,180,582.44 | 79,106,180,582.44 | 0 / 16 / 0 |

[Paired data](benchmark-results.csv): all 64 runs matched the independent
replay and official scorer, all beam runs reached all 49 depths, and no
pair edit was adopted. Worst wall time 1.194476 s. The branch defaults to
the tested width3500 with the repair off. The fresh-input means are not
compared to the official 150-case total.

## Reproduction

From repository root (`ATCODER_TOOLS` is extracted tools'
`target/release`, built with `cargo build --release --locked`):

    g++ -std=c++23 -O3 problems/ahc032/main.cpp -o /tmp/ahc032-base
    g++ -std=c++23 -O3 -DAHC032_REPAIR=1 problems/ahc032/main.cpp -o /tmp/ahc032-pair
    python problems/ahc032/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc032-base /tmp/ahc032-pair /tmp/ahc032.csv
    g++ -std=c++23 -O2 tests/ahc032_stamp_delta_test.cpp -o /tmp/ahc032-delta
    /tmp/ahc032-delta
    python scripts/bundle.py problems/ahc032/main.cpp -o /tmp/ahc032-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc032-single.cpp -o /tmp/ahc032-single

Next compare several **dihedral sweep orientations**, transforming both
the board and the stamps and inverse-transforming output anchors, against
the same total beam work in the original direction. This changes which
cells and budget allocations get committed early, not just the low-radius
repair. Verify geometry and exact score invariance before a fresh split.
