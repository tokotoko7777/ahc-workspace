# AHC008: follow-ups to the selected v24 baseline

The [v24 single-file C++23 solver](https://github.com/tokotoko7777/ahc-workspace/blob/abafd80edf423db5abea3c5725b265cb092233c8/problems/ahc008/experiments/20261122/article-capture/v24-source-release/solve-v24.cpp)
remains selected. No results here establish first-place contest performance.
The authorized Rust was used for ideas, not executed or published.
Each row is official tester + independent score and action replay.
Tables describe one changed policy at a time; no claims from unpaired seeds.

| Trial | Main result | Decision |
|---|---|---|
| Edge solo, +2 prep caps, dev30 | M5–6: one genuine new solo closure; residual38→23; full2→2 but usable≥3002→0; score164,127,279→143,874,748. | Do not adopt. |
| Existing late rail, t225–250 and 2 left, dev30 | No activation; all30 outputs and scores identical. | No behavioral gain under strict guards. |
| Defer t160 switch on real own trap momentum, dev30 | Activated7; full15→16, but new cases retained only13/44 cells and lost case retained513. Useful≥30012→11. | Do not adopt. |
| Distinct worker estimate in initial layout, both100 | Scores3,199,807,870→3,165,612,682 and3,285,615,612→3,282,480,471, useful33→31 and35→34. | Do not adopt. |
| Exact v7 when M=5, fresh third100 | M5 subset13: score171,722,219→135,018,361, full5→5/useful0→0, remaining19→39. All other87 action hashes identical; all13 intended v7 branches exact. | Fails preregistered requirement. |
| Five-person minimum line target area, dev30 | M5 mean floor447.4→546.74; full and useful unchanged1; score102,071,723→98,083,377. | Area alone insufficient. |
| Five-person sequential half-close, dev30 | No activation, all7 affected-subset actions identical. An M10 score difference comes from the late CPU-dependent sample count. | No behavioral gain. |
| Five-person room-retention forecast, fresh dev30-M5 | score434,537,087→380,384,183, full10→9/useful2→1; early true-four70→80 but final207→195. | Do not adopt. |
| Five-person regular rails through t225, same dev30-M5 | Added genuine true-four in t161–225 from3→66, but full10→4/useful2→0; final residual62→115, score434,537,087→328,454,429. | Late rescue also needs time. Next try a narrowly permitted real closure during the original rescue. |

Older trial namespaces used fresh development or fixed practice plus an
independent pool; do not conflate a result on 7 or 13 affected workers with
an all-case score. The immediately repeated official tester and independent
playback agree for every run; some historically repeated late decisions
differ at a CPU reserve and are not described as byte-identical.

## Reproduce

Apply an individual `from-v24.patch` onto the exact source above, then build
with `g++ -std=c++23`. `source.sha256` checks the reconstructed byte hash.
Use `bench.py` from the TERRY integration directory with the same official
tester, playback harness and input sets. Archives retain case/input hashes,
action hashes, official score, isolation/area statistics and telemetry.
The two fresh dev sets requiring M=5 use the same manifest
`M5-development-inputs.json`, filtered by header only before scoring from a
fixed predeclared candidate stream; generate the recorded seeds in manifest
order. The portfolio directory has its separate true unseen 100 seed
manifest, exact wrapper generator, acceptance rules and post-run branch
checks. No main merge.
