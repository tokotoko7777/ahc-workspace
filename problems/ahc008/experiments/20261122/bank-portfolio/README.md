# AHC008 visible many-human/few-animal portfolio pilot

Gate fixed ahead of test: use frozen plain staged+stable-zone bank policy (CLI5, direction0) only when visible M>=8 and N<=12; otherwise run unchanged tactical. [siman's user-authorized article](https://simanman.hatenablog.com/entry/2022/02/28/074838) uses this visible case split for a different known layout. No reference solver code, no other threshold tried. Our two own policies are included as separate C++ namespaces; reading and replaying the visible initial input is the only dispatch change. No directional ranking, irregular shapes, other new geometry or artificial practice selection.

Fifty predeclared independent unselected official generator cases: first eight SHA256 bytes little-endian of ahc008/20261122/m8-n12-plain-staged-zone/frozen50/{i}. Equal cases and official tester, ordered A/B and B/A alternately, exact independent scoring replay. Plain mode score and action bytes agreed with archived CLI5 direction0 on the first eligible case (0000-based file 0024). Off-gate original is compared on every case by action hashes; all 45 off-gate streams were identical.

| Independent random50 | Tactical | Portfolio |
| --- | ---: | ---: |
| All 50 official score | 687,164,134 | 776,772,583 |
| Eligible five only | 100,099,884 | 189,708,333 |
| Eligible final exact-four animals | 0 | 46 |
| Eligible distinct residual pets with humans | 16 | 6 |
| Eligible all-clear cases | 0 | 1 |
| Eligible safe humans | 26 | 10 |
| Eligible sum distinct safe area | 945 | 651 |

Eligible score outcomes 3 wins / 2 losses; other 45 exact ties. Overall +13.04%, but descriptive paired case bootstrap 95% interval approximately −4.7% to +38.8%, small selected set. Max tester wall 0.443/0.598 sec. All 100 actions legal. 45 direct true exact-four captures verified at construction: cow6/pig8/rabbit8/dog15/cat8. Final tight compartment count 46 includes one from residual tactical later. Main room usage did not uniformly improve, so no adoption on this sample alone. An optional validation should freeze the exact same gate and policy, not shift thresholds.

Reproduction from this directory: g++ -std=c++23 portfolio.cpp -o portfolio, official generator on seeds.txt, python bench.py. The original local runner paths use own frozen tactical and bank CLI artifacts plus repository replay harness. make.py generates self-contained portfolio.cpp from the two own inputs tactical.cpp and bank.cpp. freeze.sha256 fixes the policy and seed list.

## Exact frozen two-set confirmation

Pool and policy frozen before scoring. The fresh100 hash seeds were first 8 SHA256 bytes little-endian of ahc008/20261122/m8-n12-plain-staged-zone/confirm100/{i}; distributed100 is the shipped official v3 practice set, not original contest finals. The comparator crosses both datasets with unchanged tactical. Safe per-person area is zero for humans sharing any component with a pet, so counts all humans even in unsuccessfully protected groups.

Fresh100: **1,713,301,592→1,697,831,161** (−0.90%, 9W/81T/10L), 81/81 off-gate action streams identical. Among 19 selected: 600,208,855→584,738,424 (−2.58%), final exact-four 1→154, residual 65→37, all-clear 2→3, but safe humans **137→49 of 169** and mean per-person usable area 278.97→168.91. Median 279→0; upper quartile 415→592, showing concentrated large wins rather than broadly improved human space. Distinct safe area 5,839→3,175. The bank directly captured 153 exact-four [cow25, pig23, rabbit21, dog44, cat40]. All official and independent replay scores match; max wall 0.487/0.651 sec.

Distributed100: **1,512,932,629→1,789,105,112** (+18.25%, 12W/85T/3L), all 85 off-gate bytes match. Among 15 selected: 352,094,878→628,267,361 (+78.44%), final exact-four 0→129, residual 49→19, all-clear 2→6, safe humans 96→61 of 132. Mean per-person usable area 190.83→270.39, but median **146→0** and upper quartile 303→613. Distinct safe area 3,157→3,967. Bank directly captured 127 [cow18, pig17, rabbit21, dog35, cat36]. All runs valid; max official tester wall 0.464/0.545 sec.

Across both sets, 166/166 off-gate stdout streams matched exactly, all 400 paired case-arm validations passed official and independent replay, and first selected dispatch matched original plain CLI5/0 output bytes in **both** sets. Housing pets in exact-four groups is a robust effect of this gate and branch. A normal-score gain and broadly larger per-person usable area are not: score loses fresh and wins distributed, and median selected usable area falls to zero on both. **Do not deploy as normal-score default.** Retain the frozen capture-oriented portfolio as a separate research checkpoint; no tuning or source changes after 50, no competition submission.

### Portable repetition

Compile tactical.cpp as a tactical binary, bank.cpp as a plain test binary (the replay runner passes arguments 5 0), and portfolio.cpp as ./portfolio, each with GNU C++23 and no extra flags. Export AHC008_HARNESS (path to repository problems/ahc008 containing benchmark.py and benchmark_cases.py), AHC008_TESTER (official tester), AHC008_TACTICAL and AHC008_PLAIN. Generate in/ with seeds.txt for the exploratory50 or fresh100/in with fresh100/seeds.txt for confirmation. For the exact paired confirmation use:
    python bench100.py fresh100/in fresh-repro.csv
    python bench100.py /path/to/official/v3/distributed/in distributed-repro.csv
    python summarize.py fresh-repro.csv
    python summarize.py distributed-repro.csv
parallel_banks_direction_bench.py holds the exact prior replay compactness checker, imported only as a metric; the portfolio contains no active direction variant.
