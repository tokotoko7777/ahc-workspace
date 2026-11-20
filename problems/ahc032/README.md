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

## Opposite-orientation portfolio

The portfolio compares two complete searches (width1750 from the top-left
and width1750 in original coordinates from the bottom-right) to the same
width3500 one-direction control above. The sum of widths is 3500. The
number of candidate evaluations is logged separately because feasible
action counts depend on the sampled path and its prefix stamp consumption;
equal widths do not imply equal exact evaluation counts. Both share the
1.46 s beam cutoff, with the same greedy completion fallback.

When rotating, both the board and **all stamp patterns** are transformed,
not just their locations. Convert the selected schedule back: the
anchor grid is 7×7 rather than 9×9. The
[square dihedral helper](../../library/square-dihedral.hpp) and its
[exhaustive geometry test](../../tests/square_dihedral_test.cpp) check
forward/inverse transformations and footprints over all eight transforms.
No permission to rotate a stamp is presumed; output always uses the
original input's stamp ID and anchor.

```sh
g++ -std=c++23 -O3 -DAHC032_PORTFOLIO=0 problems/ahc032/main.cpp -o /tmp/ahc032-single
g++ -std=c++23 -O3 -DAHC032_PORTFOLIO=1 problems/ahc032/main.cpp -o /tmp/ahc032-portfolio
python problems/ahc032/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc032-single /tmp/ahc032-portfolio /tmp/ahc032-portfolio.csv --portfolio
g++ -std=c++23 -O2 tests/square_dihedral_test.cpp -o /tmp/ahc032-dihedral-test
/tmp/ahc032-dihedral-test
```

| 16 fresh cases each | Width3500 single | 2×1750 portfolio | Portfolio W/T/L | Paired evaluation ratio (mean) |
|---|---:|---:|---:|---:|
| Development | 79,091,618,213.69 | 79,080,741,334.50 | 8/0/8 | 1.00035 |
| Untouched holdout | 79,089,863,247.81 | 79,109,974,914.63 | 5/2/9 | 1.00291 |

Holdout median gain was negative (−8,949,141.5 points), despite a higher
arithmetic mean due to larger wins. The opposite orientation won its
internal two-search comparison 10/16 development and 8/16 on holdout.
All 64 scorer calls and independent replays agree; all searches reached
depth49. Wall time ≤1.395991 s, evaluation ratio range 0.893–1.099.
No broad gain, so keep the single width3500 default. See
[paired log](portfolio-results.csv). Next try retaining beam lanes by
used-stamp count rather than a single early cumulative cap; move unused
budget to edges using explicit terminal slack and same-budget controls.
